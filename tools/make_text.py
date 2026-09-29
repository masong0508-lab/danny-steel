"""Placeholder grunge title + PRESS START -> 4bpp tiles + maps (src/text_data.c).
Run: python3 tools/make_text.py   (needs Pillow, numpy, opencv-python)
Colour indices (palette bank 14): 1 = dark outline, 2 = cream, 3 = tan grunge.
"""
import numpy as np, cv2, random
from PIL import Image, ImageDraw, ImageFont
random.seed(7); rng = np.random.default_rng(7)
FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf'
SS = 4                                 # supersampling
TW, TH = 30, 10                        # title area in tiles (240x80 px)
PW, PH = 30, 2                         # press-start area in tiles (240x16 px)

def fit(text, width, maxh):
    lo, hi = 6, 200
    while lo < hi:
        mid = (lo + hi + 1) // 2
        f = ImageFont.truetype(FONT, mid)
        w = f.getlength(text)
        if w <= width and mid <= maxh: lo = mid
        else: hi = mid - 1
    return ImageFont.truetype(FONT, lo)

def line(canvas, text, width, size_cap, cy, jitter):
    """draw text letter by letter (random tilt / baseline) on the SS-scaled canvas"""
    f = fit(text, width * SS, size_cap * SS)
    total = sum(f.getlength(c) for c in text)
    x = (canvas.width - total) / 2
    for c in text:
        w = f.getlength(c)
        if c != ' ':
            g = Image.new('L', (int(w * 2 + 20), int(f.size * 2)), 0)
            ImageDraw.Draw(g).text((g.width // 2 - w / 2, f.size * 0.4), c, font=f, fill=255)
            g = g.rotate(random.uniform(-jitter, jitter), resample=Image.BICUBIC)
            dy = random.uniform(-0.05, 0.05) * f.size
            canvas.paste(255, (int(x - g.width / 2 + w / 2), int(cy * SS - f.size * 0.9 + dy)), g)
        x += w

def grunge(mask_ss, strength):
    """roughen edges, bite holes, then reduce to 1x"""
    h, w = mask_ss.shape
    n1 = cv2.GaussianBlur(rng.random((h, w)).astype(np.float32), (0, 0), 3)
    n2 = cv2.GaussianBlur(rng.random((h, w)).astype(np.float32), (0, 0), 3)
    n1 = (n1 - n1.mean()) / n1.std(); n2 = (n2 - n2.mean()) / n2.std()
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    m = cv2.remap(mask_ss, xx + n1 * 2.2 * strength, yy + n2 * 2.2 * strength, cv2.INTER_LINEAR)
    bite = cv2.GaussianBlur(rng.random((h, w)).astype(np.float32), (0, 0), 1.6)
    bite = (bite - bite.mean()) / bite.std()
    m = np.where(bite > 1.9 - 0.3 * strength, 0, m)
    return cv2.resize(m, (w // SS, h // SS), interpolation=cv2.INTER_AREA)

def colourise(cov, tx, ty):
    """coverage -> palette indices (0 clear, 1 outline, 2 cream, 3 tan)"""
    fill = cov > 120
    k = np.array([[0, 1, 0], [1, 1, 1], [0, 1, 0]], np.uint8)
    edge = cv2.dilate(fill.astype(np.uint8), k) > 0
    sh = np.zeros_like(edge); sh[1:, 1:] = edge[:-1, :-1]; shadow = sh & ~edge
    out = np.zeros(cov.shape, np.uint8)
    out[shadow] = 1; out[edge & ~fill] = 1
    out[fill] = 2
    h, w = cov.shape
    t = cv2.GaussianBlur(rng.random((h, w)).astype(np.float32), (0, 0), 1.2)
    t = (t - t.mean()) / t.std()
    out[fill & (t > 1.25)] = 3
    yy = np.arange(h)[:, None]
    out[fill & (yy % 8 >= 6) & (t > 0.6)] = 3                 # a little dirt
    for _ in range(int(w * h / 90)):                          # scratches
        x, y = random.randrange(w), random.randrange(h); L = random.randrange(4, 14)
        for i in range(L):
            if 0 <= x + i < w and 0 <= y + i // 5 < h and out[y + i // 5, x + i] == 2:
                out[y + i // 5, x + i] = 3 if random.random() < 0.6 else 1
    return out

def build(lines, tw, th, jitter, strength):
    canvas = Image.new('L', (tw * 8 * SS, th * 8 * SS), 0)
    for text, width, cap, cy in lines: line(canvas, text, width, cap, cy, jitter)
    cov = grunge(np.array(canvas, np.float32), strength)
    return colourise(cov, tw, th)

title = build([('The', 62, 15, 13), ('LAST DAYS', 226, 32, 40), ('of danny steel', 196, 20, 64)], TW, TH, 3.0, 1.0)
press = build([('PRESS START', 150, 11, 6)], PW, PH, 1.2, 0.5)

tiles = [bytes(32)]; lookup = {bytes(32): 0}
def tile_index(px):
    # 4bpp: low nibble = left pixel
    b = bytearray(32)
    for y in range(8):
        for x in range(0, 8, 2):
            b[y * 4 + x // 2] = int(px[y, x]) | (int(px[y, x + 1]) << 4)
    b = bytes(b)
    if b not in lookup: lookup[b] = len(tiles); tiles.append(b)
    return lookup[b]
def make_map(img, tw, th):
    m = []
    for ty in range(th):
        for tx in range(tw):
            t = tile_index(img[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8])
            m.append(t | (14 << 12) if t else 0)
    return m
tmap = make_map(title, TW, TH); pmap = make_map(press, PW, PH)
assert len(tiles) <= 380, 'too many text tiles: %d' % len(tiles)
words = [int.from_bytes(t[i:i + 4], 'little') for t in tiles for i in range(0, 32, 4)]
pal = [0x0000, 0x0842, 0x3BDE, 0x2AF9] + [0] * 12         # outline, cream, tan
def rgb15(r, g, b): return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
pal = [0, rgb15(0x14, 0x08, 0x0a), rgb15(0xf4, 0xe8, 0xc6), rgb15(0xc4, 0x9a, 0x5c)] + [0] * 12
with open('src/text_data.c', 'w') as o:
    o.write('// GENERATED by tools/make_text.py - do not edit\n#include "data.h"\n')
    o.write('const u16 text_pal[16] = {%s};\n' % ', '.join('0x%04x' % v for v in pal))
    o.write('const int text_tile_count = %d;\n' % len(tiles))
    o.write('const u32 text_tiles[%d] __attribute__((aligned(4))) = {\n' % len(words))
    for i in range(0, len(words), 8): o.write(', '.join('0x%08x' % v for v in words[i:i + 8]) + ',\n')
    o.write('};\n')
    o.write('const u16 title_map[%d] = {\n' % len(tmap))
    for i in range(0, len(tmap), 15): o.write(', '.join('0x%04x' % v for v in tmap[i:i + 15]) + ',\n')
    o.write('};\nconst u16 press_map[%d] = {\n' % len(pmap))
    for i in range(0, len(pmap), 15): o.write(', '.join('0x%04x' % v for v in pmap[i:i + 15]) + ',\n')
    o.write('};\n')
# previews (RGB) for checking
P = np.array([[0, 0, 0], [0x14, 0x08, 0x0a], [0xf4, 0xe8, 0xc6], [0xc4, 0x9a, 0x5c]], np.uint8)
np.save('/tmp/title_idx.npy', title); np.save('/tmp/press_idx.npy', press)
print('text tiles used:', len(tiles), '(max 383)')
