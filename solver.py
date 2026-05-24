import subprocess
import sys
import config
from detect_colors import detect_colors

def main():
    if len(sys.argv) <= 1:
        image_file = "image.png"
    else:
        image_file = sys.argv[1]
    
    exe_path = "./a.exe"
    
    tubes = detect_colors(image_file)
    
    for i, tube in enumerate(tubes):
        if (tube == ''):
            tubes[i] = '.EMPTY'
    
    tubes.append('.END')
    stdin = '\n'.join(tubes)
    
    print(stdin)
    
    try:
        process = subprocess.run(
            [exe_path],
            input=stdin,
            text=True,
            capture_output=True,
            check=True
        )
        
        print(process.stdout)
    except subprocess.CalledProcessError as e:
        print("Solver crashed!")
        print(e.stderr)

if __name__ == "__main__":
    main()
