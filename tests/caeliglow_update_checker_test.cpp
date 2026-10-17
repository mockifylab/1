// Unit tests for the CaeliGlow update checker (pure logic, no network).
#include "caeliglow/update_checker.h"

#include "test_check.h"

#include <string>

using namespace caeliglow;

namespace {

  void semverParsing() {
    const auto v = parseSemVer("1.0.0");
    TEST_CHECK(v.has_value() && v->major == 1 && v->minor == 0 && v->patch == 0 && v->prerelease.empty());

    const auto tagged = parseSemVer("v2.10.3");
    TEST_CHECK(tagged.has_value() && tagged->major == 2 && tagged->minor == 10 && tagged->patch == 3);

    const auto pre = parseSemVer("1.0.0-rc.1+build.5");
    TEST_CHECK(pre.has_value() && pre->prerelease == "rc.1");

    TEST_CHECK(!parseSemVer("").has_value());
    TEST_CHECK(!parseSemVer("1.0").has_value());
    TEST_CHECK(!parseSemVer("1.0.0.0").has_value());
    TEST_CHECK(!parseSemVer("1.01.0").has_value()); // leading zeros rejected
    TEST_CHECK(!parseSemVer("a.b.c").has_value());
    TEST_CHECK(!parseSemVer("1.0.0-").has_value());
    TEST_CHECK(!parseSemVer("1.0.0-01").has_value());
  }

  void semverComparison() {
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0"), *parseSemVer("1.0.0")) == 0);
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0"), *parseSemVer("1.0.1")) < 0);
    TEST_CHECK(compareSemVer(*parseSemVer("1.2.0"), *parseSemVer("1.10.0")) < 0);
    TEST_CHECK(compareSemVer(*parseSemVer("2.0.0"), *parseSemVer("1.99.99")) > 0);
    // Release beats prerelease; numeric prerelease identifiers compare numerically.
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0-alpha"), *parseSemVer("1.0.0")) < 0);
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0-alpha.1"), *parseSemVer("1.0.0-alpha.2")) < 0);
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0-alpha.10"), *parseSemVer("1.0.0-alpha.2")) > 0);
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0-1"), *parseSemVer("1.0.0-alpha")) < 0);
    // Current shipped version parses and compares sanely.
    TEST_CHECK(compareSemVer(*parseSemVer("1.0.0"), *parseSemVer("v1.0.0")) == 0);
  }

  void repositoryNormalization() {
    TEST_CHECK(normalizeRepository(UpdateConfig{}) == "");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "octo/shipit"}) == "octo/shipit");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "https://github.com/octo/shipit"}) == "octo/shipit");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "https://github.com/octo/shipit/"}) == "octo/shipit");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "https://github.com/octo/shipit.git"}) == "octo/shipit");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "git@github.com:octo/shipit.git"}) == "octo/shipit");
    // Rejects anything that isn't exactly owner/repo.
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "not-a-repo"}) == "");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "https://github.com/octo/shipit/releases"}) == "");
    TEST_CHECK(normalizeRepository(UpdateConfig{.repository = "https://example.com/octo/shipit"}) == "");

    TEST_CHECK(latestReleaseApiUrl(UpdateConfig{}) == "");
    TEST_CHECK(
        latestReleaseApiUrl(UpdateConfig{.repository = "octo/shipit"})
        == "https://api.github.com/repos/octo/shipit/releases/latest"
    );
  }

  void evaluationStates() {
    // No repository configured: the honest "not configured" state.
    {
      const auto r = evaluateUpdate(UpdateConfig{}, "1.0.0", std::nullopt);
      TEST_CHECK(r.state == UpdateCheckState::NotConfigured);
      TEST_CHECK(describeState(r) == "Update checking isn't configured yet.");
      TEST_CHECK(latestReleaseApiUrl(UpdateConfig{}) == "");
    }
    // Configured but the fetch failed.
    {
      const auto r = evaluateUpdate(UpdateConfig{.repository = "octo/shipit"}, "1.0.0", std::nullopt, "timeout");
      TEST_CHECK(r.state == UpdateCheckState::CheckFailed);
      TEST_CHECK(r.error == "timeout");
    }
    // Up to date / update available.
    {
      ReleaseInfo rel{.tagName = "v1.0.0", .name = "1.0.0", .body = "notes", .htmlUrl = "https://x/y"};
      const auto same = evaluateUpdate(UpdateConfig{.repository = "octo/shipit"}, "1.0.0", rel);
      TEST_CHECK(same.state == UpdateCheckState::UpToDate);
      TEST_CHECK(same.latestVersion == "v1.0.0");
      TEST_CHECK(same.releaseNotes == "notes");
      TEST_CHECK(same.releaseUrl == "https://x/y");

      rel.tagName = "v1.1.0";
      const auto newer = evaluateUpdate(UpdateConfig{.repository = "octo/shipit"}, "1.0.0", rel);
      TEST_CHECK(newer.state == UpdateCheckState::UpdateAvailable);
      TEST_CHECK(describeState(newer) == "Update available: v1.1.0.");

      // Older release than running: still up to date.
      rel.tagName = "v0.9.0";
      TEST_CHECK(evaluateUpdate(UpdateConfig{.repository = "octo/shipit"}, "1.0.0", rel).state == UpdateCheckState::UpToDate);
    }
    // Unparseable tags fail loudly instead of inventing an answer.
    {
      ReleaseInfo rel{.tagName = "banana"};
      const auto r = evaluateUpdate(UpdateConfig{.repository = "octo/shipit"}, "1.0.0", rel);
      TEST_CHECK(r.state == UpdateCheckState::CheckFailed);
    }
  }

} // namespace

int main() {
  semverParsing();
  semverComparison();
  repositoryNormalization();
  evaluationStates();
  return 0;
}
