import ctypes
import time
from sdl2 import *

# Sample Code
# https://stackoverflow.com/a/50572903 used as reference

class Joystick:
    def __init__(self):
        SDL_Init(SDL_INIT_JOYSTICK)
        self.axis = {}
        self.button = {}

    def update(self):
        event = SDL_Event()
        while SDL_PollEvent(ctypes.byref(event)) != 0:
            if event.type == SDL_JOYDEVICEADDED:
                self.device = SDL_JoystickOpen(event.jdevice.which)
            elif event.type == SDL_JOYAXISMOTION:
                self.axis[event.jaxis.axis] = event.jaxis.value
            elif event.type == SDL_JOYBUTTONDOWN:
                self.button[event.jbutton.button] = True
            elif event.type == SDL_JOYBUTTONUP:
                self.button[event.jbutton.button] = False


def main() -> None:
    joystick = Joystick()
    while True:
        joystick.update()
        time.sleep(0.05)
        print(joystick.axis)
        print(joystick.button)
        # 3: B button on Steam Controller (2015)
        if (joystick.button.get(3) == True):
            print("SEAGULLS!!!")
            break
