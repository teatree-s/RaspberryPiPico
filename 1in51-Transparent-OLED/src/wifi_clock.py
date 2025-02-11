import network
import ntptime
import time


class WiFiClock:
    def __init__(self, ssid, password, ntp_server="pool.ntp.org", timezone_offset=9):
        """
        Wi-Fi に接続して NTP から時刻を取得するクラス。

        :param ssid: Wi-FiのSSID
        :param password: Wi-Fiのパスワード
        :param ntp_server: NTPサーバー（デフォルトは "pool.ntp.org"）
        :param timezone_offset: タイムゾーンのオフセット（日本標準時は UTC+9）
        """
        self.ssid = ssid
        self.password = password
        self.ntp_server = ntp_server
        self.timezone_offset = timezone_offset
        self.wlan = network.WLAN(network.STA_IF)
        ntptime.host = self.ntp_server  # NTPサーバーを設定

    def connect_wifi(self):
        self.wlan.active(True)
        self.wlan.connect(self.ssid, self.password)

        print("Wi-Fi接続中...")
        while not self.wlan.isconnected():
            time.sleep(1)

        print("Wi-Fi 接続成功!")
        print("IPアドレス:", self.wlan.ifconfig()[0])

    def sync_time(self):
        print("NTPサーバーから時刻を取得中...")
        try:
            ntptime.settime()
            print("時刻取得成功!")
        except Exception as e:
            print("時刻取得失敗:", e)

    def get_local_time(self):
        t = time.localtime(time.time() + self.timezone_offset * 3600)
        return t
