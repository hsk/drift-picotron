# db.py : picotron-like runtime backed by pygame
import math
import numpy as np
import pygame

SCREEN_W = 480
SCREEN_H = 270

# Picotron default (WIP v5) 32-color palette. Indices 32-63 are unused by
# the game's color table but padded so palette-slot math (0x00-0x3f) never
# goes out of range; 0x3f is the conventional transparent sentinel.
PALETTE = [
    (0x00, 0x00, 0x00), (0x1d, 0x2b, 0x53), (0x7e, 0x25, 0x53), (0x00, 0x87, 0x51),
    (0xab, 0x52, 0x36), (0x5f, 0x57, 0x4f), (0xc2, 0xc3, 0xc7), (0xff, 0xf1, 0xe8),
    (0xff, 0x00, 0x4d), (0xff, 0xa3, 0x00), (0xff, 0xec, 0x27), (0x00, 0xe4, 0x36),
    (0x29, 0xad, 0xff), (0x83, 0x76, 0x9c), (0xff, 0x77, 0xa8), (0xff, 0xcc, 0xaa),
    (0x1c, 0x5e, 0xac), (0x00, 0xa5, 0xa1), (0x75, 0x4e, 0x97), (0x12, 0x53, 0x59),
    (0x74, 0x2f, 0x29), (0x49, 0x2d, 0x38), (0xa2, 0x88, 0x79), (0xff, 0xac, 0xc5),
    (0xc3, 0x00, 0x4c), (0xeb, 0x6b, 0x00), (0x90, 0xec, 0x42), (0x00, 0xb2, 0x51),
    (0x64, 0xdf, 0xf6), (0xbd, 0x9a, 0xdf), (0xe4, 0x0d, 0xab), (0xff, 0x85, 0x6d),
] + [(0xff, 0x00, 0xff)] * 32  # 32-63: unused, flagged magenta so misuse is obvious

PALETTE_ARR = np.array(PALETTE, dtype=np.uint8)

app = None
sp = None
bg = None
spr_bank = {}

pal_map = list(range(64))
palt_flags = [False] * 64

screen = None  # main framebuffer (Userdata), created by init()

_keydown_chars = set()
_btn_keymap = {
    0: pygame.K_LEFT,
    1: pygame.K_RIGHT,
    2: pygame.K_UP,
    3: pygame.K_DOWN,
    4: pygame.K_z,
    5: pygame.K_x,
    6: pygame.K_ESCAPE,
}


class Userdata:
    """Indexed-color 2D pixel buffer, analogous to Picotron's userdata('u8', w, h)."""

    def __init__(self, dtype, w, h):
        self.dtype = dtype
        self.w = w
        self.h = h
        self.data = np.zeros((h, w), dtype=np.uint8)

    def set(self, x, y, c):
        x = int(x)
        y = int(y)
        if 0 <= x < self.w and 0 <= y < self.h:
            self.data[y, x] = c & 0xff

    def get(self, x, y):
        x = int(x)
        y = int(y)
        if 0 <= x < self.w and 0 <= y < self.h:
            return int(self.data[y, x])
        return 0


def userdata(dtype, w, h):
    return Userdata(dtype, w, h)


def set_spr(n, ud):
    spr_bank[n] = ud


def get_spr(n):
    return spr_bank.get(n)


# -- palette control --------------------------------------------------------

def pal(a=None, b=None):
    if a is None:
        pal_map[:] = list(range(64))
    else:
        pal_map[a] = b


def palt(idx=None, flag=None):
    if idx is None:
        return
    palt_flags[idx] = bool(flag)


# -- picotron-style trig (turns, y-flipped like pico-8/picotron) ------------

def sin(x):
    return -math.sin(x * 2 * math.pi)


def cos(x):
    return math.cos(x * 2 * math.pi)


def ceil(x):
    return math.ceil(x)


# -- drawing ------------------------------------------------------------

def cls(c=0):
    screen.data.fill(c & 0xff)


def _plot(x, y, c):
    if palt_flags[c]:
        return
    if 0 <= x < SCREEN_W and 0 <= y < SCREEN_H:
        screen.data[y, x] = pal_map[c] & 0xff


def sspr(sprite, sx, sy, sw, sh, dx, dy, dw, dh, flip_x=False):
    """Axis-aligned scaled (optionally x-flipped) sprite blit."""
    if sprite is None or dw <= 0 or dh <= 0 or sw <= 0 or sh <= 0:
        return
    dx0 = int(round(dx))
    dy0 = int(round(dy))
    dw_i = max(1, int(round(dw)))
    dh_i = max(1, int(round(dh)))
    for j in range(dh_i):
        ty = dy0 + j
        if ty < 0 or ty >= SCREEN_H:
            continue
        v = sy + (j * sh) // dh_i
        for i in range(dw_i):
            tx = dx0 + i
            if tx < 0 or tx >= SCREEN_W:
                continue
            su = (i * sw) // dw_i
            u = sx + (sw - 1 - su if flip_x else su)
            c = sprite.get(u, v)
            _plot(tx, ty, c)


def tline3d(sprite, x0, y0, x1, y1, u0, v0, u1, v1, mx=1, my=1):
    """Textured horizontal(-ish) scanline; sp.py always calls with y0 == y1."""
    if sprite is None:
        return
    x0i = int(round(x0))
    x1i = int(round(x1))
    y = int(round(y0))
    if y < 0 or y >= SCREEN_H:
        return
    n = x1i - x0i
    if n == 0:
        c = sprite.get(int(round(u0)), int(round(v0)))
        _plot(x0i, y, c)
        return
    step = 1 if n > 0 else -1
    steps = abs(n)
    for i in range(0, steps + 1):
        x = x0i + i * step
        if x < 0 or x >= SCREEN_W:
            continue
        t = i / steps
        u = int(round(u0 + (u1 - u0) * t))
        v = int(round(v0 + (v1 - v0) * t))
        c = sprite.get(u, v)
        _plot(x, y, c)


# -- background / tilemap (unused by the game's actual render path, kept as
#    a light stand-in since bg.lua's bgdraw() is dead code in app_draw) ----

def mset(x, y, c):
    pass


def map(x, y, w, h):
    pass


# -- input --------------------------------------------------------------

def btn(i):
    keys = pygame.key.get_pressed()
    k = _btn_keymap.get(i)
    return bool(k is not None and keys[k])


def keyp(ch):
    return ch in _keydown_chars


# -- run loop -------------------------------------------------------------

def init(scale=3, caption="drift-picotron"):
    global screen, _window, _scale
    pygame.init()
    _scale = scale
    _window = pygame.display.set_mode((SCREEN_W * scale, SCREEN_H * scale))
    pygame.display.set_caption(caption)
    screen = Userdata('u8', SCREEN_W, SCREEN_H)


def present():
    rgb = PALETTE_ARR[screen.data]  # (H, W, 3)
    surf = pygame.surfarray.make_surface(rgb.swapaxes(0, 1))
    if _scale != 1:
        surf = pygame.transform.scale(surf, (SCREEN_W * _scale, SCREEN_H * _scale))
    _window.blit(surf, (0, 0))
    pygame.display.flip()


def save_png(path):
    """Dump the current framebuffer (native 480x270, no scaling) to a PNG for
    offline inspection of sprite/palette output."""
    rgb = PALETTE_ARR[screen.data]
    surf = pygame.surfarray.make_surface(rgb.swapaxes(0, 1))
    pygame.image.save(surf, path)


def run(init_fn, update_fn, draw_fn, scale=3, caption="drift-picotron"):
    init(scale, caption)
    init_fn()
    clock = pygame.time.Clock()
    running = True
    while running:
        _keydown_chars.clear()
        for e in pygame.event.get():
            if e.type == pygame.QUIT:
                running = False
            elif e.type == pygame.KEYDOWN:
                if e.unicode:
                    _keydown_chars.add(e.unicode)
        update_fn()
        draw_fn()
        present()
        clock.tick(60)
    pygame.quit()
