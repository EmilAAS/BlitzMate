import requests
import json
import time
import serial

# === CONFIG ===
TOKEN = "insert_token_here"
HEADERS = {"Authorization": f"Bearer {TOKEN}"}
SERIAL_PORT = "COM4"  # replace with correct usb port connected to Arduino
BAUD_RATE = 115200    # Match Arduino Code

# === UTILS ===
def get_current_game_id():
    url = "https://lichess.org/api/account/playing"
    try:
        r = requests.get(url, headers=HEADERS)
        data = r.json()
        for game in data["nowPlaying"]:
            return game["gameId"], game["color"]
    except Exception as e:
        print(f"[ERROR] Fetching current game: {e}")
    return None, None

def stream_lichess_moves(game_id, last_known):
    url = f"https://lichess.org/api/board/game/stream/{game_id}"
    with requests.get(url, headers=HEADERS, stream=True) as response:
        for line in response.iter_lines():
            if line:
                data = json.loads(line.decode("utf-8"))
                if data.get("type") == "gameState":
                    moves = data.get("moves", "").split()
                    w_time = data.get("wtime")  # White time in ms
                    b_time = data.get("btime")  # Black time in ms
                    new_move = moves[-1] if len(moves) > len(last_known) else None
                    return new_move, moves, w_time, b_time
    return None, last_known, None, None


def send_move_to_lichess(game_id, move):
    url = f"https://lichess.org/api/board/game/{game_id}/move/{move}"
    r = requests.post(url, headers=HEADERS)
    if r.status_code == 200:
        print(f"[Lichess] Move {move} sent successfully.")
        return True
    else:
        print(f"[ERROR] Move failed: {r.status_code} - {r.text}")
        return False

def read_move_from_arduino(ser):
    if ser.in_waiting:
        line = ser.readline().decode().strip()
        if len(line) == 4:
            print(f"[Arduino] Sent move: {line}")
            return line
    return None

def send_move_to_arduino(ser, move):
    try:
        ser.write((move + "\n").encode())
        print(f"[Board] Sent move to board: {move}")
    except Exception as e:
        print(f"[ERROR] Sending to board: {e}")

def is_my_turn(game_id):
    """Check if it's currently your turn in the game using game state API."""
    url = f"https://lichess.org/api/board/game/stream/{game_id}"
    try:
        response = requests.get(f"https://lichess.org/api/account/playing", headers=HEADERS)
        data = response.json()
        for game in data["nowPlaying"]:
            if game["gameId"] == game_id:
                return game.get("isMyTurn", False)
    except Exception as e:
        print(f"Turn check failed: {e}")
    return False

# === MAIN LOOP ===
def main():
    game_id, color = get_current_game_id()
    if not game_id:
        print("No active game found.")
        return

    print(f"[INFO] Game ID: {game_id} | You are playing as: {color.upper()}")

    last_moves = []
    last_sent_to_board = None
    last_sent_to_lichess = None

    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        time.sleep(2)
        print("[INFO] Serial connection ready. Syncing game...")
        # Handle White's First Move if You Are Playing Black ---
        if color == "white" and is_my_turn(game_id):
            print("Game just started. You're white — waiting for your first move from Arduino...")
            while True:
                my_move = read_move_from_arduino(ser)
                if my_move and my_move != last_sent_to_lichess:
                    success = send_move_to_lichess(game_id, my_move)
                    if success:
                        last_sent_to_lichess = my_move
                        last_moves.append(my_move)
                        print("First move sent to Lichess.")
                        break
                time.sleep(0.1)

        # Handle pre-existing moves if you are black
        if color == "black":
            print("You are BLACK. Checking if white has already made a move...")
            # Stream once to get current state
            new_move, full_moves, w_time, b_time = stream_lichess_moves(game_id, [])
            if full_moves:
                last_moves = full_moves
                if len(full_moves) % 2 == 1:  # If odd number of moves, it's white's move that just happened
                    white_first_move = full_moves[-1]
                    send_move_to_arduino(ser, white_first_move)
                    last_sent_to_board = white_first_move
                    print(f"White's first move ({white_first_move}) sent to board.")

        while True:
            try:
                # 1. Check for new Lichess moves
                new_move, full_moves, w_time, b_time = stream_lichess_moves(game_id, last_moves)

                # 2. Send updated clock and turn status
                if w_time and b_time:
                    clock_message = f"CLOCK:{w_time},{b_time}\n"
                    try:
                        ser.write(clock_message.encode())
                        print(f"[Clock] You: {w_time} | Opponent: {b_time}")
                    except Exception as e:
                        print(f"[ERROR] Failed to send clock or turn to Arduino: {e}")

                # 3. If opponent made a move, send it to the Arduino
                if new_move and new_move != last_sent_to_board:
                    move_index = len(full_moves) - 1
                    is_opponents_move = (
                        (color == "white" and move_index % 2 == 1) or
                        (color == "black" and move_index % 2 == 0)
                    )

                    if is_opponents_move:
                        send_move_to_arduino(ser, new_move)
                        last_sent_to_board = new_move
                        last_moves = full_moves
                
                # 4. Check for move from Arduino (our move)
                if is_my_turn(game_id):
                    print("It's your turn. Waiting for move from Arduino...")
                    while is_my_turn(game_id):  # Keep polling as long as it's our turn
                        my_move = read_move_from_arduino(ser)
                        if my_move and my_move != last_sent_to_lichess:
                            success = send_move_to_lichess(game_id, my_move)
                            if success:
                                last_sent_to_lichess = my_move
                                last_moves.append(my_move)
                                print("Move sent to Lichess.")
                                break  # Exit loop after successful move
                        time.sleep(0.1)

            except KeyboardInterrupt:
                print("\n[EXIT] Stopping...")
                break
            except Exception as e:
                print(f"[ERROR] Loop crash: {e}")
                time.sleep(1)


if __name__ == "__main__":
    main()
