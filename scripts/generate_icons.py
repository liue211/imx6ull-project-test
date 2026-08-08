#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成浅色卡片风界面图标(Pillow 10.x)。

用法: python scripts/generate_icons.py
输出: project/images/icons/*.png(48x48,透明背景,扁平线性风格)
"""
import os
from PIL import Image, ImageDraw

SIZE = 48
INK = (55, 65, 81, 255)        # #374151
BLUE = (47, 124, 246, 255)     # #2F7CF6
GREEN = (34, 197, 94, 255)     # #22C55E
GRAY = (209, 213, 219, 255)    # #D1D5DB
OUT_DIR = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "project", "images", "icons"))


def new_img():
    return Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))


def save(img, name):
    path = os.path.join(OUT_DIR, name)
    img.save(path)
    print("生成:", os.path.abspath(path))


def polygon(d, pts, color=INK, width=4):
    d.polygon(pts, fill=color)


def line(d, pts, color=INK, width=4):
    d.line(pts, fill=color, width=width, joint="curve")


def rect(d, box, color=INK, width=4):
    d.rectangle(box, outline=color, width=width)


def fill_rect(d, box, color=INK, width=4):
    d.rectangle(box, fill=color)


def dot(d, center, r, color=INK, width=4):
    x, y = center
    d.ellipse([x - r, y - r, x + r, y + r], fill=color)


def arc(d, box, start, end, color=INK, width=4):
    d.arc(box, start, end, fill=color, width=width)


def speaker(d, x0, y0, x1, y1, color=INK, width=4):
    polygon(d, [(x0, y0 + 6), (x0 + 7, y0 + 6), (x1, y0), (x1, y1),
                (x0 + 7, y1 - 6), (x0, y1 - 6)], color)


def play():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(16, 12), (16, 36), (37, 24)])
    return img


def pause():
    img = new_img(); d = ImageDraw.Draw(img)
    fill_rect(d, [13, 12, 21, 36]); fill_rect(d, [27, 12, 35, 36])
    return img


def prev():
    img = new_img(); d = ImageDraw.Draw(img)
    fill_rect(d, [11, 13, 16, 35]); polygon(d, [(22, 12), (22, 36), (38, 24)])
    return img


def next_():
    img = new_img(); d = ImageDraw.Draw(img)
    fill_rect(d, [32, 13, 37, 35]); polygon(d, [(26, 12), (26, 36), (10, 24)])
    return img


def volume():
    img = new_img(); d = ImageDraw.Draw(img)
    speaker(d, 12, 14, 28, 34)
    arc(d, [22, 10, 38, 26], -60, 60)
    arc(d, [26, 4, 44, 32], -60, 60)
    return img


def volume_up():
    img = volume(); d = ImageDraw.Draw(img)
    line(d, [(40, 20), (40, 30)]); line(d, [(35, 25), (45, 25)])
    return img


def volume_down():
    img = volume(); d = ImageDraw.Draw(img)
    line(d, [(35, 25), (45, 25)])
    return img


def fullscreen():
    img = new_img(); d = ImageDraw.Draw(img)
    for x0, y0 in [(8, 8), (32, 8), (8, 32), (32, 32)]:
        line(d, [(x0, y0), (x0 + 7, y0)])
        line(d, [(x0, y0), (x0, y0 + 7)])
    return img


def screen():
    img = new_img(); d = ImageDraw.Draw(img)
    rect(d, [6, 8, 42, 34])
    line(d, [(20, 36), (28, 36)])
    line(d, [(24, 36), (24, 42)])
    line(d, [(16, 42), (32, 42)])
    return img


def favorite(color=INK):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (17, 19), 7, color)
    dot(d, (31, 19), 7, color)
    polygon(d, [(9, 23), (39, 23), (24, 41)], color)
    return img


def favorite_on():
    return favorite(BLUE)


def list_icon():
    img = new_img(); d = ImageDraw.Draw(img)
    line(d, [(10, 14), (38, 14)]); line(d, [(10, 24), (38, 24)])
    line(d, [(10, 34), (38, 34)])
    return img


def menu():
    img = new_img(); d = ImageDraw.Draw(img)
    for y in (12, 24, 36):
        dot(d, (24, y), 3)
    return img


def photo():
    img = new_img(); d = ImageDraw.Draw(img)
    rect(d, [6, 14, 42, 36])
    polygon(d, [(18, 14), (21, 8), (27, 8), (30, 14)])
    dot(d, (24, 26), 7)
    return img


def arrow_left():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(34, 10), (16, 24), (34, 38)])
    return img


def arrow_right():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(14, 10), (32, 24), (14, 38)])
    return img


def dot_icon(color=GRAY):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 6, color)
    return img


def led(color=GRAY):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 9, color)
    return img


def beep(color=INK):
    img = new_img(); d = ImageDraw.Draw(img)
    speaker(d, 10, 14, 24, 34)
    arc(d, [20, 10, 36, 26], -60, 60, color)
    arc(d, [24, 4, 42, 32], -60, 60, color)
    return img


def check_icon(color=GREEN):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 11, color)
    line(d, [(17, 25), (22, 30), (32, 18)], color=(255, 255, 255, 255))
    return img


def cross_icon(color=GRAY):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 11, color)
    line(d, [(18, 18), (30, 30)], color=(255, 255, 255, 255))
    line(d, [(30, 18), (18, 30)], color=(255, 255, 255, 255))
    return img


def send():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(8, 12), (40, 24), (8, 36), (16, 24)])
    return img


def clear():
    img = new_img(); d = ImageDraw.Draw(img)
    line(d, [(14, 14), (34, 34)]); line(d, [(34, 14), (14, 34)])
    return img


def refresh():
    img = new_img(); d = ImageDraw.Draw(img)
    arc(d, [10, 10, 38, 38], 20, 300)
    polygon(d, [(38, 16), (30, 12), (33, 22)])
    return img


ICONS = {
    "btn_play.png": play,
    "btn_pause.png": pause,
    "btn_prev.png": prev,
    "btn_next.png": next_,
    "btn_volume.png": volume,
    "btn_volume_up.png": volume_up,
    "btn_volume_down.png": volume_down,
    "btn_fullscreen.png": fullscreen,
    "btn_screen.png": screen,
    "btn_favorite.png": favorite,
    "btn_favorite_on.png": favorite_on,
    "btn_list.png": list_icon,
    "btn_menu.png": menu,
    "btn_photo.png": photo,
    "arrow_left.png": arrow_left,
    "arrow_right.png": arrow_right,
    "dot_normal.png": lambda: dot_icon(GRAY),
    "dot_active.png": lambda: dot_icon(BLUE),
    "led_on.png": lambda: led(GREEN),
    "led_off.png": lambda: led(GRAY),
    "beep_on.png": lambda: beep(GREEN),
    "beep_off.png": lambda: beep(GRAY),
    "btn_connect.png": lambda: check_icon(GREEN),
    "btn_disconnect.png": lambda: cross_icon(GRAY),
    "btn_send.png": send,
    "btn_clear.png": clear,
    "btn_refresh.png": refresh,
}


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, fn in ICONS.items():
        save(fn(), name)
    print("共生成 %d 个图标 -> %s" % (len(ICONS), OUT_DIR))


if __name__ == "__main__":
    main()
