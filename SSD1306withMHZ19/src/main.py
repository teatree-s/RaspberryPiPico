from machine import Pin, I2C
import time
import ssd1306
from mhz19 import MHZ19UART
import writer
import font10

i2c = machine.I2C(1, sda=Pin(2), scl=Pin(3), freq=400000)
display = ssd1306.SSD1306_I2C(128, 64, i2c)
display.fill(0)
wri = writer.Writer(display, font10)
wri.printstring("    CO2 Sensor\n")
wri.printstring("---------------------")
display.show()

if False:  # demo
    display.fill(0)
    data = 0x20
    for y in range(0, 64, 8):
        for x in range(0, 128, 8):
            display.text(chr(data), x, y)
            data = data + 1
            if data == 0x7F:
                data = 0x20
    display.show()
    time.sleep(10)

sensor = MHZ19UART(tx_pin=0, rx_pin=1)
sensor.set_auto_calibration(False)
sensor.uart.read()

print("start.")
for i in range(1, 6):
    wri.set_textpos(display, 40, 0)
    wri.printstring("  " + str(i))
    display.show()
    time.sleep(1)
print("done.")

while True:
    co2, temp = sensor.get_ppm()
    print("MHZ19:", co2, "ppm", temp, "C")
    wri.set_textpos(display, 40, 0)
    wri.printstring("  " + str(co2) + " ppm ")
    wri.printstring(str(temp) + " C  ")
    display.show()
    time.sleep(1)
