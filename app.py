# app.py : application
import db
from sp import spinit, spdraw
from bg import bginit
from color import clinit
from chara import chinit
"""
from pause import psinit, psloop, psdraw
from title import tlinit, tlloop
from game import gminit
from race import rcinit
from cam import cminit
from course import csinit
from obj import obinit
from car import cainit
from rival import rvinit
from mycar import myinit
from signal import sginit
from rank import rkinit
from clear import gcinit
from over import goinit
from navi import nvinit
from cockpit import cpinit
from back import bkinit
"""
class App:
    def __init__(self):
        self.bp_ = 0     # bp_
        self.br_ = 0     # br_
        self.be_ = 0     # be_
        self.sx_ = 0     # sx_
        self.sy_ = 0     # sy_
        self.ox_ = 0     # ox_
        self.oy_ = 0     # oy_
        self.ow_ = 0     # ow_
        self.oh_ = 0     # oh_
        self.or_ = 0     # or_
        self.zn_ = 0     # zn_
        self.zf_ = 0     # zf_
        self.color_ = 0  # color_[64]
        self.gpage0_ = 0
        self.gpage1_ = 0
        self.gpage2_ = 0
        self.gpage3_ = 0
        self.pause_ = 0  # pause_
        self.debug_ = 0  # debug_
        self.state_ = 0  # state_
        self.proc_ = 0   # proc_$
        self.cycle_ = 0
        self.vsync_ = 0
        self.sound_ = 0

# create application instance
def app_init():

    # setup application
    db.palt(0x00, False) # 不透明
    db.palt(0x3f, True) # 透明

    # instance
    if not db.app:
        db.app = App()
    # initialize application vars
    db.app.bp_ = 0
    db.app.br_ = 0
    db.app.be_ = 0
    db.app.bl_ = 0
    db.app.sx_ = 0
    db.app.sy_ = 0
    db.app.ox_ = 200 + 40 # 200
    db.app.oy_ = 160 + 15 # 160
    db.app.ow_ = 200 + 40 # 200
    db.app.oh_ = 80 + 15 # 80
    db.app.or_ = 256
    db.app.zn_ = 2
    db.app.zf_ = 48
    db.app.color_ = None
    db.app.gpage2_ = None
    db.app.gpage3_ = None
    db.app.pause_ = 0
    db.app.debug_ = 0
    db.app.state_ = 0
    db.app.proc_ = tlloop
    db.app.cycle_ = 0
    db.app.vsync_ = 2
    db.app.sound_ = True

    # initialize libraries
    spinit()
    bginit()

    # initialize others
    clinit()
    """
    chinit()
    psinit()
    tlinit()
    gminit()
    rcinit()
    cminit()
    csinit()
    obinit()
    cainit()
    rvinit()
    myinit()
    sginit()
    rkinit()
    gcinit()
    goinit()
    nvinit()
    cpinit()
    bkinit()
    """
# update
def app_update():

    # switch speed
    if db.keyp('1'):
        db.app.vsync_ = 1
    if db.keyp('2'):
        db.app.vsync_ = 2

    # update cycle
    db.app.cycle_ = db.app.cycle_ + 1
    if db.app.cycle_ >= db.app.vsync_:
        db.app.cycle_ = 0

    # update button
    b = 0b00000000
    """
        PICOTRON:
            0 1 2 3     LEFT RIGHT UP DOWN
            4 5         Buttons: O X
            6           MENU
            7           reserved
            8 9 10 11   Secondary Stick L,R,U,D
            12 13       Buttons (not named yet!)
            14 15       SL SR    # update proc
        SMILE BASIC:
            b00      UP
            b01  DOWN
            b02  LEFT
            b03  RIGHT
            b04  A
            b05  B
            b06  X
            b07  Y
            b08  L
            b09  R
            b10
            b11  ZL
            b12  ZR
    """
    b2b = [
        0b00000100,
        0b00001000,
        0b00000001,
        0b00000010,
        0b00010000,
        0b00100000,
        0b01000000,
        0b00000000,
    ]
    for i in range(1, len(b2b)):
        if db.btn(i - 1):
            b = b | b2b[i]

    # control pause
    if db.keyp('p'):
        db.app.pause_ = 1 - db.app.pause_

    # update frame
    if db.app.cycle_ == 0:
        # control button
        b = b | db.app.bl_
        db.app.be_ = (b ^ db.app.bp_) & b
        db.app.bp_ = b
        db.app.bl_ = 0
        if (db.app.bp_ & 0b00000100) != 0:
            db.app.sx_ = -1
        elif (db.app.bp_ & 0b00001000) != 0:
            db.app.sx_ = 1
        else:
            db.app.sx_ = 0
            if (db.app.bp_ & 0b00000001) != 0:
                db.app.sy_ = -1
            elif (db.app.bp_ & 0b00000010) != 0:
                db.app.sy_ = 1
            else:
                db.app.sy_ = 0

        # update scene
        if db.app.pause_ == 0:
            db.app.proc_()
        """
        # update pause
        psloop()
        psdraw()
        """
    # skip frame
    else:
        # update button
        db.app.bl_ = b

# draw
def app_draw():
    # draw frame
    if db.app.cycle_ == 0:
        # clear screen
        # cls(0x01)
        # draw bg
        # bgdraw()
        # draw sprites
        """
        spdraw()
        """
        # debug
        # spr(db.app.gpage0_, 0, 0)
        # spr(db.app.gpage1_, 0, 0)
        # spr(db.app.gpage2_, 0, 0)
        # spr(db.app.gpage3_, 0, 0)
