from machine import Pin, SPI
from ST7735 import TFT,TFTColor
import os, sdcard
import time, utime
import uos

print("start\n")

# TFT
spi0 = SPI(0, baudrate=20000000, polarity=0, phase=0,
        sck=Pin(6), mosi=Pin(7), miso=None)

tft=TFT(spi0,0,1,2)
tft.initr()
tft.rgb(True)
tft.fill(TFT.BLACK)

width = 128
height = 160
tft._setwindowloc((0,0),(width - 1,height - 1))

# SD Card
spi1 = SPI(1, baudrate=42000000,
        sck=Pin(14), mosi=Pin(15), miso=Pin(12))
sd_access = 1

try:
    sd = sdcard.SDCard(spi1, Pin(13))
    os.mount(sd, '/sd')
except:
    sd_access = 0
print("sd_access:", sd_access)

if sd_access:
    dir = '/sd/bmp'
else:
    dir = './bmp'

# Draw bitmap
def draw_bmp_image(file_name):
    f=open(file_name, 'rb')
    if f.read(2) == b'BM':  #header
        dummy = f.read(8) #file size(4), creator bytes(4)
        offset = int.from_bytes(f.read(4), 'little')
        hdrsize = int.from_bytes(f.read(4), 'little')
        width = int.from_bytes(f.read(4), 'little')
        height = int.from_bytes(f.read(4), 'little')
        if int.from_bytes(f.read(2), 'little') == 1: #planes must be 1
            depth = int.from_bytes(f.read(2), 'little')
            if depth == 24 and int.from_bytes(f.read(4), 'little') == 0:#compress method == uncompressed
                print("image size:", width, "x", height)
                rowsize = (width * 3 + 3) & ~3
                if height < 0:
                    height = -height
                    flip = False
                else:
                    flip = True
                w, h = width, height
                if w > 128: w = 128
                if h > 160: h = 160
                start_time = utime.ticks_us()
                colordatas = bytearray(2*width*height)
                j = 0
                for row in range(h):
                    if flip:
                        pos = offset + (height - 1 - row) * rowsize
                    else:
                        pos = offset + row * rowsize
                    if f.tell() != pos:
                        dummy = f.seek(pos)
                    bgr = f.read(3*w)
                    for col in range(w):
                        i = col*3
                        color = ((bgr[i+2] & 0xF8) << 8) | ((bgr[i+1] & 0xFC) << 3) | (bgr[i] >> 3)
                        colordatas[j] = color >> 8
                        j += 1
                        colordatas[j] = color
                        j += 1
                tft._writedata(colordatas)
                print("writedata time:", utime.ticks_diff(utime.ticks_us(), start_time)/1000)

# Read files
files = os.listdir(dir)
file_count = len(files)
print("file_count:", file_count)
print("")

# Main loop
i = 0
while True:
    file_path = dir + '/' + files[i]
    print("file_path:", file_path)
    draw_start_time = utime.ticks_ms()
    draw_bmp_image(file_path)
    if sd_access:
        wait_time = 3000
        wait_time -= utime.ticks_diff(utime.ticks_ms(), draw_start_time)
        utime.sleep_ms(wait_time)
    else:
        wait_time = 1000
        wait_time -= utime.ticks_diff(utime.ticks_ms(), draw_start_time)
    utime.sleep_ms(wait_time)
    print("wait_time:", wait_time)
    print("")
    
    i += 1
    if i >= file_count:
        i = 0

spi1.deinit()
spi0.deinit()
