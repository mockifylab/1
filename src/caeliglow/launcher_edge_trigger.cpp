#include "caeliglow/launcher_edge_trigger.h"

#include "app/application.h"
#include "shell/panel/panel_manager.h"
#include "ui/builders.h"
#include "wayland/wayland_connection.h"

#include <chrono>
#include <cstdint>

namespace caeliglow {
  namespace {

    // Height (logical px) of the bottom-edge trigger strip. Thin enough to
    // barely intercept pointer input over surfaces beneath it, tall enough to
    // be hit reliably when the pointer is thrown at the screen edge.
    constexpr std::int32_t kStripHeight = 4;

    // Fallback re-arm interval. Re-arming normally happens on pointer-leave;
    // the poll covers the case where the launcher closes while the pointer is
    // nowhere near the strip (no leave event will ever arrive for that).
    constexpr std::chrono::milliseconds kRearmPollInterval{400};

    [[nodiscard]] bool launcherOpen() { return PanelManager::instance().isOpenPanel("launcher"); }

  } // namespace

  LauncherEdgeTrigger::LauncherEdgeTrigger(Application* app) : m_app(app) {}

  LauncherEdgeTrigger::~LauncherEdgeTrigger() {
    m_rearmTimer.stop();
    m_instances.clear();
  }

  void LauncherEdgeTrigger::initialize(WaylandConnection& wayland, RenderContext* renderContext) {
    m_wayland = &wayland;
    m_renderContext = renderContext;
    m_rearmTimer.startRepeating(kRearmPollInterval, [this]() { rearmStrips(); });
  }

  void LauncherEdgeTrigger::onOutputChange() {
    if (m_wayland == nullptr) {
      return;
    }
    m_instances.clear();
    ensureStrips();
  }

  void LauncherEdgeTrigger::ensureStrips() {
    for (const auto& out : m_wayland->outputs()) {
      if (!out.done || out.connectorName.empty()) {
        continue;
      }
      auto instance = std::make_unique<OutputInstance>();
      instance->output = out.output;
      buildStrip(instance->strip, out.output);
      m_instances.push_back(std::move(instance));
    }
  }

  void LauncherEdgeTrigger::buildStrip(EdgeStrip& strip, wl_output* output) {
    // Same layer policy as hot corners: sit on the highest layer any bar
    // occupies on this output so the strip is never occluded by shell chrome,
    // but stays out of the way of fullscreen surfaces when the bar is on Top.
    const LayerShellLayer layer = m_app->hotCornerLayerForOutput(output);

    LayerSurfaceConfig surfaceConfig{
        .nameSpace = "caeliglow_launcher_edge",
        .layer = layer,
        .anchor = LayerShellAnchor::Bottom | LayerShellAnchor::Left | LayerShellAnchor::Right,
        .width = 0, // stretched by the left+right anchors: full output width
        .height = static_cast<std::uint32_t>(kStripHeight),
        // -1: ignore other surfaces' exclusive zones so the strip anchors to
        // the absolute screen edge instead of being pushed inward by the bar.
        .exclusiveZone = -1,
    };

    strip.surface = std::make_unique<LayerSurface>(*m_wayland, surfaceConfig);
    strip.surface->initialize(output);

    auto inputArea = ui::inputArea({});
    inputArea->setPosition(0, 0);
    // The surface is stretched by the compositor to the full output width, but
    // the logical width isn't known here; size the input area generously in x.
    // Pointer coordinates are surface-local, so the oversized area can only
    // ever hit-test within the real surface bounds.
    inputArea->setSize(100000.0F, static_cast<float>(kStripHeight));
    inputArea->setOnEnter([this, &strip, output](const InputArea::PointerData&) {
      strip.pointerInside = true;
      maybeOpenLauncher(strip, output);
    });
    inputArea->setOnLeave([this, &strip]() {
      strip.pointerInside = false;
      // Re-arm only while the launcher is closed: if the pointer moved from
      // the strip into the open launcher, the strip must stay disarmed so the
      // pointer returning to the edge cannot reopen what was just dismissed.
      if (!launcherOpen()) {
        strip.armed = true;
      }
    });

    strip.sceneRoot = std::move(inputArea);
    strip.inputDispatcher.setSceneRoot(strip.sceneRoot.get());

    strip.surface->setRenderContext(m_renderContext);
    strip.surface->setSceneRoot(strip.sceneRoot.get());
    strip.surface->requestRedraw();
  }

  void LauncherEdgeTrigger::maybeOpenLauncher(EdgeStrip& strip, wl_output* output) {
    if (!strip.armed || launcherOpen()) {
      return;
    }
    strip.armed = false;
    PanelManager::instance().openPanel("launcher", PanelOpenRequest{.output = output});
  }

  void LauncherEdgeTrigger::rearmStrips() {
    // Poll fallback: if the launcher was dismissed while the pointer was
    // nowhere near the strip, no leave event re-arms it — do it here.
    if (launcherOpen()) {
      return;
    }
    for (const auto& instance : m_instances) {
      if (!instance->strip.pointerInside) {
        instance->strip.armed = true;
      }
    }
  }

  bool LauncherEdgeTrigger::onPointerEvent(const PointerEvent& event) {
    if (event.surface == nullptr) {
      return false;
    }
    for (const auto& instance : m_instances) {
      EdgeStrip& strip = instance->strip;
      if (strip.surface && event.surface == strip.surface->wlSurface()) {
        switch (event.type) {
        case PointerEvent::Type::Enter:
          strip.inputDispatcher.pointerEnter(static_cast<float>(event.sx), static_cast<float>(event.sy), event.serial);
          return true;
        case PointerEvent::Type::Leave:
          strip.inputDispatcher.pointerLeave();
          return true;
        case PointerEvent::Type::Motion:
          strip.inputDispatcher.pointerMotion(static_cast<float>(event.sx), static_cast<float>(event.sy), event.serial);
          return true;
        case PointerEvent::Type::Button:
          return strip.inputDispatcher.pointerButton(
              static_cast<float>(event.sx), static_cast<float>(event.sy), event.button, event.pressed, event.serial,
              event.time, event.touch
          );
        case PointerEvent::Type::Axis:
          return strip.inputDispatcher.pointerAxis(
              static_cast<float>(event.sx), static_cast<float>(event.sy), event.axis, event.axisSource, event.axisValue,
              event.axisDiscrete, event.axisValue120, event.axisLines
          );
        }
        return false;
      }
    }
    return false;
  }

} // namespace caeliglow
