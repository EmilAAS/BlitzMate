import requests
import json

#this code enables me to stream moves made on lichess to python
TOKEN = "insert_token_here" 
HEADERS = {"Authorization": f"Bearer {TOKEN}"}

def get_current_game_id():
    """Return the gameId of your currently active Lichess game."""
    url = "https://lichess.org/api/account/playing"
    response = requests.get(url, headers=HEADERS)
    data = response.json()

    for game in data.get("nowPlaying", []):
        print(f"Found active game vs {game['opponent']['username']}")
        return game["gameId"]
    return None

def stream_lichess_moves(game_id):
    """Continuously stream and print new moves."""
    url = f"https://lichess.org/api/board/game/stream/{game_id}"
    known_moves = []

    with requests.get(url, headers=HEADERS, stream=True) as response:
        print("Streaming moves...")
        for line in response.iter_lines():
            if line:
                data = json.loads(line.decode("utf-8"))
                if data.get("type") == "gameState":
                    moves = data.get("moves", "").split()
                    if len(moves) > len(known_moves):
                        new_move = moves[-1]
                        print(f"New move: {new_move}")
                        known_moves = moves

# === MAIN ===
if __name__ == "__main__":
    game_id = get_current_game_id()
    if game_id:
        try:
            stream_lichess_moves(game_id)
        except KeyboardInterrupt:
            print("Stream stopped.")
    else:
        print("No active games found.")
