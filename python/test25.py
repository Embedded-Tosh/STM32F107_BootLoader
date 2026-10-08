import struct
import sys
import time

import serial

# ---------------------------------------------------------------------------
# USER SETTINGS - edit these
# ---------------------------------------------------------------------------
COM_PORT = "COM3"                                   # e.g. "COM5" (Windows) or "/dev/ttyUSB0" (Linux)
BAUD_RATE = 115200                                  # bootloader baud rate
APP_BAUD_RATE = 57600                               # running application's baud rate
APP_UPDATE_COMMAND = b"CM42*"                       # makes the application reboot into the bootloader
APP_BANNER_WAIT_S = 2.0                             # how long to listen for the app's startup message
BIN_FILE_PATH = r"D:\Aashutosh\Naimex\Load_Control_100kN\STM32_ELE_100kn_LOAD_FRAME_28_09_2026\Project\proj.bin"   # full path to your application .bin
# ---------------------------------------------------------------------------

FRAME_START = 0xAA
ACK = 0x06
NACK = 0x15

CMD_SYNC = 0x01
CMD_ERASE = 0x02
CMD_WRITE = 0x03
CMD_GO = 0x04

CHUNK_SIZE = 1024
BYTE_TIMEOUT_S = 2.0
ERASE_TIMEOUT_S = 5.0   # erasing 240 KB in 2KB pages can take a moment
MAX_RETRIES = 5


def crc16_modbus(data: bytes) -> int:
    """CRC16, poly 0xA001 (reflected 0x8005), matches the bootloader's C implementation."""
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def build_frame(cmd: int, payload: bytes = b"") -> bytes:
    header = bytes([cmd, len(payload) & 0xFF, (len(payload) >> 8) & 0xFF])
    body = header + payload
    crc = crc16_modbus(body)
    return bytes([FRAME_START]) + body + struct.pack("<H", crc)


def send_command(ser: serial.Serial, cmd: int, payload: bytes = b"", timeout: float = BYTE_TIMEOUT_S) -> bool:
    frame = build_frame(cmd, payload)
    ser.timeout = timeout
    ser.write(frame)
    resp = ser.read(1)
    if len(resp) != 1:
        return False
    return resp[0] == ACK


def send_command_with_retry(ser: serial.Serial, cmd: int, payload: bytes = b"",
                             timeout: float = BYTE_TIMEOUT_S, retries: int = MAX_RETRIES) -> bool:
    for attempt in range(1, retries + 1):
        if send_command(ser, cmd, payload, timeout):
            return True
        print(f"  retry {attempt}/{retries} (cmd 0x{cmd:02X})...")
    return False


def flash_firmware(port: str, baud: int, path: str) -> int:
    with open(path, "rb") as f:
        fw = f.read()

    print(f"Firmware: {path} ({len(fw)} bytes)")

    # Step 1: tell the running application to reboot into the bootloader
    ser = serial.Serial(port, APP_BAUD_RATE, timeout=BYTE_TIMEOUT_S)
    time.sleep(0.7)  # let the port settle
    ser.write(APP_UPDATE_COMMAND)
    ser.flush()
    time.sleep(0.3)  # give the MCU time to reset
    ser.close()      # must close before reopening at a new baud rate

    # Step 2: talk to the bootloader at its own baud rate
    ser = serial.Serial(port, baud, timeout=BYTE_TIMEOUT_S)
    ser.reset_input_buffer()  # drop any stale bytes from the app

    print("Syncing with bootloader...")
    if not send_command_with_retry(ser, CMD_SYNC, timeout=BYTE_TIMEOUT_S):
        print("ERROR: no response from bootloader. Reset the board and try again"
              " within the entry window.")
        return 1

    print("Erasing application region (this can take a few seconds)...")
    if not send_command_with_retry(ser, CMD_ERASE, timeout=ERASE_TIMEOUT_S, retries=2):
        print("ERROR: erase failed.")
        return 1

    total = len(fw)
    sent = 0
    for offset in range(0, total, CHUNK_SIZE):
        chunk = fw[offset:offset + CHUNK_SIZE]
        # Bootloader rounds up to a multiple of 4 and pads with 0xFF on its side,
        # but we also pad here so the CRC covers exactly what's transmitted.
        if not send_command_with_retry(ser, CMD_WRITE, chunk):
            print(f"\nERROR: write failed at offset {offset}.")
            return 1
        sent += len(chunk)
        pct = sent * 100 // total
        print(f"\r  {sent}/{total} bytes ({pct}%)", end="", flush=True)

    print("\nWrite complete. Launching application...")
    if not send_command_with_retry(ser, CMD_GO, retries=1):
        print("WARNING: no ACK for GO command (board may have already jumped).")


    # The application prints its startup banner at ITS baud rate (not the
    # bootloader's). Switch the baud rate on the already-open port (no close/
    # reopen, so no bytes are missed in between) and show what it sends.
    ser.baudrate = APP_BAUD_RATE
    ser.timeout = 0.2
    print("\nApplication output:")
    print("-" * 40)
    end_time = time.time() + APP_BANNER_WAIT_S
    banner = b""
    while time.time() < end_time:
        banner += ser.read(256)
    ser.close()

    if banner:
        print(banner.decode("ascii", errors="replace").replace("\r\n", "\n").rstrip())
    else:
        print("(no output received from application)")
    print("-" * 40)

    print("Done.")
    return 0


def main():
    sys.exit(flash_firmware(COM_PORT, BAUD_RATE, BIN_FILE_PATH))


if __name__ == "__main__":
    main()