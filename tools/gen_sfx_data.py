#!/usr/bin/env python3
"""Extracts note/volume data from sfx/0.sfx and emits core/sfx_data.hpp.

sfx/0.sfx is a Picotron POD file: a Lua-source-looking header, then
"lz4\\0" + sizes + a raw LZ4 block, which decompresses to a "pxu\\0"-prefixed
blob using Picotron's MTF/RLE userdata compression. Decoding that blob
yields a flat byte array that is a direct memory image of Picotron's synth
data area (see picotron_synth.html): a 32-byte index, then (per this
cart's flags) instrument data right after the index, then track data at a
fixed offset (0x20000), each track being an 8-byte header followed by five
64-byte columns: pitch, instrument, volume, effect, effect_param.

This script performs that decode exactly once, offline, and prints the
extracted per-row pitch/instrument/volume for every non-empty track as a
static C++ header -- so the runtime (including a future ESP32-P4 target)
never needs an LZ4/PXU decoder at all, only the plain note data.

Algorithm credits: the pxu MTF/RLE decode is a straight port of
read_pxu()/update_mtf() from thisismypassport/shrinko8 (MIT licensed),
used here only to understand the format -- no code from that project is
included in the game itself, only the numbers it lets us extract.
"""
import struct
import sys

try:
    import lz4.block
except ImportError:
    sys.exit("pip install lz4")


def lz4_pod_decompress(path):
    data = open(path, "rb").read()
    assert data.startswith(b"--[[p"), "not a picotron pod file"
    end = data.index(b"]]", 4) + 2
    rest = data[end:]
    assert rest[:4] == b"lz4\x00", rest[:4]
    comp_size, uncomp_size = struct.unpack_from("<II", rest, 4)
    comp_data = rest[12:12 + comp_size]
    return lz4.block.decompress(comp_data, uncompressed_size=uncomp_size)


def update_mtf(mtf, idx, ch):
    for ii in range(idx, 0, -1):
        mtf[ii] = mtf[ii - 1]
    mtf[0] = ch


class Reader:
    def __init__(self, data, pos=0):
        self.data, self.pos = data, pos

    def u8(self):
        v = self.data[self.pos]; self.pos += 1; return v

    def u16(self):
        v = struct.unpack_from("<H", self.data, self.pos)[0]; self.pos += 2; return v

    def u32(self):
        v = struct.unpack_from("<I", self.data, self.pos)[0]; self.pos += 4; return v


PXU_TYPES = {3: 1, 12: 2}  # type code -> bytes per sample (u8, i16)


def read_pxu(data, idx):
    """Port of shrinko8's read_pxu(): decodes one 'pxu\\0'-prefixed MTF/RLE
    userdata blob starting at idx. Returns (values, end_pos)."""
    r = Reader(data, idx)
    assert data[r.pos:r.pos + 4] == b"pxu\x00"
    r.pos += 4
    flags = r.u16()
    type_code = flags & 0xF
    has_height = bool(flags & 0x40)
    long_size = bool(flags & 0x800)
    compression = (flags & 0xF000) >> 12

    width = r.u32() if long_size else r.u8()
    height = (r.u32() if long_size else r.u8()) if has_height else 1
    size = width * height
    type_bytes = PXU_TYPES[type_code]

    def read_type():
        return r.u8() if type_bytes == 1 else r.u16()

    out = []
    if compression == 1:  # none
        out = [read_type() for _ in range(size)]
    elif compression in (2, 8):  # mtf, rle
        bits = r.u8()
        mask = (1 << bits) - 1
        ext_count = 1 << (8 - bits)
        mapping = list(range(mask))
        mtf = list(range(mask))
        value = None
        while len(out) < size:
            b = r.u8()
            index = b & mask
            if index == mask:
                value = read_type()
                if bits:
                    mapping[mtf[-1]] = value
            else:
                update_mtf(mtf, mtf.index(index), index)
                value = mapping[index]
            count = 1 + (b >> bits)
            if count == ext_count:
                while True:
                    c = r.u8()
                    count += c
                    if c != 0xFF:
                        break
            out.extend([value] * count)
    else:
        raise ValueError(f"unsupported pxu compression {compression}")
    return out, r.pos


def extract_tracks(decoded):
    """decoded: flat byte array = Picotron synth memory image starting at
    the 0x30000 index. Returns list of (track_no, spd, pitch[64], inst[64],
    vol[64]) for tracks that contain any real (non-0xff) data."""
    def i16(o): return struct.unpack_from("<h", decoded, o)[0]

    num_tracks = i16(2)
    flags = i16(6)
    tracks_base = 0x20000 if (flags & 1) else struct.unpack_from("<i", decoded, 12)[0]

    tracks = []
    for i in range(num_tracks):
        off = tracks_base + i * 328
        if off + 328 > len(decoded):
            break
        pitch = decoded[off + 8: off + 8 + 64]
        if all(p == 0xFF for p in pitch):
            continue
        spd = decoded[off + 2]
        loop0 = decoded[off + 3]  # see TrackHeader comment in sfx_data.hpp
        inst = decoded[off + 8 + 64: off + 8 + 128]
        vol = decoded[off + 8 + 128: off + 8 + 192]
        tracks.append((i, spd, loop0, list(pitch), list(inst), list(vol)))
    return tracks


def emit_cpp(tracks, out_path):
    lines = []
    lines.append("// AUTO-GENERATED from sfx/0.sfx by tools/gen_sfx_data.py")
    lines.append("// Do not hand-edit; regenerate instead. See that script's")
    lines.append("// docstring for how this data was extracted.")
    lines.append("#pragma once")
    lines.append("")
    lines.append("struct SfxTrackDef {")
    lines.append("    int track_no;")
    lines.append("    int spd;             // ticks per row")
    lines.append("    int loop0;           // row to loop back to for tracks played on a")
    lines.append("                         // held/repeating trigger (see audio.hpp's")
    lines.append("                         // wants_continuous_loop) -- 0 if unused")
    lines.append("    unsigned char pitch[64];  // picotron pitch, 0xff = rest (48 = middle C)")
    lines.append("    unsigned char inst[64];   // instrument id, 0xff = n/a")
    lines.append("    unsigned char vol[64];    // 0-64, 0xff = n/a")
    lines.append("};")
    lines.append("")
    lines.append(f"inline const SfxTrackDef kSfxTracks[] = {{")
    for track_no, spd, loop0, pitch, inst, vol in tracks:
        p = ",".join(str(v) for v in pitch)
        ii = ",".join(str(v) for v in inst)
        vv = ",".join(str(v) for v in vol)
        lines.append(f"    {{ {track_no}, {spd}, {loop0}, {{{p}}}, {{{ii}}}, {{{vv}}} }},")
    lines.append("};")
    lines.append(f"inline constexpr int kSfxTracks_count = {len(tracks)};")
    lines.append("")
    lines.append("// find a track's row data by its original track number (see comment above")
    lines.append("// gen_sfx_data.py's extract_tracks for how track_no maps to sfx()/music() ids)")
    lines.append("inline const SfxTrackDef* find_sfx_track(int track_no) {")
    lines.append("    for (int i = 0; i < kSfxTracks_count; i++)")
    lines.append("        if (kSfxTracks[i].track_no == track_no) return &kSfxTracks[i];")
    lines.append("    return nullptr;")
    lines.append("}")
    lines.append("")
    open(out_path, "w").write("\n".join(lines))
    print(f"wrote {out_path}: {len(tracks)} tracks")


if __name__ == "__main__":
    sfx_path = sys.argv[1] if len(sys.argv) > 1 else "sfx/0.sfx"
    out_path = sys.argv[2] if len(sys.argv) > 2 else "core/sfx_data.hpp"

    decompressed = lz4_pod_decompress(sfx_path)
    values, end = read_pxu(decompressed, 0)
    assert end == len(decompressed), f"trailing bytes: {end} != {len(decompressed)}"
    decoded = bytes(v & 0xFF for v in values)
    print(f"decoded {len(decoded)} bytes from {sfx_path}")

    tracks = extract_tracks(decoded)
    print(f"found {len(tracks)} non-empty tracks: {[t[0] for t in tracks]}")
    emit_cpp(tracks, out_path)
