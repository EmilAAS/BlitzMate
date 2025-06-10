##this code enables me to send moves to lichess through the api with inputs on UCI format e.g. "e2e4"
import requests
import json
import time

# === CONFIGURATION ===
TOKEN = "add_token_here"
HEADERS = {"Authorization": f"Bearer {TOKEN}"}


def get_current_game_id():
    """Return your active game ID (if any)."""
    url = "https://lichess.org/api/account/playing"
    try:
        response = requests.get(url, headers=HEADERS)
        data = response.json()

        for game in data["nowPlaying"]:
            if game.get("isMyTurn", False) or game.get("lastMove"):
                print(f"Found game vs {game['opponent']['username']}")
                return game["gameId"]
        return None
    except Exception as e:
        print(f"Error: {e}")
        return None


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


def send_move(game_id, move_uci):
    """Send a UCI move (e.g. e2e4) to Lichess."""
    url = f"https://lichess.org/api/board/game/{game_id}/move/{move_uci}"
    response = requests.post(url, headers=HEADERS)
    if response.status_code == 200:
        print(f"Move {move_uci} sent successfully.")
    else:
        print(f"Failed to send move: {response.status_code} - {response.text}")


# === MAIN LOOP ===
if __name__ == "__main__":
    game_id = get_current_game_id()

    if not game_id:
        print("No active game found. Start a game first.")
        exit(1)

    print(f"Game ID: {game_id}")
    print("You can now send moves in UCI format (e.g. e2e4, g1f3). Ctrl+C to quit.")

    while True:
        try:
            if is_my_turn(game_id):
                move = input(">>> Enter your move (UCI): ").strip().lower()
                if len(move) >= 4:
                    send_move(game_id, move)
                else:
                    print("Invalid input. Move must be at least 4 characters (like e2e4).")
            else:
                print("Waiting for your turn...")
                time.sleep(2)
        except KeyboardInterrupt:
            print("\nExiting.")
            break
