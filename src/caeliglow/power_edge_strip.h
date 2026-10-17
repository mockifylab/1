#pragma once

// CaeliGlow right-edge power controls.
//
// A slim vertical pill anchored to the right edge of every output, holding
// four icon buttons in Noctalia's visual style: Restart, Shut Down, Suspend,
// Log Out (top to bottom).
//
// Safety model (deliberately stricter than the Caelestia behavioral reference,
// which executes session actions immediately with no labels):
//   * Hovering a button never executes anything; it shows an accessible
//     tooltip label. Keyboard focus on a button shows the same label.
//   * Destructive actions (restart, shut down, log out) require explicit
//     confirmation: the first click arms the button (highlighted, tooltip
//     changes to "click again to confirm"), the second click executes. The
//     armed state expires after a few seconds or when another button is
//     pressed. Suspend is non-destructive and executes on a single click.
//   * Execution reuses SessionActionRunner, the same runner as the session
//     panel, so command auto-detection and the UWSM-aware logout path behave
//     identically.
//
// After these controls are verified on target hardware, the original
// power-menu entry (the session bar widget's default click gesture) is
// disabled so there are no duplicate controls (see widget_gesture_defaults).

#include "core/timer_manager.h"
#include "render/scene/input_dispatcher.h"
#include "wayland/layer_surface.h"
#include "wayland/wayland_seat.h"

#include <memory>
#include <string>
#include <vector>

class Application;
class Button;
class RenderContext;
class WaylandConnection;
struct wl_output;

namespace caeliglow {

  class PowerEdgeStrip {
  public:
    explicit PowerEdgeStrip(Application* app);
    ~PowerEdgeStrip();

    PowerEdgeStrip(const PowerEdgeStrip&) = delete;
    PowerEdgeStrip& operator=(const PowerEdgeStrip&) = delete;

    void initialize(WaylandConnection& wayland, RenderContext* renderContext);
    void onOutputChange();
    bool onPointerEvent(const PointerEvent& event);

  private:
    struct ActionDef {
      std::string action; // session action id: "reboot" | "shutdown" | "suspend" | "logout"
      std::string labelKey; // i18n key for the accessible label
      bool destructive = true;
    };

    struct OutputInstance {
      wl_output* output = nullptr;
      std::unique_ptr<LayerSurface> surface;
      std::unique_ptr<Node> sceneRoot;
      InputDispatcher inputDispatcher;
      std::vector<Button*> buttons; // owned by the scene
      std::vector<ActionDef> actions;
      std::string armedAction; // action id awaiting confirmation; empty = none
      Timer confirmTimer;
    };

    static std::vector<ActionDef> defaultActions();
    void ensureStrips();
    void buildStrip(OutputInstance& instance, const struct WaylandOutput& out);
    void onActionClicked(OutputInstance& instance, std::size_t index);
    void executeAction(const std::string& action);
    void disarm(OutputInstance& instance);
    void showFocusLabel(OutputInstance& instance, Button* button, bool show);

    Application* m_app = nullptr;
    WaylandConnection* m_wayland = nullptr;
    RenderContext* m_renderContext = nullptr;

    std::vector<std::unique_ptr<OutputInstance>> m_instances;
  };

} // namespace caeliglow
