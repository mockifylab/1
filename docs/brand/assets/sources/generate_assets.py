#!/usr/bin/env python3
"""CaeliGlow brand asset generator.

Hand-authored vector artwork (no stock, no AI imagery). SVGs are the editable
masters; PNGs are rendered from them with cairosvg + Inter.
"""
import math, os, random

OUT = os.path.expanduser("~/workspace/caeliglow/docs/brand/assets")
os.makedirs(OUT, exist_ok=True)

# ---- palette ----
MIDNIGHT = "#0A0F1E"
DUSK = "#151C34"
MOONLIGHT = "#E9EDF8"
STARDUST = "#9AA3C0"
GLOW = "#FFC766"
EMBER = "#A35500"
EMBER_BRIGHT = "#D97A06"
AURORA = "#6FD9C0"
PAPER = "#F7F8FD"
INK = "#131A30"
SLATE = "#5A6486"

FONT = "Inter"

def pt(angle_deg, r=56.0, cx=64.0, cy=64.0):
    a = math.radians(angle_deg)
    return (cx + r * math.cos(a), cy + r * math.sin(a))

def ring_path():
    # 320-degree arc, gap centered at -45 deg (upper right), round caps
    x1, y1 = pt(-25)
    x2, y2 = pt(-65)
    return (f"M {x1:.1f} {y1:.1f} A 56 56 0 1 1 {x2:.1f} {y2:.1f}")

def spark_path(cx, cy, r):
    k = r * 0.13
    return (
        f"M {cx} {cy - r} "
        f"C {cx + k} {cy - r * 0.32}, {cx + r * 0.32} {cy - k}, {cx + r} {cy} "
        f"C {cx + r * 0.32} {cy + k}, {cx + k} {cy + r * 0.32}, {cx} {cy + r} "
        f"C {cx - k} {cy + r * 0.32}, {cx - r * 0.32} {cy + k}, {cx - r} {cy} "
        f"C {cx - r * 0.32} {cy - k}, {cx - k} {cy - r * 0.32}, {cx} {cy - r} Z"
    )

def crescent_path():
    # Crescent = disk (64,64,r36) minus disk (80,52,r31), as a single path.
    # Intersection points computed analytically; outer arc takes the major
    # (left) way, inner arc the minor (upper-right) way back.
    ox, oy, r1 = 64.0, 64.0, 36.0
    ix, iy, r2 = 80.0, 52.0, 31.0
    dx, dy = ix - ox, iy - oy
    d = math.hypot(dx, dy)
    a = (r1 * r1 - r2 * r2 + d * d) / (2 * d)
    h = math.sqrt(max(r1 * r1 - a * a, 0))
    px, py = ox + a * dx / d, oy + a * dy / d
    qx, qy = -dy / d, dx / d
    i1 = (px - h * qx, py - h * qy)
    i2 = (px + h * qx, py + h * qy)
    # i1 should be the lower one (larger y)
    if i1[1] < i2[1]:
        i1, i2 = i2, i1
    return (f"M {i1[0]:.1f} {i1[1]:.1f} "
            f"A {r1:.0f} {r1:.0f} 0 1 1 {i2[0]:.1f} {i2[1]:.1f} "
            f"A {r2:.0f} {r2:.0f} 0 0 1 {i1[0]:.1f} {i1[1]:.1f} Z")

def mark_group(crescent, spark, ring):
    return f"""<path d="{crescent_path()}" fill="{crescent}"/>
  <path d="{spark_path(88, 40, 14)}" fill="{spark}"/>
  <path d="{ring_path()}" fill="none" stroke="{ring}" stroke-width="6" stroke-linecap="round"/>"""

def mark_svg(crescent, spark, ring, bg=None):
    bg_rect = f'<rect width="128" height="128" fill="{bg}"/>' if bg else ""
    return f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128" width="128" height="128" role="img" aria-label="CaeliGlow mark">
  <title>CaeliGlow mark</title>
  {bg_rect}
  {mark_group(crescent, spark, ring)}
</svg>
"""

def lockup_svg(wordmark_color, crescent, spark, ring, bg=None):
    bg_rect = f'<rect width="480" height="128" fill="{bg}"/>' if bg else ""
    return f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 480 128" width="480" height="128" role="img" aria-label="CaeliGlow logo">
  <title>CaeliGlow logo</title>
  {bg_rect}
  {mark_group(crescent, spark, ring)}
  <text x="148" y="86" font-family="{FONT}, 'Noto Sans', sans-serif" font-weight="700" font-size="62" letter-spacing="-1.5" fill="{wordmark_color}">CaeliGlow</text>
</svg>
"""

def mono_mark_svg():
    return f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128" width="128" height="128" role="img" aria-label="CaeliGlow mark (mono)">
  <title>CaeliGlow mark (mono)</title>
  <g fill="currentColor">
  <path d="{crescent_path()}" fill="currentColor"/>
  <path d="{spark_path(88, 40, 14)}"/>
  <path d="{ring_path()}" fill="none" stroke="currentColor" stroke-width="6" stroke-linecap="round"/>
  </g>
</svg>
"""

def write(name, content):
    p = os.path.join(OUT, name)
    with open(p, "w") as f:
        f.write(content)
    print("wrote", p)

# ---- logo SVGs (editable masters) ----
write("logo-mark-dark.svg", mark_svg(GLOW, MOONLIGHT, MOONLIGHT))
write("logo-mark-light.svg", mark_svg(INK, EMBER, INK))
write("logo-mark-mono.svg", mono_mark_svg())
write("logo-primary-dark.svg", lockup_svg(MOONLIGHT, GLOW, MOONLIGHT, MOONLIGHT))
write("logo-primary-light.svg", lockup_svg(INK, INK, EMBER, INK))

# ---- social preview 1280x640 (SVG master, rendered to PNG) ----
random.seed(42)
stars = []
for _ in range(70):
    x = random.uniform(20, 1260)
    y = random.uniform(20, 620)
    # keep the left text zone calmer
    if x < 560 and y > 180 and y < 520:
        continue
    r = random.uniform(0.8, 2.2)
    o = random.uniform(0.12, 0.55)
    stars.append(f'<circle cx="{x:.0f}" cy="{y:.0f}" r="{r:.1f}" fill="{MOONLIGHT}" opacity="{o:.2f}"/>')
stars_svg = "\n    ".join(stars)

social_svg = f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1280 640" width="1280" height="640" role="img" aria-label="CaeliGlow social preview">
  <title>CaeliGlow — a calm, glowing desktop shell for CachyOS and Hyprland</title>
  <rect width="1280" height="640" fill="{MIDNIGHT}"/>
  <g>
    {stars_svg}
  </g>
  <g transform="translate(880,40) scale(4)" opacity="0.16">
    {mark_group(GLOW, MOONLIGHT, MOONLIGHT)}
  </g>
  <g transform="translate(120,196)">
    {mark_group(GLOW, MOONLIGHT, MOONLIGHT)}
    <text x="160" y="88" font-family="{FONT}, 'Noto Sans', sans-serif" font-weight="700" font-size="76" letter-spacing="-2" fill="{MOONLIGHT}">CaeliGlow</text>
    <text x="162" y="140" font-family="{FONT}, 'Noto Sans', sans-serif" font-weight="400" font-size="30" fill="{STARDUST}">A calm, glowing desktop shell for CachyOS + Hyprland.</text>
    <g transform="translate(162,168)">
      <rect x="0" y="0" width="118" height="40" rx="20" fill="none" stroke="{GLOW}" stroke-width="2" opacity="0.9"/>
      <text x="59" y="27" text-anchor="middle" font-family="{FONT}, 'Noto Sans', sans-serif" font-weight="600" font-size="20" fill="{GLOW}">v1.0.0</text>
    </g>
  </g>
</svg>
"""
write("social-preview.svg", social_svg)

# ---- banner 1600x400 ----
banner_stars = []
random.seed(7)
for _ in range(40):
    x = random.uniform(700, 1580)
    y = random.uniform(20, 380)
    r = random.uniform(0.8, 2.0)
    o = random.uniform(0.10, 0.45)
    banner_stars.append(f'<circle cx="{x:.0f}" cy="{y:.0f}" r="{r:.1f}" fill="{MOONLIGHT}" opacity="{o:.2f}"/>')
banner_svg = f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1600 400" width="1600" height="400" role="img" aria-label="CaeliGlow banner">
  <title>CaeliGlow banner</title>
  <rect width="1600" height="400" fill="{MIDNIGHT}"/>
  {"".join(banner_stars)}
  <g transform="translate(1330,60) scale(2.2)" opacity="0.14">
    {mark_group(GLOW, MOONLIGHT, MOONLIGHT)}
  </g>
  <g transform="translate(96,88)">
    {mark_group(GLOW, MOONLIGHT, MOONLIGHT)}
    <text x="152" y="82" font-family="{FONT}, 'Noto Sans', sans-serif" font-weight="700" font-size="68" letter-spacing="-1.8" fill="{MOONLIGHT}">CaeliGlow</text>
    <text x="154" y="128" font-family="{FONT}, 'Noto Sans', sans-serif" font-weight="400" font-size="27" fill="{STARDUST}">A calm, glowing desktop shell for CachyOS + Hyprland.</text>
  </g>
</svg>
"""
write("banner.svg", banner_svg)

# ---- readme logo (transparent lockup, rendered 2x for retina) ----
readme_svg = lockup_svg(MOONLIGHT, GLOW, MOONLIGHT, MOONLIGHT)
write("readme-logo.svg", readme_svg)
readme_light_svg = lockup_svg(INK, INK, EMBER, INK)
write("readme-logo-light.svg", readme_light_svg)

print("done")
