from machine import Pin, SPI
import os, time, utime
from ST7735 import TFT, TFTColor
import sysfont
import network
import urequests

# for TFT
spi0 = SPI(
    0, baudrate=20000000, polarity=0, phase=0, sck=Pin(18), mosi=Pin(19), miso=None
)

tft = TFT(spi0, 21, 20, 17)  # def __init__( self, spi, aDC, aReset, aCS) :
tft.initr()
tft.rgb(True)
tft.fill(TFT.WHITE)

width = 128
height = 160
tft._setwindowloc((0, 0), (width - 1, height - 1))
dir = "./bmp"

# for Wi-Fi
SSID = "your-ssid"
PASSWORD = "your-password"

# Notes
# For Public API user (Demo plan), the rate limit is ~30 calls per minutes and it varies depending on the traffic size.
# You may cross-check the price on CoinGecko and learn more about our price methodology here
# Cache/Update Frequency: every 60 seconds for Public API
ENDPOINT = "https://api.coingecko.com/api/v3"
WAIT_TIME_FOR_API = 60  # sec

cointable = [
    {"id": "bitcoin", "name": "Bitcoin", "symbol": "BTC", "usd": 0.0, "jpy": 0.0},
    {"id": "ethereum", "name": "Ethereum", "symbol": "ETH", "usd": 0.0, "jpy": 0.0},
    {"id": "binancecoin", "name": "BNB", "symbol": "BNB", "usd": 0.0, "jpy": 0.0},
    {"id": "solana", "name": "Solana", "symbol": "SOL", "usd": 0.0, "jpy": 0.0},
    {"id": "dogecoin", "name": "Dogecoin", "symbol": "DOGE", "usd": 0.0, "jpy": 0.0},
]


# get bitmap data
def get_bmp(file_name):
    f = open(file_name, "rb")
    if f.read(2) == b"BM":  # header
        dummy = f.read(8)  # file size(4), creator bytes(4)
        offset = int.from_bytes(f.read(4), "little")
        hdrsize = int.from_bytes(f.read(4), "little")
        width = int.from_bytes(f.read(4), "little")
        height = int.from_bytes(f.read(4), "little")
        if int.from_bytes(f.read(2), "little") == 1:  # planes must be 1
            depth = int.from_bytes(f.read(2), "little")
            if (
                depth == 24 and int.from_bytes(f.read(4), "little") == 0
            ):  # compress method == uncompressed
                print("image size:", width, "x", height)
                rowsize = (width * 3 + 3) & ~3
                if height < 0:
                    height = -height
                    flip = False
                else:
                    flip = True
                w, h = width, height
                if w > 128:
                    w = 128
                if h > 160:
                    h = 160
                colordatas = bytearray(2 * width * height)
                j = 0
                for row in range(h):
                    if flip:
                        pos = offset + (height - 1 - row) * rowsize
                    else:
                        pos = offset + row * rowsize
                    if f.tell() != pos:
                        dummy = f.seek(pos)
                    bgr = f.read(3 * w)
                    for col in range(w):
                        i = col * 3
                        color = (
                            ((bgr[i + 2] & 0xF8) << 8)
                            | ((bgr[i + 1] & 0xFC) << 3)
                            | (bgr[i] >> 3)
                        )
                        colordatas[j] = color >> 8
                        j += 1
                        colordatas[j] = color
                        j += 1
    return width, height, colordatas


def draw_bmp(file_name):
    start_time = utime.ticks_us()
    width, height, colordatas = get_bmp(file_name)
    tft._writedata(colordatas)
    # print("writedata time:", utime.ticks_diff(utime.ticks_us(), start_time)/1000)


def draw_bmp_position(file_name, x, y):
    start_time = utime.ticks_us()
    w, h, colordatas = get_bmp(file_name)
    tft.image(x, y, x + w - 1, y + h - 1, colordatas)
    # print("writedata time:", utime.ticks_diff(utime.ticks_us(), start_time)/1000)


def text(aPos, aString, aColor, aSize=1):
    tft.text(aPos, aString, aColor, sysfont.sysfont, aSize, aBackColor=TFT.WHITE)


def connect_to_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print("Connecting to network...")
        wlan.connect(SSID, PASSWORD)
        while not wlan.isconnected():
            time.sleep(1)
    print("Network config:", wlan.ifconfig())


def get_price(id, currency):
    try:
        response = urequests.get(
            ENDPOINT
            + "/simple/price?ids="
            + id
            + "&vs_currencies="
            + currency
            + "&include_last_updated_at=true"
        )

        print(response.status_code)
        print(response.json())
        data = response.json()
        response.close()
        return data
    except Exception as e:
        print("Error:", e)


def get_cointable_symbol_by_id(id):
    for row in cointable:
        if row["id"] == id:
            return row["symbol"]


def get_cointable_value_by_id(id):
    for row in cointable:
        if row["id"] == id:
            return row["usd"], row["jpy"]


def set_cointable_value_by_id(id, usd, jpy):
    for row in cointable:
        if row["id"] == id:
            row["usd"] = usd
            row["jpy"] = jpy


print("start\n")

# read files
files = os.listdir(dir)
file_count = len(files)
# print(files)
# print("file_count:", file_count)

connect_to_wifi()


ids = ""
for row in cointable:
    # print(f"id: {row['id']}", f"name: {row['name']}", f"symbol: {row['symbol']}")
    ids = ids + row["id"] + ","


while True:  # for i in range(10):
    json_data = get_price(ids, "usd,jpy")

    for key, value in json_data.items():
        date = time.localtime(value["last_updated_at"])
        formatted_date = "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(
            date[0], date[1], date[2], date[3], date[4], date[5]
        )
        # print(formatted_date)
        text((2, 2), formatted_date, TFT.BLACK)
        break

    y = 12
    x = 40
    for key, value in json_data.items():
        print(f"{key}: {value['usd']}  {value['jpy']}")

        last_usd, last_jpy = get_cointable_value_by_id(key)
        print(last_usd, last_jpy)

        symbol = get_cointable_symbol_by_id(key)
        file_path = dir + "/" + symbol + ".bmp"
        draw_bmp_position(file_path, 8, y)

        if last_usd < value["usd"]:
            text(
                (x, y + 2),
                "$" + str(value["usd"]) + " >   ",
                TFTColor(0x00, 0x80, 0x00),
            )
            text(
                (x, y + 14),
                "\\" + str(value["jpy"]) + " >   ",
                TFTColor(0x00, 0x80, 0x00),
            )
        elif last_usd > value["usd"]:
            text((x, y + 2), "$" + str(value["usd"]) + " <   ", TFT.RED)
            text((x, y + 14), "\\" + str(value["jpy"]) + " <   ", TFT.RED)
        else:
            text((x, y + 2), "$" + str(value["usd"]) + "     ", TFT.BLACK)
            text((x, y + 14), "\\" + str(value["jpy"]) + "     ", TFT.BLACK)

        y = y + 30

        set_cointable_value_by_id(key, value["usd"], value["jpy"])
    time.sleep(WAIT_TIME_FOR_API)


# spi0.deinit()
