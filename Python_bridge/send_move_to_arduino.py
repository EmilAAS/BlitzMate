import serial
import time

# === CONFIGURATION ===
SERIAL_PORT = "COM4"       # <-- Replace with your Arduino's port
BAUD_RATE = 115200         # match baud rate of Arduino code

def send_move_to_arduino(ser, move):
    try:
        ser.write((move + "\n").encode())
        print(f"[Board] Sent move to board: {move}")
    except Exception as e:
        print(f"[ERROR] Sending to board: {e}")

# === MAIN PROGRAM ===
if __name__ == "__main__":
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Give Arduino time to reset
        print("Connected to Arduino.")

        while True:
            move = input("Enter move: ").strip().lower()
            send_move_to_arduino(ser, move)

    except KeyboardInterrupt:
        print("\nExiting.")
    except Exception as e:
        print(f"Error: {e}")
