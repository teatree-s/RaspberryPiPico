# /*****************************************************************************
# * | File        :	  OLED_1in51.py
# * | Function    :   Driver for OLED_1in51
# * | Info        :
# ******************************************************************************/
import time
import framebuf
from config import RaspberryPiPico

OLED_WIDTH = 128
OLED_HEIGHT = 64


class OLED_1in51(RaspberryPiPico):
    def __init__(self):
        super().__init__()

        self.width = OLED_WIDTH
        self.height = OLED_HEIGHT

        self.buffer = bytearray(self.width * self.height // 8)
        self.fb = framebuf.FrameBuffer(
            self.buffer, self.width, self.height, framebuf.MONO_VLSB
        )

        self.Init()

    def command(self, cmd):
        self.digital_write(self.DC_PIN, 0)
        self.spi_writebyte([cmd])

    def Init(self):
        self.digital_write(self.RST_PIN, 1)
        time.sleep(0.1)
        self.digital_write(self.RST_PIN, 0)
        time.sleep(0.1)
        self.digital_write(self.RST_PIN, 1)
        time.sleep(0.1)

        self.command(0xAE)
        self.command(0xD5)
        self.command(0x80)
        self.command(0xA8)
        self.command(0x3F)
        self.command(0xD3)
        self.command(0x00)
        self.command(0x40)
        self.command(0xA1)
        self.command(0xC8)
        self.command(0xDA)
        self.command(0x12)
        self.command(0x81)
        self.command(0x7F)
        self.command(0xA4)
        self.command(0xA6)
        self.command(0xD5)
        self.command(0x80)
        self.command(0x8D)
        self.command(0x14)
        self.command(0xAF)

    def getbuffer(self, image):
        image_monocolor = image.convert("1")
        imwidth, imheight = image_monocolor.size
        pixels = image_monocolor.load()

        for y in range(imheight):
            for x in range(imwidth):
                if pixels[x, y] == 0:
                    self.fb.pixel(x, y, 1)
                else:
                    self.fb.pixel(x, y, 0)
        return self.buffer

    def ShowImage(self, buffer):
        for page in range(8):
            self.command(0xB0 + page)
            self.command(0x00)
            self.command(0x10)
            self.digital_write(self.DC_PIN, 1)
            self.spi_writebyte(buffer[page * self.width : (page + 1) * self.width])

    def clear(self):
        self.fb.fill(0)
        self.ShowImage(self.buffer)
