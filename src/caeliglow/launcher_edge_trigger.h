#pragma once

// CaeliGlow bottom-edge launcher reveal.
//
// A thin, invisible layer-shell strip along the bottom edge of every output.
// When the pointer reaches the strip, Noctalia's existing launcher panel is
// opened with its normal reveal animation; the launcher's own design,
// functionality, and keyboard shortcuts are untouched.
//
// Behavioral reference: Caelestia's bottom-edge launcher trigger
// (reimplemented independently — no Caelestia code). Differences from the
// reference, documented honestly:
//   * Caelestia's trigger is drag-based by default (hover is opt-in); CaeliGlow
//     uses hover-reveal as the task requires.
//   * Caelestia's "gooey" concave-to-edge morph comes from a bespoke SDF blob
//     scene-graph plugin that has no equivalent in Noctalia's renderer and
//     cannot be copied (GPL). The launcher keeps Noctalia's attached
//     clip-reveal, retimed to the reference's 500 ms overshooting curve
//     (see PanelManager::startAttachedOpenAnimation).
//   * The strip is full output width; the reference bounds its trigger to the
//     launcher width.
//
// Staying open: the trigger only *opens* the launcher. Dismissal keeps the
// existing paths (click-away, Escape, activation), so moving the pointer from
// the strip into the revealed launcher never closes it. Re-arming requires
// the pointer to leave the strip while the launcher is closed (plus a poll
// fallback), so dismissing the launcher with the pointer still over the strip
// cannot immediately reopen it.

#include "core/timer_manager.h"
#include "render/scene/input_area.h"
#include "render/scene/input_dispatcher.h"
#include "wayland/layer_surface.h"
#include "wayland/wayland_seat.h"

#include <memory>
#include <vector>

class Application;
class RenderContext;
class WaylandConnection;
struct wl_output;

namespace caeliglow {

  class LauncherEdgeTrigger {
  public:
    explicit LauncherEdgeTrigger(Application* app);
    ~LauncherEdgeTrigger();

    LauncherEdgeTrigger(const LauncherEdgeTrigger&) = delete;
    LauncherEdgeTrigger& operator=(const LauncherEdgeTrigger&) = delete;

    void initialize(WaylandConnection& wayland, RenderContext* renderContext);
    void onOutputChange();
    bool onPointerEvent(const PointerEvent& event);

  private:
    struct EdgeStrip {
      std::unique_ptr<LayerSurface> surface;
      std::unique_ptr<Node> sceneRoot;
      InputDispatcher inputDispatcher;
      bool armed = true;         // may trigger an open on pointer-enter
      bool pointerInside = false;
    };

    struct OutputInstance {
      wl_output* output = nullptr;
      EdgeStrip strip;
    };

    void ensureStrips();
    void buildStrip(EdgeStrip& strip, wl_output* output);
    void maybeOpenLauncher(EdgeStrip& strip, wl_output* output);
    void rearmStrips();

    Application* m_app = nullptr;
    WaylandConnection* m_wayland = nullptr;
    RenderContext* m_renderContext = nullptr;
    Timer m_rearmTimer;

    std::vector<std::unique_ptr<OutputInstance>> m_instances;
  };

} // namespace caeliglow
