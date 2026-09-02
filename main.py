# main.py : entry point
from app import app_init, app_update, app_draw

# called once when the program starts
def _init():
    # initialize application
    app_init()

# called at 60fps
def _update():
    # update application
    app_update()

# called whenever a frame is drawn
def _draw():
    # draw application
    app_draw()
