import cv2
import numpy as np
import pyautogui
import time
import config
import subprocess
from detect_colors import get_autoplayer_input

def play_automatically():
    screenshot = np.array(pyautogui.screenshot())
    img = cv2.cvtColor(screenshot, cv2.COLOR_RGB2BGR)
    
    tube_locations, stdin = get_autoplayer_input(img)
    
    print(stdin)
    
    try:
        process = subprocess.run(
            [config.EXE_PATH],
            input=stdin,
            text=True,
            capture_output=True,
            check=True
        )
        
        print(process.stdout)
        
        moves = [str.split(" ") for str in process.stdout.splitlines()]
        moves = [(int(move[0]), int(move[1])) for move in moves]
        
        print(moves)
        
        input("Click any button to begin playback.")
        play_game(tube_locations, moves)
    except subprocess.CalledProcessError as e:
        print("Solver crashed!")
        print(e.stderr)
    
def play_game(tube_locations, move_list):
    for (move_from, move_to) in move_list:
        tube_a_x, tube_a_y = tube_locations[move_from]
        tube_b_x, tube_b_y = tube_locations[move_to]
        
        print(f"{move_from} -> {move_to}")
        
        pyautogui.moveTo(tube_a_x, tube_a_y)
        time.sleep(0.1)
        pyautogui.click()
        time.sleep(0.1)
        pyautogui.moveTo(tube_b_x, tube_b_y)
        time.sleep(0.1)
        pyautogui.click()
        time.sleep(1.5)
        
        
        

if __name__ == "__main__":
    play_automatically()
