import os, time, utime
import random
from machine import Pin, SPI
from OLED_1in51 import OLED_1in51
from jellyfish import Jellyfish
import json, framebuf
from wifi_clock import WiFiClock


SSID = "your-ssid"
PASSWORD = "your-password"
clock = WiFiClock(SSID, PASSWORD, ntp_server="ntp.nict.jp", timezone_offset=9)
clock.connect_wifi()
clock.sync_time()


class Bubble:
    def __init__(self):
        self._x = 0
        self._y = 0
        self._sleep = 0
        self.reset()

    def reset(self):
        self._x = random.randint(0, 127)
        self._y = 64
        self._sleep = random.randint(1, 10)

    def update(self):
        if self._y == 0:
            self.reset()
        elif self._sleep > 0:
            self._sleep -= 1
        else:
            self._y -= 4

    @property
    def x(self):
        return self._x

    @property
    def y(self):
        return self._y


def load_bmp_images(folderpath):
    images = []
    files = os.listdir(folderpath)
    for file in sorted(files):
        if file.lower().endswith(".bmp"):
            try:
                with open(folderpath + "/" + file, "rb") as f:
                    f.read(0x3E)  # Skip the bitmap header
                    raw_data = f.read()
                    # Convert the bitmap image data to MONO_HLSB format
                    img_data = bytearray()
                    for i in range(len(raw_data) - 4, -1, -4):
                        img_data.append(raw_data[i] ^ 0xFF)
                        img_data.append(raw_data[i + 1] ^ 0xFF)
                    fb = framebuf.FrameBuffer(img_data, 16, 16, framebuf.MONO_HLSB)
                    images.append(fb)
            except Exception as e:
                print(f"Image loading error: {file}, {e}")
    return images


try:
    disp = OLED_1in51()
    print("\r1.51inch OLED ")
    # Initialize library.
    disp.Init()
    # Clear display.
    ("clear display")
    disp.clear()

    with open("/pic/jellyfish.json", "r") as f:
        jellyfish_images_order = json.load(f)
    jellyfish_images = load_bmp_images("/pic")

    jellyfish1 = Jellyfish(jellyfish_images, jellyfish_images_order)
    jellyfish2 = Jellyfish(jellyfish_images, jellyfish_images_order, 3)
    bubble1 = Bubble()
    bubble2 = Bubble()

    buffer = bytearray(disp.width * disp.height // 8)
    fb = framebuf.FrameBuffer(buffer, disp.width, disp.height, framebuf.MONO_VLSB)

    lasttime = utime.ticks_ms()
    while True:
        fb.fill(0)
        # Draw jellyfish
        jellyfish1.update()
        fb.blit(jellyfish1.get_image(), jellyfish1.x, jellyfish1.y)
        jellyfish2.update()
        fb.blit(jellyfish2.get_image(), jellyfish2.x, jellyfish2.y)
        # Draw bubble
        bubble1.update()
        fb.pixel(bubble1.x, bubble1.y, 1)
        bubble2.update()
        fb.pixel(bubble2.x, bubble2.y, 1)
        # Draw time
        t = clock.get_local_time()
        time_str = "{:02}:{:02}".format(t[3], t[4])
        fb.text(time_str, 0, 0)
        # Show Image
        disp.ShowImage(buffer)
        # Wait 400ms
        sleeptime = 400 - (utime.ticks_ms() - lasttime)
        utime.sleep_ms(400)
        lasttime = utime.ticks_ms()

except KeyboardInterrupt:
    disp.module_exit()
    print("Exiting...")
