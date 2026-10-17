# CaeliGlow

**CaeliGlow 1.0.0** — a standalone desktop shell for **CachyOS + Hyprland**, built as a
customization of [Noctalia](https://github.com/noctalia-dev/noctalia-shell) (v5.3.0 base).

CaeliGlow keeps Noctalia's bar, widgets, launcher UI, settings, and overall appearance, and
adds:

1. **Bar hover animations** tuned to a behavioral reference (500 ms overshooting reveal,
   400 ms conceal) — see `src/caeliglow/caeliglow_animations.h`.
2. **Bottom-edge launcher reveal** — a thin trigger strip along the bottom edge opens
   Noctalia's existing launcher; keyboard shortcuts and dismissal behavior are unchanged.
3. **Right-edge power controls** — a vertical pill with Restart, Shut Down, Suspend, and
   Log Out. Hover/keyboard focus shows an accessible label; hover never executes anything;
   destructive actions need an explicit second click to confirm.
4. **Native CaeliGlow About section** in Settings (not a plugin page), with project info,
   an animated banner, and the update checker.
5. **Update checker** on the About page, configurable via `caeliglow.repository_url`.

## Relationship to other projects

- **Noctalia-based customization.** CaeliGlow is a renamed, modified copy of Noctalia
  (MIT). Upstream's `VERSION` file, copyright notice, and license are preserved; CaeliGlow
  changes are grouped under `src/caeliglow/` and marked with `CaeliGlow:` comments so the
  tree can be rebased against upstream.
- **Caelestia is a behavioral reference only.** Interaction ideas (hover-reveal timing,
  edge-trigger concept) were reimplemented independently. **No Caelestia code or assets
  were copied** (Caelestia is GPL-3.0; copying would have forced a license change).
  There is no affiliation with or endorsement by either the Noctalia or Caelestia projects.

## Prerequisites (CachyOS / Arch)

C++23, so GCC 13+ or Clang 16+. Install the documented Noctalia dependency set:

```sh
sudo pacman -S meson gcc just \
  wayland wayland-protocols \
  libglvnd freetype2 fontconfig \
  cairo pango harfbuzz \
  libxkbcommon glib2 \
  libsecret libsodium \
  sdbus-cpp libpipewire wireplumber polkit \
  pam curl libwebp libjxl libsndfile librsvg \
  libqalculate libxml2 \
  md4c tomlplusplus libical \
  nlohmann-json stb \
  jemalloc
```

Optional at runtime: `upower` (battery), `ddcutil` (monitor brightness), a Secret Service
provider (GNOME Keyring / KWallet / KeePassXC) for durable secrets, and the PipeWire daemon
for audio. See `BUILDING.md` for Fedora/NixOS/Nix lists and troubleshooting.

## Build & run

```sh
just configure && just build && just run     # debug build, runs ./build-debug/noctalia in place
just test                                    # unit tests
```

Release install:

```sh
just configure release && just build release && sudo just install release
```

The installed binary is still named `noctalia` (upstream naming is kept so existing
compositor configs, IPC scripts, and docs keep working); the CaeliGlow identity lives in
the Settings → CaeliGlow section and the `CAELIGLOW_VERSION` file.

## Configuration

`caeliglow.repository_url` — the project's GitHub repository, as `owner/repo` or a full
`https://github.com/owner/repo` URL. **Not set by default** (no repository exists yet).
Once the project is published, set it in your config file:

```toml
[caeliglow]
repository_url = "yourname/caeliglow"
```

This feeds the About page's repository row and the update checker.

## Updating CaeliGlow

> **Update checking isn't configured yet.** No repository URL is set (see above), so the
> About page's update checker reports "Update checking isn't configured yet" and no
> network request is ever made.

When a repository is configured, the checker is *designed* to query the GitHub Releases
API (`https://api.github.com/repos/<owner>/<repo>/releases/latest`), compare semantic
versions, and display release notes. It will **never** download or run remote code and
will never silently replace files. Any future install/upgrade flow must be an explicit
user action with:

1. an automatic backup of the current install,
2. checksum/signature validation of the downloaded artifact,
3. a tested rollback path (see below).

## Upgrade & rollback

- **Upgrade (source):** `git pull`, `just configure release && just build release`,
  `sudo just install release`, then restart the shell from your compositor.
- **Rollback:** keep the previous release build directory (or your distro package).
  Reinstall it with `sudo just install release` from that tree, or
  `sudo pacman -U /var/cache/pacman/pkg/<previous-package>` if you packaged it.
  Your user config is never touched by install/upgrade.

## Tests

- `tests/caeliglow_update_checker_test.cpp` — unit tests for the update checker's semver
  parsing/comparison, repository normalization, and state machine (runs under
  `just test`; also compilable standalone with any C++23 compiler).
- The project's existing ~130 unit tests are untouched and should keep passing.

## Known limitations

- **GUI behavior is not verified in this build environment.** The shell is a Wayland
  layer-shell client; hover animations, the edge trigger, the power strip, and the About
  page were implemented against the real source APIs but can only be visually verified
  in a live Hyprland session. They are marked as unverified until tested on target
  hardware.
- **No gooey concave-to-edge morph.** The reference's signature deform comes from a
  bespoke SDF scene-graph plugin with no equivalent in this renderer (and it cannot be
  copied for license reasons). The launcher keeps Noctalia's attached clip-reveal,
  retimed to the reference's 500 ms overshooting curve.
- **Easing approximation.** The animation system has no arbitrary cubic-bezier support;
  the reveal uses `EaseOutBack` (≈10% overshoot) instead of the reference's
  `(0.38, 1.21, 0.22, 1)`, and the conceal keeps `EaseInQuad` at 400 ms.
- **Hover-reveal, not drag.** The reference's launcher trigger is drag-based by default;
  CaeliGlow uses hover-reveal per the project requirements.
- **Session safety exceeds the reference.** The reference executes session actions on a
  single click with no labels; CaeliGlow requires hover-safe labels plus explicit
  two-click confirmation for destructive actions.
- The original session bar widget's *default click gesture* is disabled (the widget
  itself remains available) so the right-edge strip is the single power control surface.

## Versioning

One canonical source: the `CAELIGLOW_VERSION` file at the repo root (`1.0.0`), baked in
via meson as `-DCAELIGLOW_VERSION`. Upstream Noctalia's `VERSION` (5.3.0) is untouched.

## License

MIT — see `LICENSE` (Noctalia copyright retained, CaeliGlow addition noted).
