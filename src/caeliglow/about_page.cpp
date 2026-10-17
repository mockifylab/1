#include "caeliglow/about_page.h"

#include "caeliglow/caeliglow_version.h"
#include "caeliglow/update_checker.h"
#include "config/config_types.h"
#include "i18n/i18n.h"
#include "render/animation/animation_manager.h"
#include "render/animation/motion_service.h"
#include "render/scene/node.h"
#include "shell/settings/settings_content_common.h"
#include "ui/builders.h"
#include "ui/controls/button.h"
#include "ui/controls/label.h"
#include "ui/palette.h"
#include "ui/style.h"

#include <functional>
#include <memory>

namespace caeliglow {
  namespace {

    // Animated wordmark banner. A gentle "breathing" pulse on the glyph
    // (opacity) and wordmark (scale), implemented with the scene's
    // AnimationManager. Lifetime-safe: each animation is owned by its target
    // node, so Node::~Node cancels it if the page is rebuilt mid-flight.
    // Reduced motion: the global MotionService toggle is honored — when motion
    // is disabled the banner renders statically and no loop is started.
    void startBannerAnimation(Node* glyphNode, Node* wordmarkNode) {
      if (glyphNode == nullptr || wordmarkNode == nullptr) {
        return;
      }
      AnimationManager* mgr = glyphNode->animationManager();
      if (mgr == nullptr || !MotionService::instance().enabled()) {
        glyphNode->setOpacity(1.0F);
        wordmarkNode->setScale(1.0F);
        return;
      }

      constexpr float kPulseMs = 2200.0F;
      auto pulseGlyph = std::make_shared<std::function<void(bool)>>();
      auto pulseWordmark = std::make_shared<std::function<void(bool)>>();
      *pulseGlyph = [mgr, glyphNode, pulseGlyph](bool glowUp) {
        mgr->animate(
            glowUp ? 0.55F : 1.0F, glowUp ? 1.0F : 0.55F, kPulseMs, Easing::EaseInOutQuad,
            [glyphNode](float v) { glyphNode->setOpacity(v); },
            [mgr, glyphNode, pulseGlyph, glowUp]() {
              if (!MotionService::instance().enabled()) {
                glyphNode->setOpacity(1.0F);
                return;
              }
              (*pulseGlyph)(!glowUp);
            },
            glyphNode
        );
      };
      *pulseWordmark = [mgr, wordmarkNode, pulseWordmark](bool grow) {
        mgr->animate(
            grow ? 1.0F : 1.035F, grow ? 1.035F : 1.0F, kPulseMs, Easing::EaseInOutQuad,
            [wordmarkNode](float v) { wordmarkNode->setScale(v); },
            [mgr, wordmarkNode, pulseWordmark, grow]() {
              if (!MotionService::instance().enabled()) {
                wordmarkNode->setScale(1.0F);
                return;
              }
              (*pulseWordmark)(!grow);
            },
            wordmarkNode
        );
      };
      (*pulseGlyph)(true);
      (*pulseWordmark)(true);
    }

    std::unique_ptr<Flex> infoRow(
        float scale, std::string_view title, std::string_view value, bool dimValue = false
    ) {
      auto row = ui::row({.align = FlexAlign::Center, .gap = Style::spaceSm * scale, .fillWidth = true});
      row->addChild(settings::makeLabel(
          title, Style::fontSizeBody * scale, colorSpecFromRole(ColorRole::OnSurfaceVariant)
      ));
      row->addChild(settings::makeLabel(
          value, Style::fontSizeBody * scale,
          colorSpecFromRole(dimValue ? ColorRole::OnSurfaceVariant : ColorRole::OnSurface), FontWeight::Medium
      ));
      return row;
    }

    UpdateCheckResult runUpdateCheck(const Config* cfg) {
      UpdateConfig uc;
      std::string fetchError;
      if (cfg != nullptr && !cfg->caeliGlow.repositoryUrl.empty()) {
        uc.repository = cfg->caeliGlow.repositoryUrl;
        // No repository exists yet and no fetcher is wired: the check is
        // designed to query latestReleaseApiUrl() once a fetcher exists.
        fetchError = i18n::tr("caeliglow.about.fetchNotImplemented");
      }
      return evaluateUpdate(uc, caeliglow::version(), std::nullopt, std::move(fetchError));
    }

  } // namespace

  void addSettingsCaeliGlow(Flex& content, AboutPageContext ctx) {
    if (ctx.selectedSection != "caeliglow") {
      return;
    }
    const float scale = ctx.scale;

    auto sectionCol = ui::column({
        .align = FlexAlign::Stretch,
        .gap = Style::spaceMd * scale,
        .padding = Style::spaceLg * scale,
        .fill = clearColorSpec(),
        .fillWidth = true,
    });
    Flex* section = sectionCol.get();
    content.addChild(std::move(sectionCol));

    // Title row.
    auto titleRow = ui::row(
        {.align = FlexAlign::Center, .gap = Style::spaceSm * scale, .fillWidth = true},
        ui::glyph({
            .glyph = "sparkles",
            .glyphSize = Style::fontSizeHeader * scale,
            .color = colorSpecFromRole(ColorRole::Primary),
        }),
        settings::makeLabel(
            i18n::tr("settings.navigation.sections.caeliglow"), Style::fontSizeHeader * scale,
            colorSpecFromRole(ColorRole::Primary), FontWeight::Bold
        )
    );
    if (ctx.pageTitleRow != nullptr) {
      ctx.pageTitleRow->addChild(std::move(titleRow));
    } else {
      section->addChild(std::move(titleRow));
    }

    // Animated banner: breathing wordmark. Original artwork-free animation —
    // no Caelestia code or assets involved.
    Node* bannerGlyph = nullptr;
    Node* bannerWordmark = nullptr;
    {
      auto banner = ui::column({
          .align = FlexAlign::Center,
          .justify = FlexJustify::Center,
          .gap = Style::spaceXs * scale,
          .padding = Style::spaceLg * scale,
          .fill = colorSpecFromRole(ColorRole::PrimaryContainer, 0.35F),
          .radius = Style::scaledRadiusLg(scale),
          .fillWidth = true,
      });
      auto glyphNode = ui::glyph({
          .glyph = "sparkles",
          .glyphSize = 44.0F * scale,
          .color = colorSpecFromRole(ColorRole::Primary),
      });
      bannerGlyph = banner->addChild(std::move(glyphNode));
      auto wordmarkNode = settings::makeLabel(
          "CaeliGlow", Style::fontSizeHeader * 1.6F * scale, colorSpecFromRole(ColorRole::OnSurface), FontWeight::Bold
      );
      bannerWordmark = banner->addChild(std::move(wordmarkNode));
      banner->addChild(settings::makeLabel(
          std::string(caeliglow::version()) + "  ·  " + i18n::tr("caeliglow.about.tagline"),
          Style::fontSizeBody * scale, colorSpecFromRole(ColorRole::OnSurfaceVariant)
      ));
      section->addChild(std::move(banner));
    }
    startBannerAnimation(bannerGlyph, bannerWordmark);

    // Project facts.
    section->addChild(infoRow(scale, i18n::tr("caeliglow.about.version"), caeliglow::version()));
    {
      std::string basedOn = i18n::tr("caeliglow.about.basedOn", "version", NOCTALIA_VERSION);
      section->addChild(infoRow(scale, "", basedOn, true));
    }
    section->addChild(infoRow(scale, i18n::tr("caeliglow.about.developer"), "Aasif"));

    // Repository link (configurable; nothing invented).
    {
      std::string repoText;
      bool dim = false;
      if (ctx.config != nullptr && !ctx.config->caeliGlow.repositoryUrl.empty()) {
        repoText = normalizeRepository(UpdateConfig{ctx.config->caeliGlow.repositoryUrl});
        if (repoText.empty()) {
          repoText = ctx.config->caeliGlow.repositoryUrl;
        }
      } else {
        repoText = i18n::tr("caeliglow.about.repositoryNotConfigured");
        dim = true;
      }
      section->addChild(infoRow(scale, i18n::tr("caeliglow.about.repository"), repoText, dim));
    }

    // Update checker.
    {
      Label* statusLabel = nullptr;
      auto updateRow = ui::row({.align = FlexAlign::Center, .gap = Style::spaceSm * scale, .fillWidth = true});
      updateRow->addChild(settings::makeLabel(
          i18n::tr("caeliglow.about.updateStatus"), Style::fontSizeBody * scale,
          colorSpecFromRole(ColorRole::OnSurfaceVariant)
      ));
      auto statusNode = ui::label({.out = &statusLabel, .text = describeState(runUpdateCheck(ctx.config))});
      updateRow->addChild(std::move(statusNode));
      updateRow->addChild(ui::button({
          .text = i18n::tr("caeliglow.about.checkForUpdates"),
          .onClick = [statusLabel, cfg = ctx.config]() {
            if (statusLabel != nullptr) {
              statusLabel->setText(describeState(runUpdateCheck(cfg)));
            }
          },
      }));
      section->addChild(std::move(updateRow));

      // Transparency: show which API endpoint a future fetcher would query.
      if (ctx.config != nullptr && !ctx.config->caeliGlow.repositoryUrl.empty()) {
        const std::string apiUrl = latestReleaseApiUrl(UpdateConfig{ctx.config->caeliGlow.repositoryUrl});
        if (!apiUrl.empty()) {
          section->addChild(infoRow(scale, "", i18n::tr("caeliglow.about.wouldQuery", "url", apiUrl), true));
        }
      }
    }

    // Attribution.
    section->addChild(settings::makeLabel(
        i18n::tr("caeliglow.about.attribution"), Style::fontSizeCaption * scale,
        colorSpecFromRole(ColorRole::OnSurfaceVariant)
    ));
  }

} // namespace caeliglow
