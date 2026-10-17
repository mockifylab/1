# CaeliGlow Brand Asset Inventory

All artwork is **original**, hand-authored vector geometry created for this
project (2026-10-17). No stock, no third-party logos, no AI-generated imagery,
nothing copied from Noctalia or Caelestia.

SVG files are the **editable masters**. PNGs are deterministic renders of the
SVGs, produced by `assets/sources/generate_assets.py` with `cairosvg` and the
Inter typeface (OFL, fetched from the google/fonts repository for rendering;
the brand font itself is freely obtainable — see brand guidelines §7).

## Logos (SVG masters)

| File | Intended use | Format | Canvas |
|---|---|---|---|
| `assets/logo-primary-dark.svg` | Default lockup (mark + "CaeliGlow" wordmark) on dark backgrounds | SVG, transparent | 480 × 128 |
| `assets/logo-primary-light.svg` | Lockup on light backgrounds | SVG, transparent | 480 × 128 |
| `assets/logo-mark-dark.svg` | Icon-only mark on dark backgrounds (avatars, favicons, About page) | SVG, transparent | 128 × 128 |
| `assets/logo-mark-light.svg` | Icon-only mark on light backgrounds | SVG, transparent | 128 × 128 |
| `assets/logo-mark-mono.svg` | One-color mark; uses `currentColor`, adapts to context | SVG, transparent | 128 × 128 |

Wordmark is live `<text>` in Inter Bold (tracking −2%) so it stays editable;
falls back to Noto Sans / system sans where Inter is absent.

## Raster artwork (PNG renders)

| File | Intended use | Format | Dimensions |
|---|---|---|---|
| `assets/social-preview.png` | GitHub repo social preview (exact required size) | PNG | 1280 × 640 |
| `assets/banner.png` | Release notes / announcements header | PNG | 1600 × 400 |
| `assets/readme-logo-dark.png` | README logo for dark GitHub theme; display at 480 px wide | PNG, transparent | 960 × 256 |
| `assets/readme-logo-light.png` | README logo for light GitHub theme; display at 480 px wide | PNG, transparent | 960 × 256 |

Also kept as SVG masters: `assets/social-preview.svg`, `assets/banner.svg`.

## Sources

| File | Notes |
|---|---|
| `assets/sources/generate_assets.py` | The exact script that authored every SVG and rendered every PNG. Re-run to regenerate after edits. |

## Verification performed

- Rendered every SVG at 1×–4× and inspected: no clipped text or geometry, wordmark
  fully inside viewBox, star fields kept out of the text zone.
- Social preview confirmed at exactly 1280 × 640; banner at 1600 × 400.
- Mark legibility checked at 64 px, 32 px, and 16 px on both dark and light
  backgrounds — crescent, ring, and spark remain distinguishable at 16 px.
- Text/background contrast ratios measured (sRGB): Moonlight/Midnight 16.3:1,
  Ink/Paper 16.3:1, Glow/Midnight 12.4:1, Aurora/Midnight 11.2:1,
  Stardust/Midnight 7.6:1, Slate/Paper 5.5:1, Ember/Paper 5.1:1.
  (Ember Bright `#D97A06` is 2.9:1 on Paper — graphics only, never body text.)
- No existing repository files were modified or overwritten; `docs/brand/` is new.
