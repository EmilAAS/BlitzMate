import serial
import time

# === CONFIGURATION ===
SERIAL_PORT = "COM4"       # <-- replace with your Arduino's port
BAUD_RATE = 115200           #match baud rate of arduino code

def read_move_from_arduino(ser):
    if ser.in_waiting:
        line = ser.readline().decode().strip()
        if len(line) == 4 or len(line) == 5:
            print(f"[Arduino] Sent move: {line}")
            return line
    return None

# === MAIN PROGRAM ===
if __name__ == "__main__":
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Allow Arduino to reset
        print("Serial connection established. Reading moves...")

        while True:
            move = read_move_from_arduino(ser)
            if move:
                print(move)
                pass
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nStopped by user.")
    except Exception as e:
        print(f"Serial error: {e}")
