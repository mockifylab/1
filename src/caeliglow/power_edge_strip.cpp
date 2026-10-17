#include "caeliglow/power_edge_strip.h"

#include "app/application.h"
#include "i18n/i18n.h"
#include "shell/session/session_action_meta.h"
#include "shell/session/session_action_runner.h"
#include "shell/tooltip/tooltip_manager.h"
#include "ui/builders.h"
#include "ui/controls/button.h"
#include "wayland/wayland_connection.h"

#include <algorithm>
#include <chrono>
#include <cstdint>

namespace caeliglow {
  namespace {

    constexpr float kSurfaceWidth = 60.0F;
    constexpr float kButtonSize = 40.0F;
    constexpr float kGap = 8.0F;
    constexpr float kPadding = 10.0F;
    constexpr float kPillRadius = 16.0F;
    constexpr float kMarginRight = 8.0F;
    // Armed (awaiting confirmation) state expires after this long.
    constexpr std::chrono::milliseconds kConfirmTimeout{6000};

    constexpr float kPillHeight = kPadding * 2.0F + 4.0F * kButtonSize + 3.0F * kGap;

  } // namespace

  // Defined here (not in the header) to keep the private ActionDef type out of
  // namespace-scope declarations.
  std::vector<PowerEdgeStrip::ActionDef> PowerEdgeStrip::defaultActions() {
    return {
        {.action = "reboot", .labelKey = "caeliglow.power.restart", .destructive = true},
        {.action = "shutdown", .labelKey = "caeliglow.power.shutdown", .destructive = true},
        {.action = "suspend", .labelKey = "caeliglow.power.suspend", .destructive = false},
        {.action = "logout", .labelKey = "caeliglow.power.logout", .destructive = true},
    };
  }

  PowerEdgeStrip::PowerEdgeStrip(Application* app) : m_app(app) {}

  PowerEdgeStrip::~PowerEdgeStrip() { m_instances.clear(); }

  void PowerEdgeStrip::initialize(WaylandConnection& wayland, RenderContext* renderContext) {
    m_wayland = &wayland;
    m_renderContext = renderContext;
  }

  void PowerEdgeStrip::onOutputChange() {
    if (m_wayland == nullptr) {
      return;
    }
    m_instances.clear();
    ensureStrips();
  }

  void PowerEdgeStrip::ensureStrips() {
    for (const auto& out : m_wayland->outputs()) {
      if (!out.done || out.connectorName.empty()) {
        continue;
      }
      auto instance = std::make_unique<OutputInstance>();
      instance->output = out.output;
      buildStrip(*instance, out);
      m_instances.push_back(std::move(instance));
    }
  }

  void PowerEdgeStrip::buildStrip(OutputInstance& instance, const WaylandOutput& out) {
    // Same layer policy as hot corners: above shell chrome, below transient
    // panels, and out of the way of fullscreen surfaces.
    const LayerShellLayer layer = m_app->hotCornerLayerForOutput(out.output);

    const std::int32_t logicalH = out.effectiveLogicalHeight();
    const std::int32_t marginTop =
        logicalH > static_cast<std::int32_t>(kPillHeight) ? (logicalH - static_cast<std::int32_t>(kPillHeight)) / 2 : 0;

    LayerSurfaceConfig surfaceConfig{
        .nameSpace = "caeliglow_power_edge",
        .layer = layer,
        .anchor = LayerShellAnchor::Top | LayerShellAnchor::Right,
        .width = static_cast<std::uint32_t>(kSurfaceWidth),
        .height = static_cast<std::uint32_t>(kPillHeight),
        .exclusiveZone = -1, // overlay the edge; never reserve space
        .marginTop = marginTop,
        .marginRight = static_cast<std::int32_t>(kMarginRight),
        .keyboard = LayerShellKeyboard::None,
    };

    instance.surface = std::make_unique<LayerSurface>(*m_wayland, surfaceConfig);
    instance.surface->initialize(out.output);

    auto root = ui::node({});
    root->setSize(kSurfaceWidth, kPillHeight);

    // Pill background in the shell's theme style.
    auto* bg = root->addChild(ui::box({.radius = kPillRadius}));
    bg->setPosition(4.0F, 4.0F);
    bg->setSize(kSurfaceWidth - 8.0F, kPillHeight - 8.0F);

    instance.actions = defaultActions();
    instance.buttons.reserve(instance.actions.size());
    for (std::size_t i = 0; i < instance.actions.size(); ++i) {
      const ActionDef& def = instance.actions[i];
      const std::string label = i18n::tr(def.labelKey);
      const float y = kPadding + static_cast<float>(i) * (kButtonSize + kGap);

      Button* btn = nullptr;
      auto buttonNode = ui::button({
          .out = &btn,
          .glyph = std::string(session_action::defaultGlyph(def.action)),
          .tooltip = label,
          .minWidth = kButtonSize,
          .minHeight = kButtonSize,
          .onClick = [this, inst = &instance, i]() { onActionClicked(*inst, i); },
      });
      buttonNode->setPosition((kSurfaceWidth - kButtonSize) / 2.0F, y);
      root->addChild(std::move(buttonNode));
      instance.buttons.push_back(btn);

      // Keyboard focus shows the same accessible label as hover. (The strip
      // itself doesn't take keyboard focus; this path exists so the label is
      // available to any focus-driven interaction, e.g. assistive tools.)
      if (InputArea* area = btn->inputArea()) {
        area->setOnFocusGain([this, inst = &instance, btn]() { showFocusLabel(*inst, btn, true); });
        area->setOnFocusLoss([this, inst = &instance, btn]() { showFocusLabel(*inst, btn, false); });
      }
    }

    instance.sceneRoot = std::move(root);
    instance.inputDispatcher.setSceneRoot(instance.sceneRoot.get());

    instance.surface->setRenderContext(m_renderContext);
    instance.surface->setSceneRoot(instance.sceneRoot.get());
    instance.surface->requestRedraw();
  }

  void PowerEdgeStrip::showFocusLabel(OutputInstance& instance, Button* button, bool show) {
    if (instance.surface == nullptr || button == nullptr || button->inputArea() == nullptr) {
      return;
    }
    if (show) {
      TooltipManager::instance().onHoverChange(
          button->inputArea(), instance.surface->layerSurface(), instance.output
      );
    } else {
      TooltipManager::instance().onHoverChange(nullptr, nullptr, nullptr);
    }
  }

  void PowerEdgeStrip::onActionClicked(OutputInstance& instance, std::size_t index) {
    if (index >= instance.actions.size() || index >= instance.buttons.size()) {
      return;
    }
    const ActionDef& def = instance.actions[index];
    Button* button = instance.buttons[index];

    // Hover never arms or executes: only clicks reach this handler.
    if (!def.destructive) {
      executeAction(def.action);
      return;
    }

    if (instance.armedAction == def.action) {
      // Second click: confirmed.
      executeAction(def.action);
      disarm(instance);
      return;
    }

    // First click: arm and require an explicit second click.
    disarm(instance);
    instance.armedAction = def.action;
    if (button != nullptr) {
      button->setSelected(true);
      button->setTooltip(i18n::tr("caeliglow.power.confirmHint", "action", i18n::tr(def.labelKey)));
    }
    instance.confirmTimer.start(kConfirmTimeout, [this, inst = &instance]() { disarm(*inst); });
  }

  void PowerEdgeStrip::disarm(OutputInstance& instance) {
    instance.confirmTimer.stop();
    if (!instance.armedAction.empty()) {
      for (std::size_t i = 0; i < instance.actions.size() && i < instance.buttons.size(); ++i) {
        if (instance.actions[i].action == instance.armedAction && instance.buttons[i] != nullptr) {
          instance.buttons[i]->setSelected(false);
          instance.buttons[i]->setTooltip(i18n::tr(instance.actions[i].labelKey));
        }
      }
      instance.armedAction.clear();
    }
  }

  void PowerEdgeStrip::executeAction(const std::string& action) {
    SessionPanelActionConfig cfg;
    cfg.action = action;
    // Same runner as the session panel: identical command auto-detection and
    // UWSM-aware logout behavior.
    m_app->sessionActionRunner().invoke(cfg);
  }

  bool PowerEdgeStrip::onPointerEvent(const PointerEvent& event) {
    if (event.surface == nullptr) {
      return false;
    }
    for (const auto& instance : m_instances) {
      if (instance->surface && event.surface == instance->surface->wlSurface()) {
        switch (event.type) {
        case PointerEvent::Type::Enter:
          instance->inputDispatcher.pointerEnter(
              static_cast<float>(event.sx), static_cast<float>(event.sy), event.serial
          );
          return true;
        case PointerEvent::Type::Leave:
          instance->inputDispatcher.pointerLeave();
          return true;
        case PointerEvent::Type::Motion:
          instance->inputDispatcher.pointerMotion(
              static_cast<float>(event.sx), static_cast<float>(event.sy), event.serial
          );
          return true;
        case PointerEvent::Type::Button:
          return instance->inputDispatcher.pointerButton(
              static_cast<float>(event.sx), static_cast<float>(event.sy), event.button, event.pressed, event.serial,
              event.time, event.touch
          );
        case PointerEvent::Type::Axis:
          return instance->inputDispatcher.pointerAxis(
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
