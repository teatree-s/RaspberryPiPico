from machine import Pin, I2C
import utime
import ssd1306
import framebuf,sys
# import imgfile
import imglogo
import font10
import writer

# i2c = I2C(0,sda=Pin(0),scl=Pin(1),freq=400000)
i2c = machine.I2C(1,sda=Pin(2),scl=Pin(3),freq=400000)

display = ssd1306.SSD1306_I2C(128, 64, i2c)

# draw Ras Pi Pico
display.fill(0)
data = 0x20
for y in range(0, 64, 8):
    for x in range(0, 128, 8):
        display.text(chr(data), x, y)
        data = data + 1
        if data == 0x7F:
            data = 0x20
display.show() 

utime.sleep(10)

# draw Ras Pi Pico
font_writer = writer.Writer(display, font10)

buffer,img_res = imglogo.get_img() # get the image byte array

# frame buff types: GS2_HMSB, GS4_HMSB, GS8, MONO_HLSB, MONO_VLSB, MONO_HMSB, MVLSB, RGB565
fb = framebuf.FrameBuffer(buffer, img_res[0], img_res[1], framebuf.MONO_HMSB) # MONO_HLSB, MONO_VLSB, MONO_HMSB

display.fill(0) # clear the OLED
display.blit(fb, 0, 6) # show the image at location (x=0,y=0)

start_x = 72 # start point for text in x-dir
start_y = 8 # start point for text in y-dir
lineskip = 16 # space between text in y-dir
txt_array = ['Ras','Pi','Pico'] # text array
for iter_ii,txt in enumerate(txt_array):
    font_writer.set_textpos(start_x, start_y+(iter_ii*lineskip))
    font_writer.printstring(txt)
#    display.text(txt,start_x,start_y+(iter_ii*lineskip)) # add text at (start_x,start_y)

display.show() # show the new text and image

while True:
    utime.sleep(1)
