# /*****************************************************************************
# * | File        :	  config.py
# * | Function    :   Hardware underlying interface,for Raspberry pi pico
# * | Info        :
# ******************************************************************************/
import time
import machine

Device_SPI = 1
Device_I2C = 0


class RaspberryPiPico:
    def __init__(
        self,
        spi_id=0,
        spi_freq=10_000_000,
        rst=20,
        dc=21,
        cs=17,
        sck=18,
        mosi=19,
        miso=16,
    ):
        self.Device = Device_SPI
        self.SPEED = spi_freq

        self.spi = machine.SPI(
            spi_id,
            baudrate=self.SPEED,
            polarity=0,
            phase=0,
            sck=machine.Pin(sck),
            mosi=machine.Pin(mosi),
            miso=machine.Pin(miso),
        )
        self.cs = machine.Pin(cs, machine.Pin.OUT)

        self.RST_PIN = machine.Pin(rst, machine.Pin.OUT)
        self.DC_PIN = machine.Pin(dc, machine.Pin.OUT)

        self.init_module()

    def delay_ms(self, delaytime):
        time.sleep(delaytime / 1000.0)

    def digital_write(self, pin, value):
        pin.value(value)

    def digital_read(self, pin):
        return pin.value()

    def spi_writebyte(self, data):
        self.cs.value(0)
        self.spi.write(bytearray(data))
        self.cs.value(1)

    def init_module(self):
        self.digital_write(self.RST_PIN, 0)
        time.sleep(0.1)
        self.digital_write(self.RST_PIN, 1)
        time.sleep(0.1)
        return 0

    def module_exit(self):
        self.spi.deinit()
        self.digital_write(self.RST_PIN, 0)
        self.digital_write(self.DC_PIN, 0)
