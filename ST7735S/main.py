from machine import Pin, SPI
from ST7735s import TFT,TFTColor
from sysfont import sysfont
import os, sdcard
import time, utime

print("start\n")

# TFT
spi0 = SPI(0, baudrate=20000000, polarity=0, phase=0,
        sck=Pin(6), mosi=Pin(7), miso=None)

tft=TFT(spi0,0,1,2)
tft.initb80x160()
tft.rotation(1)
tft.invertcolor(True)
tft.fill(TFT.BLACK)

# width = 128
# height = 160
width = 80
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

# Read files
files = os.listdir(dir)
file_count = len(files)
print("file_count:", file_count)
print("")

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
                        color = TFTColor(bgr[i], bgr[i+1], bgr[i+2])
                        #color = ((bgr[i+2] & 0xF8) << 8) | ((bgr[i+1] & 0xFC) << 3) | (bgr[i] >> 3)
                        colordatas[j] = color >> 8
                        j += 1
                        colordatas[j] = color
                        j += 1
                tft._writedata(colordatas)
                print("writedata time:", utime.ticks_diff(utime.ticks_us(), start_time)/1000)

def print_string():
    h = 82
    v = 0
    tft.text((h, v), " The powerful, flexible microcontroller board, available from $4 Raspberry Pi Pico is a tiny, fast, and versatile board built using RP2040", TFT.WHITE, sysfont, 1)
#     v += (sysfont["Height"] + 1) * 5
#     tft.text((h, v), " Raspberry Pi Pico is a tiny, fast, and versatile board built using RP2040, the flagship microcontroller chip designed by Raspberry Pi in the UK", TFT.WHITE, sysfont, 1)

# Main loop
i = 0
while True:
    if i % 2:
        tft.rotation(1)
        tft._offset[0] = 2
        tft._offset[1] = 2 + 24
    else:
        tft.rotation(0)
        tft._offset[0] = 2 + 24
        tft._offset[1] = 1
        
    tft.fill(TFT.BLACK)
    tft._setwindowloc((0,0),(width - 1,height - 1))
    
    file_path = dir + '/' + files[i]
    print("file_path:", file_path)
    draw_start_time = utime.ticks_ms()
    if i % 2:
        # draw_bmp_image
        draw_bmp_image(file_path)
        # print_string
        print_string()
    else:
        # draw_bmp_image
        draw_bmp_image(file_path)
    # wait
    if sd_access:
        wait_time = 3000
    else:
        wait_time = 1000
    wait_time -= utime.ticks_diff(utime.ticks_ms(), draw_start_time)
#     utime.sleep_ms(wait_time)
    utime.sleep_ms(5000)
    print("wait_time:", wait_time)
    print("")
    
    i += 1
    if i >= file_count:
        i = 0

spi1.deinit()
spi0.deinit()

