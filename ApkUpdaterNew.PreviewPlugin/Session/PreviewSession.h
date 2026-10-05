#pragma once
#include "../../ApkUpdaterNew.Application/Core/ApplicationSession.h"
#include "./PreviewNavigationController.h"
#include "./preview_PackageInstaller.h"

namespace apkupdaternew::preview::session {
    class PreviewSession final {
    public:
        PreviewSession(int width, int height);
        xaml::Element& Root();
        PreviewNavigationController& Navigation();
        const apkupdaternew::application::core::PageManager& Pages() const;
        bool Navigate(std::span<const std::string_view> transitionIds, std::string& error);
        std::string_view CurrentPage() const;
        std::string_view PageTitle(std::string_view page) const;
        bool LoadPage(std::string_view page);
        void Resize(int width, int height);
        bool ReloadMarkup(std::string_view page, std::string_view markup, std::string_view sourcePath, std::string& error);
        void SetAnimationPlaybackRate(float value);
        void ApplyScenario(std::string_view json);
        void PointerDown(float x, float y);
        void PointerMove(float x, float y);
        void PointerUp(float x, float y);
        void CancelPointer();
        bool Update();
        void Render(xaml::IRenderBackend& renderer);
        static std::vector<std::string> ParseNavigationTransitionIds(std::string_view json);

    private:
        apkupdaternew::application::core::ApplicationSession applicationSession;
        preview_PackageInstaller packageInstaller;
        xaml::Size viewport;
        PreviewNavigationController navigation;
    };
}