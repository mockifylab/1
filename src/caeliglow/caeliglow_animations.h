#pragma once

// CaeliGlow animation timings for the bar hover reveal/conceal.
//
// Behavioral reference: Caelestia reveals its bar over 500 ms with an
// overshooting cubic-bezier (0.38, 1.21, 0.22, 1) and conceals over 400 ms
// with the Material emphasized easing curve. Reimplemented independently here
// (no Caelestia code); adapted to what Noctalia's animation system supports:
//
//   * Noctalia's Easing enum has no arbitrary cubic-bezier, so the reveal uses
//     Easing::EaseOutBack (standard 10% overshoot) — the closest available
//     match to the reference's overshooting ease-out. The overshoot only
//     affects the bar's slide transform (hideOpacity is never used as a raw
//     alpha), so it reads as a gentle spring settle.
//   * The conceal keeps Easing::EaseInQuad at 400 ms; the reference's
//     multi-segment emphasized curve has no equivalent in the enum.
//
// These constants are used by the bar auto-hide reveal/conceal paths
// (Bar::revealAutoHideBar, Bar::startHideFadeOut, and the IPC visibility
// toggle). All animations run through AnimationManager, which honors the
// global reduced-motion setting.

namespace caeliglow {

  inline constexpr float kBarRevealMs = 500.0F;
  inline constexpr float kBarConcealMs = 400.0F;

} // namespace caeliglow
