import ast, re, sys

path = "/private/tmp/claude-501/-Users-hiroshisakurai-Downloads-basic-drift-picotron/9363d073-bfa9-4b56-b323-3fbc7858cc98/scratchpad/chara_ref.py"
src = open(path, encoding="utf-8").read()
tree = ast.parse(src)

def find_assign(func_name, var_name):
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name == func_name:
            for stmt in node.body:
                if isinstance(stmt, ast.Assign):
                    if any(isinstance(t, ast.Name) and t.id == var_name for t in stmt.targets):
                        code = compile(ast.Expression(stmt.value), "<data>", "eval")
                        return eval(code, {"__builtins__": {}})
    raise SystemExit(f"not found: {func_name}/{var_name}")

spr_data = find_assign("chinitspr", "data")
bg_data = find_assign("chinitbg", "data")

def group(flat, scale_uv=1, scale_wh=1):
    entries = []
    i = 0
    while flat[i] != -1:
        u = flat[i+0]*scale_uv; v = flat[i+1]*scale_uv; w = flat[i+2]*scale_wh; h = flat[i+3]*scale_wh
        i += 4
        s = flat[i]; i += 1
        rows = []
        for _ in range(h // (scale_wh) if scale_wh else h):
            rows.append(flat[i]); i += 1
        entries.append((u, v, w, h, s, rows))
    return entries

# for chinitspr: no scaling of u,v,w,h; rows count = h
spr_entries = []
i = 0
while spr_data[i] != -1:
    u, v, w, h = spr_data[i:i+4]
    i += 4
    s = spr_data[i]; i += 1
    rows = []
    for _ in range(h):
        rows.append(spr_data[i]); i += 1
    spr_entries.append((u, v, w, h, s, rows))

# chinitbg scales the raw (u,v,w,h) tuple by (16,16,8,8); the row COUNT is
# the scaled h (h_raw*8), matching the original Lua/Python loop bounds.
bg_entries = []
i = 0
while bg_data[i] != -1:
    u0, v0, w0, h0 = bg_data[i:i+4]
    u, v, w, h = u0 * 16, v0 * 16, w0 * 8, h0 * 8
    i += 4
    s = bg_data[i]; i += 1
    rows = []
    for _ in range(h):
        rows.append(bg_data[i]); i += 1
    bg_entries.append((u, v, w, h, s, rows))

def emit(name, entries):
    lines = []
    lines.append(f"inline const SpriteDef {name}[] = {{")
    for (u, v, w, h, s, rows) in entries:
        row_lits = ", ".join(f'"{r}"' for r in rows)
        lines.append(f'    {{ {u}, {v}, {w}, {h}, "{s}", {{ {row_lits} }} }},')
    lines.append("};")
    lines.append(f"inline constexpr int {name}_count = {len(entries)};")
    return "\n".join(lines)

out = []
out.append("// AUTO-GENERATED from chara.py (pygame-port branch) by tools/gen_chara_cpp.py")
out.append("// Do not hand-edit; regenerate instead.")
out.append("#pragma once")
out.append("")
out.append("struct SpriteDef {")
out.append("    int u, v, w, h;")
out.append("    const char* colors;")
out.append("    const char* rows[32];")
out.append("};")
out.append("")
out.append(emit("kCharaSprites", spr_entries))
out.append("")
out.append(emit("kCharaBg", bg_entries))
out.append("")

print(f"spr_entries={len(spr_entries)} bg_entries={len(bg_entries)}", file=sys.stderr)
max_rows = max(len(r[5]) for r in spr_entries + bg_entries)
print(f"max_rows={max_rows}", file=sys.stderr)

open("/private/tmp/claude-501/-Users-hiroshisakurai-Downloads-basic-drift-picotron/9363d073-bfa9-4b56-b323-3fbc7858cc98/scratchpad/chara_data.gen.hpp", "w").write("\n".join(out))
print("done", file=sys.stderr)
