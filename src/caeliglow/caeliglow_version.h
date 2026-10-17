#pragma once

// Canonical CaeliGlow version source.
//
// The single source of truth is the CAELIGLOW_VERSION file at the repository
// root (currently "1.0.0"). The build bakes it in via -DCAELIGLOW_VERSION="..."
// (see meson.build). Upstream Noctalia's VERSION file is intentionally left
// untouched so CaeliGlow stays rebase-friendly.

#include <string_view>

namespace caeliglow {

  [[nodiscard]] inline std::string_view version() {
#ifdef CAELIGLOW_VERSION
    return CAELIGLOW_VERSION;
#else
    // Fallback for ad-hoc compiles that don't go through meson (e.g. standalone
    // unit-test builds). Meson builds always define CAELIGLOW_VERSION.
    return "1.0.0-dev";
#endif
  }

  // "CaeliGlow 1.0.0" style display string.
  [[nodiscard]] inline std::string displayVersion() { return std::string("CaeliGlow ") + std::string(version()); }

} // namespace caeliglow
