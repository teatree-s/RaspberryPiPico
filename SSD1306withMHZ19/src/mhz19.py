from machine import UART, Pin
import time


class MHZ19UART:
    RESPONSE_CNT = 9

    def __init__(self, tx_pin=0, rx_pin=1, uart_num=0):
        self.uart = UART(uart_num, baudrate=9600, tx=Pin(tx_pin), rx=Pin(rx_pin))
        self._co2 = 0
        self._temp = 0
        self._status = 0
        self._resp = 0

        self._get_ppm_cmd = bytearray([0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00])
        self._zero_calib_cmd = bytearray([0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00])
        self._span_calib_cmd = bytearray([0xFF, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00])
        self._auto_calib_on_cmd = bytearray([0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00])
        self._auto_calib_off_cmd = bytearray([0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00])

    def _calculate_checksum(self, cmd):
        total = sum(cmd[1:]) & 0xFF
        return (0xFF - total + 1) & 0xFF

    def _write_command(self, cmd, response_required=False):
        cmd = bytearray(cmd)
        checksum = self._calculate_checksum(cmd)
        self.uart.write(cmd)
        self.uart.write(bytearray([checksum]))
        time.sleep(0.1)

        if response_required:
            for _ in range(10):
                if self.uart.any() >= self.RESPONSE_CNT:
                    return self.uart.read(self.RESPONSE_CNT)
                time.sleep(0.05)
            return None

    def _read_serial_data(self):
        resp = self._write_command(self._get_ppm_cmd, response_required=True)
        if not resp or len(resp) != self.RESPONSE_CNT:
            self._co2 = self._temp = self._status = -1
            return

        self._resp = resp

        if (
            resp[0] == 0xFF
            and resp[1] == 0x86
            and self._calculate_checksum(resp[:-1]) == resp[-1]
        ):
            self._co2 = resp[2] * 256 + resp[3]
            self._temp = resp[4] - 40
        else:
            self._co2 = self._temp = self._status = -1

    def get_ppm(self):
        self._read_serial_data()
        return self._co2, self._temp

    def get_status(self):
        return self._status

    def get_response(self):
        return self._resp

    def calibrate_zero(self):
        self._write_command(self._zero_calib_cmd)

    def calibrate_span(self, ppm):
        if ppm < 1000:
            return
        cmd = bytearray(self._span_calib_cmd)
        cmd[3] = (ppm // 256) & 0xFF
        cmd[4] = (ppm % 256) & 0xFF
        self._write_command(cmd)

    def set_auto_calibration(self, enable=True):
        self._write_command(
            self._auto_calib_on_cmd if enable else self._auto_calib_off_cmd
        )
