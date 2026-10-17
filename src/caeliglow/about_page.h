#pragma once

// CaeliGlow native About section for Noctalia Settings.
//
// This is a first-class settings section (SettingsSection::CaeliGlow), not a
// plugin page: it is registered in the settings registry descriptors and
// renders fully custom content here, following the Plugins section's pattern.
//
// Contents: an animated wordmark banner (lightweight, honors the global
// reduced-motion setting), project name, version, developer, a configurable
// repository link, the update checker, and attribution stating that CaeliGlow
// is a Noctalia-based customization with Caelestia as a behavioral reference
// only (no affiliation or endorsement).

#include <string_view>

class Flex;
struct Config;

namespace caeliglow {

  struct AboutPageContext {
    float scale = 1.0F;
    std::string_view selectedSection;
    const Config* config = nullptr;
    Flex* pageTitleRow = nullptr;
  };

  // Render the CaeliGlow About section into `content` when
  // ctx.selectedSection == "caeliglow".
  void addSettingsCaeliGlow(Flex& content, AboutPageContext ctx);

} // namespace caeliglow
