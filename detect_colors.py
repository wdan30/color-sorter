import cv2
import numpy as np
import sys


def find_tubes(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (3, 3), 0)

    edges = cv2.Canny(blurred, 30, 100)

    kernel = np.ones((3, 3), np.uint8)
    dilated_edges = cv2.dilate(edges, kernel, iterations=1)

    contours, _ = cv2.findContours(
        dilated_edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    tubes_boxes = []

    for cnt in contours:
        x, y, width, height = cv2.boundingRect(cnt)

        aspect_ratio = height / float(width)

        if 2.0 < aspect_ratio < 5.0 and height > 80:
            tubes_boxes.append((x, y, width, height))

    tubes_boxes.sort(key=lambda tube: (tube[1] // 50, tube[0]))
    
    print("tube_boxes:", tubes_boxes)
    
    return tubes_boxes


def draw_boxes(img, boxes):
    image_with_boxes = img.copy()

    for box in boxes:
        start_point = (box[0], box[1])
        end_point = (box[0] + box[2], box[1] + box[3])
        color = (0, 0, 255)

        image_with_boxes = cv2.rectangle(
            image_with_boxes, start_point, end_point, color, thickness=2
        )

    return image_with_boxes


def show_image(name, image):
    cv2.namedWindow(name, cv2.WINDOW_NORMAL)
    cv2.imshow(name, image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


def get_background_color(img):
    img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    pixels = img.reshape(-1, 3)

    unique, counts = np.unique(pixels, axis=0, return_counts=True)
    background_color = unique[np.argmax(counts)]

    return background_color


def parse_tubes(img, tubes):
    parsed_tubes = []
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    seen_colors = []

    h_img, w_img = hsv.shape[:2]
    bg_color = get_background_color(img)

    for x, y, width, height in tubes:
        middle = int(x + (width / 2))

        layer_heights = [
            int(y + height * 0.87),
            int(y + height * 0.68),
            int(y + height * 0.49),
            int(y + height * 0.30),
        ]

        tube_layers = []

        for height in layer_heights:
            y0 = max(0, height - 1)
            y1 = min(h_img, height + 2)
            x0 = max(0, middle - 1)
            x1 = min(w_img, middle + 2)

            if y0 >= y1 or x0 >= x1:
                roi = None
            else:
                roi = hsv[y0:y1, x0:x1]

            if roi is None or roi.size == 0:
                avg = (0, 0, 0)
            else:
                avg = cv2.mean(roi)[:3]

            if cv2.norm(bg_color - avg) > 80:
                color_char, seen_colors = color_to_char_dynamic(avg, seen_colors)

                # print(f"BG COLOR: {bg_color}")
                # print(f"AVG: {avg}")

                tube_layers.append(color_char)

        tube_str = "".join(tube_layers).rstrip("0")

        parsed_tubes.append(tube_str)

    return parsed_tubes


def color_to_char_dynamic(
    pixel_color,
    seen_colors,
    alphabet="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
    hue_tolerance=10,
    sat_tolerance=20,
):
    h, s, v = pixel_color

    # check all previously seen colors to see if it matches our pixel
    for i, known_color in enumerate(seen_colors):
        hue_diff = min(abs(h - known_color[0]), 180 - abs(h - known_color[0]))
        sat_diff = abs(s - known_color[1])

        if hue_diff < hue_tolerance and sat_diff < sat_tolerance:
            return alphabet[i], seen_colors

    if len(seen_colors) < len(alphabet):
        seen_colors.append((int(h), int(s), int(v)))

        return alphabet[len(seen_colors) - 1], seen_colors
    else:
        raise RuntimeError("Not enough characters left")


def detect_colors(path_to_image):
    image = cv2.imread(path_to_image)

    tube_boxes = find_tubes(image)

    return parse_tubes(image, tube_boxes)

def get_tube_center(box):
    return (box[0] + box[2] // 2, box[1] + box[3] // 2)

def get_autoplayer_input(img):
    tube_boxes = find_tubes(img)
    
    tube_centers = [get_tube_center(box) for box in tube_boxes]
    
    img_with_boxes = draw_boxes(img, tube_boxes)
    show_image("With boxes", img_with_boxes)
    
    tube_colors = parse_tubes(img, tube_boxes)
    
    print(tube_colors)
    
    for i, tube in enumerate(tube_colors):
        if tube == "":
            tube_colors[i] = ".EMPTY"

    tube_colors.append(".END")
    return tube_centers, "\n".join(tube_colors)
