#pragma once
#include <XamlRuntime/Binding.h>

#include "../../Interface/IPage.h"
#include "../../Core/PageRegistry.h"

#include <map>

namespace apkupdaternew::application::ui::page {
    class MainPageViewModel final : public interface::IPage {
    public:
        static constexpr std::string_view PageName = "MainPage";
        enum class Property {
            status,
            packageVersion,
            heading,
            detail,
            versions,
            progressText,
            showLauncher,
            showCard,
            showAccept,
            showCancel,
            showProgress,
            acceptText,
            cancelText,
            showAndroidTools,
        };
        using PropertyChangedHandler = std::function<void(Property)>;

        explicit MainPageViewModel(core::PageContext& pageContext);
        ~MainPageViewModel() = default;

        //
        // INavigationPage
        //
        std::unique_ptr<base::NavigationStateBase> OnNavigatingFrom(const core::NavigationRequest& navigationRequest) override;
        bool OnNavigatingTo(
            const core::NavigationRequest& navigationRequest,
            std::unique_ptr<base::NavigationStateBase> navigationState) override;

        //
        // IPage
        //
        std::string_view Name() const override;
        std::string_view Title() const override;
        void Initialize(xaml::Size availableSize) override;
        void Update() override;
        xaml::Element& Root() override;
#if defined(ANDROID_APP_PREVIEWER)
        xaml::runtime::RuntimeBindingContext preview_RuntimeContext() override;
        void preview_ReplaceRuntimeTree(xaml::runtime::RuntimeBuildResult runtimeBuildResult) override;
#endif
        xaml::Element::Command NavigateToSettingsCommand();
        xaml::Element::Command RequestApplicationUpdateCommand();
        xaml::Element::Command SendLogsCommand();
        const std::string& Status() const;
        const std::string& PackageVersion() const;
        std::string Heading() const;
        std::string Detail() const;
        std::string Versions() const;
        std::string ProgressText() const;
        std::string AcceptText() const;
        std::string CancelText() const;
        bool ShowLauncher() const;
        bool ShowAndroidTools() const;
        bool ShowCard() const;
        bool ShowAccept() const;
        bool ShowCancel() const;
        bool ShowProgress() const;
        xaml::Element::Command StartUpdateCommand();
        xaml::Element::Command AcceptUpdateCommand();
        xaml::Element::Command CancelUpdateCommand();
        std::function<void()> Subscribe(PropertyChangedHandler propertyChangedHandler);

    private:
        void UpdateProgress(xaml::Element& xamlElement);

    private:
        core::PageContext& context;
        std::string status;
        std::string packageVersion;
        std::map<size_t, PropertyChangedHandler> handlers;
        size_t nextSubscription = 0;
        int updateRevision = -1;
        std::string backgroundState;
        std::unique_ptr<xaml::Element> root;
        std::unique_ptr<xaml::BindingScope> bindings;
    };
}