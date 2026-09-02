# bg.py : bg library
import db

class Bg:
    def __init__(self):
        self.screen_w = 0
        self.screen_h = 0
        self.ofs_x = 0
        self.ofs_y = 0
        self.ofs_z = 0
        self.home_x = 0
        self.home_y = 0
        self.scale_x = 1
        self.scale_y = 1
        self.rot = 0
        self.show = False

# initialize bg
def bginit():
    if not db.bg:
        db.bg = Bg()

# set bg screen size
def bgscreen(layer, w, h):
    db.bg.screen_w = w
    db.bg.screen_h = h

# set bg offset
def bgofs(layer, x, y, z):
    db.bg.ofs_x = x
    db.bg.ofs_y = y
    db.bg.ofs_z = z

# set bg home
def bghome(layer, x, y):
    db.bg.home_x = x
    db.bg.home_y = y

# set bg scale
def bgscale(layer, x, y):
    db.bg.scale_x = x
    db.bg.scale_y = y

# set bg rotate
def bgrot(layer, r):
    db.bg.rot = r

# put bg character
def bgput(layer, x, y, c):
    db.mset(x, y, c)

# fill bg character
def bgfill(layer, x0, y0, x1, y1, c):
    for y in range(y0, y1):
        for x in range(x0, x1):
            db.mset(x, y, c)

# show bg
def bgshow(layer):
    db.bg.show = True

# hide bg
def bghide(layer):
    db.bg.show = False

# draw bg
def bgdraw():
    bg = db.bg
    if bg.show:
        db.map(0, 0, bg.ofs_x - bg.home_x, bg.ofs_y - bg.home_y)
