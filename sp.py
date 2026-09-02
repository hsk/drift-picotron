# sp.py : sprite library
import db

class Sp:
    def __init__(self):
        self.chr_u = 0
        self.chr_v = 0
        self.chr_w = 16
        self.chr_h = 16
        self.chr_a = 0x00
        self.ofs_x = 0
        self.ofs_y = 0
        self.ofs_z = 0
        self.home_x = 0
        self.home_y = 0
        self.scale_x = 1
        self.scale_y = 1
        self.rot = 0
        self.color = 0x07
        self.page = app.gpage2_
        self.show = False

# initialize sprite
def spinit():
    if not db.sp:
        db.sp = []
    sprset()

# reset sprite
def sprset():
    for i in range(512):
        db.sp[i] = Sp()

# set sprite character
def spchr(n, u, v, w, h, a):
    sp = db.sp
    sp[n].chr_u = u
    sp[n].chr_v = v
    sp[n].chr_w = w
    sp[n].chr_h = h
    sp[n].chr_a = a

# set sprite offset
def spofs(n, x, y, z):
    sp = db.sp
    sp[n].ofs_x = x
    sp[n].ofs_y = y
    sp[n].ofs_z = z

# set sprite home
def sphome(n, x, y):
    sp = db.sp
    sp[n].home_x = x
    sp[n].home_y = y

# set sprite scale
def spscale(n, x, y):
    sp = db.sp
    sp[n].scale_x = x
    sp[n].scale_y = y

# set sprite rotate
def sprot(n, r):
    sp = db.sp
    sp[n].rot = r / 360

# set sprite color
def spcolor(n, c):
    sp = db.sp
    sp[n].color = c

# set sprite page
def sppage(n, p):
    sp = db.sp
    sp[n].page = p

# show sprite
def spshow(n):
    sp = db.sp
    sp[n].show = True

# hide sprite
def sphide(n):
    sp = db.sp
    sp[n].show = False

# draw scale sprite
def spsspr(sprite, u, v, w, h, x, y, offset_x, offset_y, scale_x, scale_y, a):
    flip_x = False
    if (a & 0b00001000) != 0:
        flip_x = True
    sspr(
        sprite,
        u,
        v,
        w,
        h,
        x - offset_x * scale_x,
        y - offset_y * scale_y,
        w * scale_x,
        h * scale_y,
        flip_x
    )
class UV:
    def __init__(u,v):
        self.u = u
        self.v = v
class XY:
    def __init__(x,y):
        self.x = x
        self.y = y

# draw scale & rotate sprite
def sprspr(sprite, u, v, w, h, x, y, offset_x, offset_y, scale_x, scale_y, rotate, a):

    # [1] - [2]
    #  |     |
    # [3] - [4]

    # calc uv vectors
    uv = [
        UV(u,v),
        UV(u + w - 1, v),
        UV(u, v + h - 1),
        UV(u + w - 1, v = v + h - 1),
    ]
    if (a & 0b00001000) != 0:
        uv[1].u, uv[2].u = uv[2].u, uv[1].u
        uv[3].u, uv[4].u = uv[4].u, uv[3].u

    # calc xy vectors
    sin_r = sin(rotate)
    cos_r = cos(rotate)
    x0 = -offset_x
    y0 = -offset_y
    x1 = x0 + w
    y1 = y0 + h
    x0 = x0 * scale_x
    y0 = y0 * scale_y
    x1 = x1 * scale_x - 1
    y1 = y1 * scale_y - 1
    xy = [
        XY(ceil((x0 * cos_r - y0 * sin_r) + x),
           ceil((x0 * sin_r + y0 * cos_r) + y)),
        XY(ceil((x1 * cos_r - y0 * sin_r) + x),
           ceil((x1 * sin_r + y0 * cos_r) + y)),
        XY(ceil((x0 * cos_r - y1 * sin_r) + x),
           ceil((x0 * sin_r + y1 * cos_r) + y)),
        XY(ceil((x1 * cos_r - y1 * sin_r) + x),
           ceil((x1 * sin_r + y1 * cos_r) + y)),
    ]

    # calc index
    id = {}
    if xy[1].y <= xy[2].y:
        if xy[1].y < xy[3].y:
            id = [1, 2, 3, 4]
        elif xy[3].y < xy[4].y:
            id = [3, 1, 4, 2]
        else:
            id = [4, 3, 2, 1]

    else:
        if xy[2].y <= xy[4].y:
            id = [2, 4, 1, 3]
        else:
            id = [4, 3, 2, 1]

    # draw sprite
    if xy[id[1]].y == xy[id[2]].y:
        lu = uv[id[3]].u - uv[id[1]].u
        lv = uv[id[3]].v - uv[id[1]].v
        ru = uv[id[4]].u - uv[id[2]].u
        rv = uv[id[4]].v - uv[id[2]].v
        d = xy[id[3]].y - xy[id[1]].y
        for i in range(d):
            x0 = xy[id[1]].x
            x1 = xy[id[4]].x
            y = xy[id[1]].y + i
            if x0 < 480 and x1 >= 0 and y >= 0 and y < 270:
                t = i / d
                tline3d(
                    sprite,
                    x0, # xy[id[1]].x,
                    y,  # xy[id[1]].y + i,
                    x1, # xy[id[4]].x,
                    y,  # xy[id[1]].y + i,
                    lu * t + uv[id[1]].u,
                    lv * t + uv[id[1]].v,
                    ru * t + uv[id[2]].u,
                    rv * t + uv[id[2]].v,
                    1,
                    1
                )
    else:
        ls = id[1]
        le = id[3]
        lu = uv[le].u - uv[ls].u
        lv = uv[le].v - uv[ls].v
        lx = xy[le].x - xy[ls].x
        ly = xy[le].y - xy[ls].y
        li = 0
        rs = id[1]
        re = id[2]
        ru = uv[re].u - uv[rs].u
        rv = uv[re].v - uv[rs].v
        rx = xy[re].x - xy[rs].x
        ry = xy[re].y - xy[rs].y
        ri = 0
        d = xy[id[4]].y - xy[id[1]].y
        for i in range(d):
            lt = li / ly
            rt = ri / ry
            x0 = lx * lt + xy[ls].x
            x1 = rx * rt + xy[rs].x
            y = li + xy[ls].y #ly * lt + xy[ls].y
            if x0 < 480 and x1 >= 0 and y >= 0 and y < 270:
                tline3d(
                    sprite,
                    x0, # lx * lt + xy[ls].x,
                    y,  # ly * lt + xy[ls].y,
                    x1, # rx * rt + xy[rs].x,
                    y,  # ry * rt + xy[rs].y,
                    lu * lt + uv[ls].u,
                    lv * lt + uv[ls].v,
                    ru * rt + uv[rs].u,
                    rv * rt + uv[rs].v,
                    1,
                    1
                )

            li = li + 1
            if li > ly:
                ls = id[3]
                le = id[4]
                lu = uv[le].u - uv[ls].u
                lv = uv[le].v - uv[ls].v
                lx = xy[le].x - xy[ls].x
                ly = xy[le].y - xy[ls].y
                li = 0

            ri = ri + 1
            if ri > ry:
                rs = id[2]
                re = id[4]
                ru = uv[re].u - uv[rs].u
                rv = uv[re].v - uv[rs].v
                rx = xy[re].x - xy[rs].x
                ry = xy[re].y - xy[rs].y
                ri = 0

# sort sprite
def spqsort(t, first, last):
    sp = db.sp
    if first > last:
        return

    p = first
    for i in range(first + 1, last):
        if sp[t[i]].ofs_z > sp[t[first]].ofs_z:
            p = p + 1
            t[p], t[i] = t[i], t[p]

    t[p], t[first] = t[first], t[p]
    spqsort(t, first, p - 1)
    spqsort(t, p + 1, last)

# draw sprite
def spdraw():
    sp = db.sp
    order = []
    for i in range(1, 512):
        if sp[i].show:
            order.append(i)

    spqsort(order, 1, len(order))
    for i in range(len(order)):
        n = order[i]
        if sp[n].color != 0x07:
            db.pal(0x07, sp[n].color)

        if sp[n].rot == 0:
            spsspr(
                sp[n].page,
                sp[n].chr_u,
                sp[n].chr_v,
                sp[n].chr_w,
                sp[n].chr_h,
                sp[n].ofs_x,
                sp[n].ofs_y,
                sp[n].home_x,
                sp[n].home_y,
                sp[n].scale_x,
                sp[n].scale_y,
                sp[n].chr_a
            )
        else:
            sprspr(
                sp[n].page,
                sp[n].chr_u,
                sp[n].chr_v,
                sp[n].chr_w,
                sp[n].chr_h,
                sp[n].ofs_x,
                sp[n].ofs_y,
                sp[n].home_x,
                sp[n].home_y,
                sp[n].scale_x,
                sp[n].scale_y,
                sp[n].rot,
                sp[n].chr_a
            )

        if sp[n].color != 0x07:
            db.pal(0x07, 0x07)
