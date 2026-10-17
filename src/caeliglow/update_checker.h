#pragma once

// CaeliGlow update checker — pure decision logic, no I/O.
//
// Design:
//   * The checker is configured with the project's eventual GitHub repository
//     ("owner/repo" or a full https://github.com/owner/repo URL). Until a
//     repository exists and is configured, every check reports NotConfigured
//     and the About page shows "Update checking isn't configured yet".
//   * When configured, the intended data source is the GitHub Releases API
//     (latestReleaseApiUrl()). Networking is injected by the caller: the UI
//     fetches ReleaseInfo and passes it to evaluateUpdate(). This keeps this
//     unit dependency-free and testable, and guarantees we never run remote
//     code or silently replace files — any future install step must be an
//     explicit user action with backup, validation, and rollback (see README).
//   * Version comparison follows semver precedence (build metadata ignored).

#include <optional>
#include <string>
#include <string_view>

namespace caeliglow {

  struct SemVer {
    int major = 0;
    int minor = 0;
    int patch = 0;
    std::string prerelease; // empty = final release (higher precedence than any prerelease)
  };

  // Parses "1.2.3", "v1.2.3", "1.2.3-rc.1+build". Returns nullopt on invalid input.
  [[nodiscard]] std::optional<SemVer> parseSemVer(std::string_view text);

  // Semver precedence: -1 if a < b, 0 if equal, 1 if a > b.
  [[nodiscard]] int compareSemVer(const SemVer& a, const SemVer& b);

  enum class UpdateCheckState {
    NotConfigured, // no repository configured yet — nothing to check against
    UpToDate,      // latest release is not newer than the running version
    UpdateAvailable,
    CheckFailed, // configured, but the check could not complete (network, bad data, ...)
  };

  struct ReleaseInfo {
    std::string tagName; // e.g. "v1.1.0"
    std::string name;
    std::string body; // release notes (markdown)
    std::string htmlUrl;
  };

  struct UpdateConfig {
    // GitHub repository: "owner/repo" or "https://github.com/owner/repo"
    // (".git" suffix and trailing slashes are tolerated). Empty = not configured.
    std::string repository;
  };

  // Normalized "owner/repo", or empty when the config does not identify a repo.
  [[nodiscard]] std::string normalizeRepository(const UpdateConfig& config);

  // "https://api.github.com/repos/<owner>/<repo>/releases/latest", or empty
  // when not configured. The caller performs the HTTPS fetch (respecting the
  // shell's offline mode) and hands the parsed ReleaseInfo to evaluateUpdate().
  [[nodiscard]] std::string latestReleaseApiUrl(const UpdateConfig& config);

  struct UpdateCheckResult {
    UpdateCheckState state = UpdateCheckState::NotConfigured;
    std::string currentVersion;
    std::string latestVersion; // release tag, when known
    std::string releaseNotes;
    std::string releaseUrl;
    std::string error; // human-readable detail when state == CheckFailed
  };

  // Pure decision step (no I/O). `fetched` is the release info obtained from
  // latestReleaseApiUrl(), or nullopt when the fetch did not produce data.
  // `fetchError` carries the fetch failure reason, if any.
  [[nodiscard]] UpdateCheckResult evaluateUpdate(
      const UpdateConfig& config, std::string_view currentVersion, const std::optional<ReleaseInfo>& fetched,
      std::string fetchError = {}
  );

  // Short human-readable status line for the About page.
  [[nodiscard]] std::string describeState(const UpdateCheckResult& result);

} // namespace caeliglow
