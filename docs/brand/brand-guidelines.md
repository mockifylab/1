# CaeliGlow Brand Guidelines

Version 1.0 — 2026-10-17. Lives with the project at `docs/brand/`.

## 1. Brand at a glance

- **Name:** CaeliGlow — pronounced *KYE-lee-gloh*. From Latin *caeli* ("of the sky")
  + *glow*.
- **Tagline:** *A calm, glowing desktop shell for CachyOS + Hyprland.*
- **One-liner:** CaeliGlow is a standalone desktop shell for CachyOS and Hyprland,
  built as a customization of Noctalia. It keeps Noctalia's bar, widgets, launcher,
  settings, and appearance, and adds calm hover animations, edge-triggered launcher
  and power controls, a native About section, and an update checker.

**Relationship honesty (non-negotiable in all brand copy):** CaeliGlow is a
Noctalia-based customization; Caelestia is a behavioral reference only. There is
no affiliation with, or endorsement by, the Noctalia or Caelestia projects. Never
present CaeliGlow as either project, and never reuse their logos or artwork.

## 2. Brand purpose and positioning

Linux desktop enthusiasts already have powerful shells; what they lack is one that
feels *considered*. CaeliGlow's purpose is to make a Noctalia-based daily driver
feel intentional: every reveal is timed, every destructive action asks first, every
surface respects the user's motion preferences.

Positioning: **the refined, atmospheric shell for people who rice with restraint.**
Not the flashiest setup on the forum — the one that still looks right a year later.

## 3. Audience

- **Primary:** CachyOS / Arch + Hyprland users who customize their desktop and
  value polish over novelty. Comfortable with config files, `just`, and the AUR.
- **Secondary:** Noctalia users curious about a more guided, animated experience;
  theme and dotfile authors looking for a coherent visual system to build on.

They notice kerning, timing curves, and contrast ratios. The brand should reward
that attention, not shout over it.

## 4. Brand personality

| Trait | What it means in practice |
|---|---|
| **Calm** | Generous whitespace, muted surfaces, one accent at a time. |
| **Luminous** | Light is the motif — glow accents, never neon noise. |
| **Precise** | Aligned geometry, real contrast ratios, documented specs. |
| **Honest** | Plain-language status copy ("Update checking isn't configured yet."). No fake screenshots, no invented features. |
| **Considerate** | Reduced-motion respected, labels on controls, destructive actions confirmed. |

## 5. Logo

### Concept

The mark is a **crescent moon cradling a four-pointed spark**, wrapped in a thin
orbit ring broken at the spark. It reads three ideas at once:

- the *caeli* — night sky, the crescent;
- the *glow* — warm amber light against the dark;
- the *shell* — the orbit ring, a system in motion (a nod to the hover-reveal
  interactions CaeliGlow is built around).

The four-pointed spark echoes the "sparkles" motif already used in the project's
Settings UI. The mark is original artwork drawn for this project — it copies
nothing from Noctalia or Caelestia.

### Variants

| File | Use |
|---|---|
| `logo-primary-dark.svg` | Default horizontal lockup (mark + wordmark) on dark backgrounds |
| `logo-primary-light.svg` | Horizontal lockup on light backgrounds |
| `logo-mark-dark.svg` | Icon-only mark on dark backgrounds |
| `logo-mark-light.svg` | Icon-only mark on light backgrounds |
| `logo-mark-mono.svg` | Single-color mark; inherits `currentColor`. For favicons, embossing, one-color print |

SVG files are the editable masters. PNGs are renders, not masters (see
`asset-inventory.md`).

### Construction and clear space

- The mark lives on a 128-unit grid: orbit ring r=56, stroke 6, with a 40° gap at
  upper-right; crescent from two circles; spark a concave four-point star.
- **Clear space:** at least the height of the spark on all sides. Nothing enters it.
- **Minimum sizes:** mark ≥ 16 px, lockup ≥ 96 px wide (below that, use the mark alone).
- **Don't** stretch, rotate, recolor, add shadows/outlines, place on busy imagery,
  or set the dark lockup on light backgrounds (and vice versa).

### Correct / incorrect usage

✅ Dark lockup on Midnight; light lockup on Paper; mono mark in `currentColor`
for single-color contexts; mark alone as avatar/favicon.

❌ Recoloring the crescent teal "to match my theme"; adding a drop shadow;
animating the logo on the website hero (see §9 — the product animates, the brand
mark stays still); pairing the wordmark with a different font; using the mark as
a bullet or decorative texture.

## 6. Color

Core palette. All text pairings below meet WCAG AA (measured, sRGB):

| Name | HEX | Role |
|---|---|---|
| Midnight | `#0A0F1E` | Primary dark background |
| Dusk | `#151C34` | Dark surface / card |
| Moonlight | `#E9EDF8` | Primary text on dark — 16.3:1 on Midnight |
| Stardust | `#9AA3C0` | Secondary text on dark — 7.6:1 on Midnight |
| Glow | `#FFC766` | Primary accent on dark; large graphics — 12.4:1 on Midnight |
| Aurora | `#6FD9C0` | Secondary accent, sparing (success, highlights) — 11.2:1 on Midnight |
| Paper | `#F7F8FD` | Primary light background |
| Ink | `#131A30` | Primary text on light — 16.3:1 on Paper |
| Slate | `#5A6486` | Secondary text on light — 5.5:1 on Paper |
| Ember | `#A35500` | Accent **text** on light — 5.1:1 on Paper |
| Ember Bright | `#D97A06` | Large graphics on light only (not body text — 2.9:1) |

Rules:

- **One glow at a time.** Glow/Ember is the voice of the brand; Aurora appears at
  most once per composition, for status or delight.
- Accent text on dark: Glow or Moonlight. Accent text on light: Ember or Ink.
  Never set body text in pure accent colors outside these pairings.
- The palette is atmospheric, not neon: no gradients except a subtle radial glow
  behind the mark in hero art, and even that is optional.

## 7. Typography

- **Interface / display / wordmark: Inter** (OFL, Google Fonts). Wordmark is Inter
  Bold, tracking −2%. UI text Inter Regular/Medium; headings Inter SemiBold.
  Easy to obtain: `inter-font` on Arch/CachyOS, or Google Fonts.
- **Code / numbers: JetBrains Mono** (OFL). `ttf-jetbrains-mono` on Arch.
- Fallback stack: `Inter, "Noto Sans", system-ui, sans-serif`.
- Scale (marketing/docs): display 76, H1 40, H2 28, body 17–18, caption 13–14.
  In-product UI keeps Noctalia's own type settings — brand type governs project
  surfaces (README, site, release art), not the shell itself.

## 8. Iconography, illustration, layout, image style

- **Iconography:** geometric, 2 px optical stroke at 24 px grid, rounded caps —
  the orbit ring's DNA. Noctalia's existing widget glyphs stay as-is in product.
- **Illustration:** flat celestial geometry (arcs, rings, dots) on Midnight.
  No gradients-as-decoration, no 3D, no clip-art laptops.
- **Photography/imagery:** none required. If showing the desktop, use real,
  un-retouched screenshots labeled as such — never mockups that imply features
  that don't exist.
- **Layout:** left-aligned, generous margins (≥ 10% of canvas), one focal element.
  Social and banner art follow the compositions in `assets/` — don't rearrange them.

## 9. Accessibility

- **Contrast:** only the pairings in §6 for text. Check any new combination with a
  ratio tool; AA is the floor (4.5:1 text, 3:1 large text/graphics).
- **Reduced motion:** the brand mark never animates. In product, the About page's
  animated banner already gates on the system's reduced-motion setting — any new
  brand motion must do the same (static frame when reduced motion is on).
- **Don't rely on color alone:** status always pairs color with text or shape.
- **Alt text:** logos get `CaeliGlow logo`; decorative star fields get empty alt.

## 10. Tone of voice

Calm, plain, precise. Short sentences. Say what is true, including what isn't
done yet.

| Instead of… | Write… |
|---|---|
| "Revolutionary next-gen desktop experience!!" | "A calm, glowing desktop shell for CachyOS + Hyprland." |
| "Coming soon!" (for the updater) | "Update checking isn't configured yet — set `caeliglow.repository_url` to enable it." |
| "Blazingly fast 500ms animations" | "The bar reveals in 500 ms and settles in 400 ms." |
| Hiding the Noctalia base | "Built as a customization of Noctalia (v5.3.0 base)." |

Example release note (voice in the wild):

> CaeliGlow 1.0.0 is out. It keeps everything Noctalia does and adds calmer
> hover animations, a bottom-edge launcher, and power controls that ask before
> they act. GUI behavior is implemented but needs eyes on a real Hyprland
> session — please report what you see.

## 11. Using the identity

- **README:** use the `<picture>` snippet so the logo adapts to GitHub's theme:
  ```html
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/brand/assets/readme-logo-dark.png">
    <img alt="CaeliGlow logo" src="docs/brand/assets/readme-logo-light.png" width="480">
  </picture>
  ```
  Rendered at 960 px, displayed at 480 px for retina crispness.
- **Social preview:** upload `social-preview.png` (1280 × 640) in the repo's
  *Settings → Social preview*. Don't crop or re-export it.
- **Release artwork:** `banner.png` (1600 × 400) tops release notes; keep the
  v1.0.0-style version pill accurate to the release being announced.
- **Settings UI (About page):** the in-app banner keeps the product's animated
  treatment; use Glow/Moonlight on the Midnight family so it feels continuous
  with this identity. Mark may appear at 32–48 px next to the version string.
- **Website (future):** Midnight hero, Moonlight headline in Inter Bold, one Glow
  accent (a link, a rule, or the mark). Star-field motif only in the hero.

## 12. Files

All assets and their specs: [`asset-inventory.md`](asset-inventory.md).
Regenerate PNGs from the SVG masters with
`assets/sources/generate_assets.py` (needs `cairosvg` and the Inter font).
