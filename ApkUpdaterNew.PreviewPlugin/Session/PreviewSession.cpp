#include "PreviewSession.h"

#include <stdexcept>
#include <cctype>
#include <regex>

namespace apkupdaternew::preview::session {
    PreviewSession::PreviewSession(int width, int height)
        : packageInstaller(this->applicationSession.Controller().Updates())
        , viewport{static_cast<float>(width), static_cast<float>(height)}
        , navigation(*this) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Preview session dimensions must be positive");
        }
        this->applicationSession.Initialize({static_cast<float>(width), static_cast<float>(height)});
    }

    //
    // API
    //
    xaml::Element& PreviewSession::Root() {
        return this->applicationSession.Root();
    }

    PreviewNavigationController& PreviewSession::Navigation() {
        return this->navigation;
    }

    const apkupdaternew::application::core::PageManager& PreviewSession::Pages() const {
        return this->applicationSession.Pages();
    }

    bool PreviewSession::Navigate(std::span<const std::string_view> ids, std::string& error) {
        return this->applicationSession.Pages().preview_NavigateTransitions(ids, error);
    }

    std::string_view PreviewSession::CurrentPage() const {
        return this->Pages().CurrentPageName();
    }

    std::string_view PreviewSession::PageTitle(std::string_view page) const {
        return this->Pages().PageTitle(page);
    }

    bool PreviewSession::LoadPage(std::string_view page) {
        return this->applicationSession.Pages().Navigate(page);
    }

    void PreviewSession::Resize(int width, int height) {
        this->viewport = {static_cast<float>(width), static_cast<float>(height)};
        this->applicationSession.Resize({static_cast<float>(width), static_cast<float>(height)});
    }

    bool PreviewSession::ReloadMarkup(std::string_view page, std::string_view markup, std::string_view sourcePath, std::string& error) {
        return this->applicationSession.Pages().preview_ReloadMarkup(page, markup, sourcePath, error);
    }

    void PreviewSession::SetAnimationPlaybackRate(float value) {
        this->applicationSession.Pages().SetAnimationPlaybackRate(value);
        this->packageInstaller.SetRate(value);
    }

    void PreviewSession::ApplyScenario(std::string_view json) {
        // Небольшой строгий контракт сценария: ровно один строковый идентификатор.
        static const std::regex pattern(R"re(^\s*\{\s*"scenario"\s*:\s*"([a-z-]+)"\s*\}\s*$)re");
        const std::string text(json);
        std::smatch match;
        if (!std::regex_match(text, match, pattern)) {
            throw std::invalid_argument("Expected {\"scenario\":\"success\"}");
        }
        this->packageInstaller.Configure(match[1].str());
        this->applicationSession.Update();
    }

    void PreviewSession::PointerDown(float x, float y) {
        this->applicationSession.PointerDown(x, y);
    }

    void PreviewSession::PointerMove(float x, float y) {
        this->applicationSession.PointerMove(x, y);
    }

    void PreviewSession::PointerUp(float x, float y) {
        this->applicationSession.PointerUp(x, y);
    }

    void PreviewSession::CancelPointer() {
        this->applicationSession.CancelPointer();
    }

    bool PreviewSession::Update() {
        this->packageInstaller.Tick();
        return this->applicationSession.Update();
    }

    void PreviewSession::Render(xaml::IRenderBackend& renderer) {
        // Подложка принадлежит только preview: имитация приложения под updater.
        const auto& state = this->applicationSession.Controller().Updates().State();
        const bool applicationStopped = state.phase == application::core::UpdatePhase::installing
            || state.phase == application::core::UpdatePhase::launching || (state.installed && !state.launched);
        renderer.DrawRoundedRect({0, 0, this->viewport.width, this->viewport.height}, {0.91f, 0.94f, 0.98f, 1}, 0);
        if (!applicationStopped) {
            renderer.DrawRoundedRect({0, 0, this->viewport.width, 180}, {0.16f, 0.30f, 0.52f, 1}, 0);
            renderer.DrawText({40, 72, this->viewport.width - 80, 64}, "DocumentTranslator", {1, 1, 1, 1}, 36, "Bold", xaml::attr::Alignment::left);
            for (int i = 0; i < 4; ++i) {
                renderer.DrawRoundedRect({32, 220.f + i * 150, this->viewport.width - 64, 120}, {1, 1, 1, 1}, 16);
                renderer.DrawText({56, 252.f + i * 150, this->viewport.width - 112, 56}, "Документ " + std::to_string(i + 1),
                    {0.16f, 0.22f, 0.32f, 1}, 28, "Normal", xaml::attr::Alignment::left);
            }
        } else {
            renderer.DrawText({40, 72, this->viewport.width - 80, 64}, "Рабочий стол", {0.16f, 0.22f, 0.32f, 1}, 36, "Bold", xaml::attr::Alignment::left);
        }
        this->applicationSession.Render(renderer);
        if (this->CurrentPage() == "MainPage" && this->Root().Background().alpha >= 0.9999f && this->Root().Opacity() >= 1) {
            this->applicationSession.Controller().Updates().OpaqueFramePresented();
        }
    }

    std::vector<std::string> PreviewSession::ParseNavigationTransitionIds(std::string_view json) {
        const size_t property = json.find("\"transitionIds\"");
        if (property == std::string_view::npos) {
            throw std::invalid_argument("Navigation request does not contain transitionIds");
        }
        const size_t arrayStart = json.find('[', property);
        const size_t arrayEnd = arrayStart == std::string_view::npos ? std::string_view::npos : json.find(']', arrayStart);
        if (arrayStart == std::string_view::npos || arrayEnd == std::string_view::npos) {
            throw std::invalid_argument("Navigation request contains an invalid transitionIds array");
        }
        std::vector<std::string> result;
        size_t position = arrayStart + 1;
        while (position < arrayEnd) {
            while (position < arrayEnd && std::isspace(static_cast<unsigned char>(json[position]))) {
                ++position;
            }
            if (position == arrayEnd) {
                break;
            }
            if (json[position] != '\"') {
                throw std::invalid_argument("Navigation transition ID must be a JSON string");
            }
            const size_t valueStart = ++position;
            const size_t valueEnd = json.find('\"', valueStart);
            if (valueEnd == std::string_view::npos || valueEnd > arrayEnd) {
                throw std::invalid_argument("Navigation transition ID is not terminated");
            }
            if (json.substr(valueStart, valueEnd - valueStart).find('\\') != std::string_view::npos) {
                throw std::invalid_argument("Navigation transition ID must not contain JSON escapes");
            }
            result.emplace_back(json.substr(valueStart, valueEnd - valueStart));
            position = valueEnd + 1;
            while (position < arrayEnd && std::isspace(static_cast<unsigned char>(json[position]))) {
                ++position;
            }
            if (position < arrayEnd) {
                if (json[position] != ',') {
                    throw std::invalid_argument("Navigation transition IDs must be comma-separated");
                }
                ++position;
            }
        }
        if (result.empty()) {
            throw std::invalid_argument("Navigation request does not contain transition IDs");
        }
        return result;
    }
}