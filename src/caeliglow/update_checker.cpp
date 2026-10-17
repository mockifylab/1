#include "caeliglow/update_checker.h"

#include <charconv>
#include <cctype>

namespace caeliglow {
  namespace {

    bool isDigits(std::string_view s) {
      if (s.empty()) {
        return false;
      }
      for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
          return false;
        }
      }
      return true;
    }

    // Parses a numeric component; rejects leading zeros per semver ("01" invalid).
    bool parseNumber(std::string_view s, int& out) {
      if (!isDigits(s) || (s.size() > 1 && s.front() == '0')) {
        return false;
      }
      int value = 0;
      const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
      if (ec != std::errc() || ptr != s.data() + s.size()) {
        return false;
      }
      out = value;
      return true;
    }

    // Splits "1.2.3-rc.1+build" into core "1.2.3", prerelease "rc.1". Build metadata dropped.
    bool splitCore(std::string_view text, std::string_view& core, std::string_view& prerelease) {
      if (text.empty()) {
        return false;
      }
      if (text.front() == 'v' || text.front() == 'V') {
        text.remove_prefix(1);
      }
      const std::size_t plus = text.find('+');
      if (plus != std::string_view::npos) {
        text = text.substr(0, plus);
      }
      const std::size_t dash = text.find('-');
      if (dash != std::string_view::npos) {
        core = text.substr(0, dash);
        prerelease = text.substr(dash + 1);
        if (prerelease.empty()) {
          return false;
        }
      } else {
        core = text;
        prerelease = {};
      }
      return true;
    }

    // Prerelease precedence per semver §11: numeric identifiers compare
    // numerically, alphanumeric lexically, numeric < alphanumeric, and a
    // shorter set is lower precedence when all preceding identifiers match.
    int comparePrerelease(std::string_view a, std::string_view b) {
      if (a == b) {
        return 0;
      }
      if (a.empty()) {
        return 1; // release > prerelease
      }
      if (b.empty()) {
        return -1;
      }
      std::size_t ia = 0, ib = 0;
      while (ia < a.size() && ib < b.size()) {
        const std::size_t ea = a.find('.', ia);
        const std::size_t eb = b.find('.', ib);
        const std::string_view fa = a.substr(ia, ea == std::string_view::npos ? ea : ea - ia);
        const std::string_view fb = b.substr(ib, eb == std::string_view::npos ? eb : eb - ib);
        const bool na = isDigits(fa);
        const bool nb = isDigits(fb);
        if (na && nb) {
          int va = 0, vb = 0;
          // Identifiers are validated at parse time; from_chars cannot fail here.
          std::from_chars(fa.data(), fa.data() + fa.size(), va);
          std::from_chars(fb.data(), fb.data() + fb.size(), vb);
          if (va != vb) {
            return va < vb ? -1 : 1;
          }
        } else if (na != nb) {
          return na ? -1 : 1; // numeric identifiers have lower precedence
        } else if (fa != fb) {
          return fa < fb ? -1 : 1;
        }
        ia = (ea == std::string_view::npos) ? a.size() : ea + 1;
        ib = (eb == std::string_view::npos) ? b.size() : eb + 1;
      }
      if (ia >= a.size() && ib >= b.size()) {
        return 0;
      }
      return (ia >= a.size()) ? -1 : 1;
    }

    bool validPrerelease(std::string_view pre) {
      if (pre.empty()) {
        return true;
      }
      std::size_t i = 0;
      while (i < pre.size()) {
        const std::size_t e = pre.find('.', i);
        const std::string_view id = pre.substr(i, e == std::string_view::npos ? e : e - i);
        if (id.empty()) {
          return false;
        }
        for (char c : id) {
          const auto uc = static_cast<unsigned char>(c);
          if (!std::isalnum(uc) && c != '-') {
            return false;
          }
        }
        if (isDigits(id) && id.size() > 1 && id.front() == '0') {
          return false;
        }
        i = (e == std::string_view::npos) ? pre.size() : e + 1;
      }
      return true;
    }

    std::string trimSlashes(std::string_view s) {
      while (!s.empty() && s.back() == '/') {
        s.remove_suffix(1);
      }
      return std::string(s);
    }

  } // namespace

  std::optional<SemVer> parseSemVer(std::string_view text) {
    std::string_view core, prerelease;
    if (!splitCore(text, core, prerelease) || !validPrerelease(prerelease)) {
      return std::nullopt;
    }
    SemVer v;
    std::size_t i = 0;
    int* parts[3] = {&v.major, &v.minor, &v.patch};
    for (int p = 0; p < 3; ++p) {
      const std::size_t e = core.find('.', i);
      const std::string_view part = core.substr(i, e == std::string_view::npos ? e : e - i);
      if (!parseNumber(part, *parts[p])) {
        return std::nullopt;
      }
      if (e == std::string_view::npos) {
        if (p != 2) {
          return std::nullopt;
        }
        i = core.size();
      } else {
        i = e + 1;
      }
    }
    if (i != core.size()) {
      return std::nullopt;
    }
    v.prerelease = std::string(prerelease);
    return v;
  }

  int compareSemVer(const SemVer& a, const SemVer& b) {
    if (a.major != b.major) {
      return a.major < b.major ? -1 : 1;
    }
    if (a.minor != b.minor) {
      return a.minor < b.minor ? -1 : 1;
    }
    if (a.patch != b.patch) {
      return a.patch < b.patch ? -1 : 1;
    }
    return comparePrerelease(a.prerelease, b.prerelease);
  }

  std::string normalizeRepository(const UpdateConfig& config) {
    std::string repo = trimSlashes(config.repository);
    if (repo.empty()) {
      return {};
    }
    constexpr std::string_view kPrefix = "https://github.com/";
    constexpr std::string_view kSshPrefix = "git@github.com:";
    if (repo.starts_with(kPrefix)) {
      repo = repo.substr(kPrefix.size());
    } else if (repo.starts_with(kSshPrefix)) {
      repo = repo.substr(kSshPrefix.size());
    }
    constexpr std::string_view kGitSuffix = ".git";
    if (repo.ends_with(kGitSuffix)) {
      repo = repo.substr(0, repo.size() - kGitSuffix.size());
    }
    repo = trimSlashes(repo);
    // Must be exactly "owner/repo" with no extra path segments.
    const std::size_t slash = repo.find('/');
    if (slash == std::string::npos || slash == 0 || slash + 1 >= repo.size()
        || repo.find('/', slash + 1) != std::string::npos) {
      return {};
    }
    return repo;
  }

  std::string latestReleaseApiUrl(const UpdateConfig& config) {
    const std::string repo = normalizeRepository(config);
    if (repo.empty()) {
      return {};
    }
    return "https://api.github.com/repos/" + repo + "/releases/latest";
  }

  UpdateCheckResult evaluateUpdate(
      const UpdateConfig& config, std::string_view currentVersion, const std::optional<ReleaseInfo>& fetched,
      std::string fetchError
  ) {
    UpdateCheckResult result;
    result.currentVersion = std::string(currentVersion);

    if (normalizeRepository(config).empty()) {
      result.state = UpdateCheckState::NotConfigured;
      return result;
    }

    if (!fetchError.empty() || !fetched.has_value()) {
      result.state = UpdateCheckState::CheckFailed;
      result.error = fetchError.empty() ? "No release information was returned." : std::move(fetchError);
      return result;
    }

    const auto current = parseSemVer(currentVersion);
    const auto latest = parseSemVer(fetched->tagName);
    if (!current.has_value() || !latest.has_value()) {
      result.state = UpdateCheckState::CheckFailed;
      result.error = "Could not parse version numbers.";
      return result;
    }

    result.latestVersion = fetched->tagName;
    result.releaseNotes = fetched->body;
    result.releaseUrl = fetched->htmlUrl.empty()
        ? ("https://github.com/" + normalizeRepository(config) + "/releases")
        : fetched->htmlUrl;

    result.state =
        (compareSemVer(*latest, *current) > 0) ? UpdateCheckState::UpdateAvailable : UpdateCheckState::UpToDate;
    return result;
  }

  std::string describeState(const UpdateCheckResult& result) {
    switch (result.state) {
    case UpdateCheckState::NotConfigured:
      return "Update checking isn't configured yet.";
    case UpdateCheckState::UpToDate:
      return "You're up to date.";
    case UpdateCheckState::UpdateAvailable:
      return "Update available: " + result.latestVersion + ".";
    case UpdateCheckState::CheckFailed:
      return result.error.empty() ? "Update check failed." : ("Update check failed: " + result.error);
    }
    return "Update checking isn't configured yet.";
  }

} // namespace caeliglow
