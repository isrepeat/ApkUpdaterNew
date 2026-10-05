#include "MainPageViewModel.h"

#if defined(ANDROID_APP_PREVIEWER)
#include <XamlRuntime/RuntimeMarkup/RuntimeBindingRegistry.h>
#endif
#include <XamlRuntime/Animation.h>

#include "../../../!Generated/ApkUpdaterNew.Application/Xaml/Page/MainPage.xaml.h"
#include "../../../!Generated/Build/BuildVersion.h"
#include "../../Core/NavigationStates.h"

#include <utility>

namespace apkupdaternew::application::ui::page {
    MainPageViewModel::MainPageViewModel(core::PageContext& pageContext)
        : context(pageContext)
        , packageVersion("Version " APKUPDATERNEW_PACKAGE_VERSION) {
    }

    //
    // INavigationPage
    //
    std::unique_ptr<base::NavigationStateBase> MainPageViewModel::OnNavigatingFrom(const core::NavigationRequest&) {
        return {};
    }

    bool MainPageViewModel::OnNavigatingTo(const core::NavigationRequest&, std::unique_ptr<base::NavigationStateBase> navigationState) {
        return navigationState == nullptr;
    }

    //
    // IPage
    //
    std::string_view MainPageViewModel::Name() const {
        return PageName;
    }

    std::string_view MainPageViewModel::Title() const {
        return "Main";
    }

    void MainPageViewModel::Initialize(xaml::Size availableSize) {
        this->bindings = std::make_unique<xaml::BindingScope>();
        this->root = xaml::generated::MainPage::Create(*this, *this->bindings);
        xaml::layoutInViewport(*this->root, availableSize);
    }

    void MainPageViewModel::Update() {
        const auto& state = this->context.controller.Updates().State();
        if (this->updateRevision != state.revision) {
            this->updateRevision = state.revision;
            this->UpdateProgress(*this->root);
            for (int value = static_cast<int>(Property::heading); value <= static_cast<int>(Property::showAndroidTools); ++value) {
                for (const auto& [id, handler] : this->handlers) {
                    handler(static_cast<Property>(value));
                }
            }
        }
        const std::string desired = state.phase == core::UpdatePhase::closed ? "Closed" : state.opaque ? "Opaque" : "Transparent";
        if (desired != this->backgroundState) {
            xaml::VisualStateManager::GoToState(*this->root, "UpdateBackground", desired, !this->backgroundState.empty());
            this->backgroundState = desired;
        }
        if (this->status != this->context.controller.Status()) {
            this->status = this->context.controller.Status();
            for (const auto& [id, handler] : this->handlers) {
                handler(Property::status);
            }
        }
    }

    xaml::Element& MainPageViewModel::Root() {
        return *this->root;
    }
#if defined(ANDROID_APP_PREVIEWER)
    xaml::runtime::RuntimeBindingContext MainPageViewModel::preview_RuntimeContext() {
        xaml::runtime::RuntimeBindingContext result;
        result.xamlNamespace = "urn:apkupdaternew:xaml";
        result.owner = PageName;
        result.bindings = std::make_shared<xaml::runtime::RuntimeBindingRegistry>();
        result.bindings->AddCommand("NavigateToSettingsCommand", this->NavigateToSettingsCommand());
        result.bindings->AddText("PackageVersion", [this] {
            return this->PackageVersion();
        });
        result.bindings->AddCommand("RequestApplicationUpdateCommand", this->RequestApplicationUpdateCommand());
        result.bindings->AddCommand("SendLogsCommand", this->SendLogsCommand());
        result.bindings->AddCommand("StartUpdateCommand", this->StartUpdateCommand());
        result.bindings->AddCommand("AcceptUpdateCommand", this->AcceptUpdateCommand());
        result.bindings->AddCommand("CancelUpdateCommand", this->CancelUpdateCommand());
        const auto subscribe = [this](std::function<void()> handler) {
            return this->Subscribe([handler](Property) {
                handler();
            });
        };
        result.bindings->AddText("Heading", [this] {
            return this->Heading();
        }, subscribe);
        result.bindings->AddText("Detail", [this] {
            return this->Detail();
        }, subscribe);
        result.bindings->AddText("Versions", [this] {
            return this->Versions();
        }, subscribe);
        result.bindings->AddText("ProgressText", [this] {
            return this->ProgressText();
        }, subscribe);
        result.bindings->AddText("AcceptText", [this] {
            return this->AcceptText();
        }, subscribe);
        result.bindings->AddText("CancelText", [this] {
            return this->CancelText();
        }, subscribe);
        result.bindings->AddBoolean("ShowLauncher", [this] {
            return this->ShowLauncher();
        }, subscribe);
        result.bindings->AddBoolean("ShowCard", [this] {
            return this->ShowCard();
        }, subscribe);
        result.bindings->AddBoolean("ShowAccept", [this] {
            return this->ShowAccept();
        }, subscribe);
        result.bindings->AddBoolean("ShowAndroidTools", [this] {
            return this->ShowAndroidTools();
        }, subscribe);
        result.bindings->AddBoolean("ShowCancel", [this] {
            return this->ShowCancel();
        }, subscribe);
        result.bindings->AddBoolean("ShowProgress", [this] {
            return this->ShowProgress();
        }, subscribe);
        result.bindings->AddText("Status", [this] {
            return this->Status();
        }, [this](std::function<void()> handler) {
            return this->Subscribe([handler](Property) {
                handler();
            });
        });
        return result;
    }

    void MainPageViewModel::preview_ReplaceRuntimeTree(xaml::runtime::RuntimeBuildResult runtimeBuildResult) {
        this->bindings = std::move(runtimeBuildResult.bindings);
        this->root = std::move(runtimeBuildResult.root);
        this->backgroundState.clear();
        this->updateRevision = -1;
    }
#endif
    //
    // API
    //
    xaml::Element::Command MainPageViewModel::NavigateToSettingsCommand() {
        return [this] {
            auto state = std::make_unique<core::GreetingNavigationState>();
            state->Message = this->context.repository.Greeting();
            this->context.navigator.Trigger(core::NavigationTrigger::navigateToSettings, std::move(state));
        };
    }

    xaml::Element::Command MainPageViewModel::RequestApplicationUpdateCommand() {
        return [this] {
            this->context.hostCommands.Dispatch(core::HostCommand::requestApplicationUpdate);
            this->Update();
        };
    }

    xaml::Element::Command MainPageViewModel::SendLogsCommand() {
        return [this] {
            this->context.hostCommands.Dispatch(core::HostCommand::sendLogs);
            this->Update();
        };
    }

    const std::string& MainPageViewModel::Status() const {
        return this->status;
    }

    const std::string& MainPageViewModel::PackageVersion() const {
        return this->packageVersion;
    }

    std::string MainPageViewModel::Heading() const {
        switch (this->context.controller.Updates().State().phase) {
        case core::UpdatePhase::preparing:
            return "Проверка обновления";
        case core::UpdatePhase::confirmation:
            return this->context.controller.Updates().State().installedVersion == this->context.controller.Updates().State().availableVersion
                ? "Переустановить приложение?" : "Обновить приложение?";
        case core::UpdatePhase::covering:
            return "Подготовка установки";
        case core::UpdatePhase::systemConfirmation:
            return "Установить обновление?";
        case core::UpdatePhase::installing:
            return "Установка обновления";
        case core::UpdatePhase::launching:
            return "Запуск новой версии…";
        case core::UpdatePhase::failed:
            return "Не удалось завершить обновление";
        case core::UpdatePhase::closed:
            return "Updater закрыт";
        default:
            return "Симуляция обновления";
        }
    }

    std::string MainPageViewModel::Detail() const {
        const auto& state = this->context.controller.Updates().State();
        switch (state.phase) {
        case core::UpdatePhase::preparing:
            return "Проверяем пакет и подпись приложения…";
        case core::UpdatePhase::confirmation:
            return "Подтвердите установку выбранной версии. Данные приложения сохранятся.";
        case core::UpdatePhase::covering:
            return "Подготавливаем экран установки…";
        case core::UpdatePhase::systemConfirmation:
            return "Подтверждение Android · макет для preview";
        case core::UpdatePhase::installing:
            return state.progress < 0 ? "Ожидаем данные установщика…" : "Пожалуйста, подождите";
        case core::UpdatePhase::launching:
            return "Обновление установлено. Возвращаемся в приложение.";
        case core::UpdatePhase::failed:
            return state.error;
        case core::UpdatePhase::closed:
            return state.launched ? "Новая версия приложения запущена"
            : state.installed ? "Обновление установлено. Запуск не выполнен." : "Обновление отменено";
        default:
            return "Тестовый запрос с готовым APK. Сценарий выбирается в панели previewer.";
        }
    }

    std::string MainPageViewModel::Versions() const {
        const auto& state = this->context.controller.Updates().State();
        return state.application + "   " + state.installedVersion + "  >  " + state.availableVersion;
    }

    std::string MainPageViewModel::ProgressText() const {
        const auto progress = this->context.controller.Updates().State().progress;
        return progress < 0 ? "Подготовка…" : std::to_string(progress) + " %";
    }

    std::string MainPageViewModel::AcceptText() const {
        return this->context.controller.Updates().State().phase == core::UpdatePhase::confirmation ? "Продолжить" : "Установить";
    }

    std::string MainPageViewModel::CancelText() const {
        return this->context.controller.Updates().State().phase == core::UpdatePhase::failed ? "Закрыть" : "Отмена";
    }

    bool MainPageViewModel::ShowLauncher() const {
        const auto& state = this->context.controller.Updates().State();
        return state.preview_isEnabled && (state.phase == core::UpdatePhase::idle || state.phase == core::UpdatePhase::closed);
    }

    bool MainPageViewModel::ShowAndroidTools() const {
        return !this->context.controller.Updates().State().preview_isEnabled;
    }

    bool MainPageViewModel::ShowCard() const {
        const auto phase = this->context.controller.Updates().State().phase;
        return phase != core::UpdatePhase::idle && phase != core::UpdatePhase::closed;
    }

    bool MainPageViewModel::ShowAccept() const {
        const auto phase = this->context.controller.Updates().State().phase;
        return phase == core::UpdatePhase::confirmation || phase == core::UpdatePhase::systemConfirmation;
    }

    bool MainPageViewModel::ShowCancel() const {
        const auto phase = this->context.controller.Updates().State().phase;
        return this->ShowAccept() || phase == core::UpdatePhase::preparing || phase == core::UpdatePhase::failed;
    }

    bool MainPageViewModel::ShowProgress() const {
        return this->context.controller.Updates().State().phase == core::UpdatePhase::installing;
    }

    xaml::Element::Command MainPageViewModel::StartUpdateCommand() {
        return [this] {
            this->context.controller.Updates().Start();
            this->Update();
        };
    }

    xaml::Element::Command MainPageViewModel::AcceptUpdateCommand() {
        return [this] {
            this->context.controller.Updates().Accept();
            this->Update();
        };
    }

    xaml::Element::Command MainPageViewModel::CancelUpdateCommand() {
        return [this] {
            this->context.controller.Updates().Cancel();
            this->Update();
        };
    }

    std::function<void()> MainPageViewModel::Subscribe(PropertyChangedHandler propertyChangedHandler) {
        const size_t id = ++this->nextSubscription;
        this->handlers.emplace(id, std::move(propertyChangedHandler));
        return [this, id] {
            this->handlers.erase(id);
        };
    }

    //
    // Internal
    //
    void MainPageViewModel::UpdateProgress(xaml::Element& xamlElement) {
        if (xamlElement.Id() == "updateProgressTrack") {
            const auto progress = this->context.controller.Updates().State().progress;
            int threshold = 10;
            for (const auto& segment : xamlElement.Children()) {
                segment->SetVisibility(progress >= threshold ? xaml::attr::Visibility::visible : xaml::attr::Visibility::collapsed);
                threshold += 10;
            }
            return;
        }
        for (const auto& child : xamlElement.Children()) {
            this->UpdateProgress(*child);
        }
    }
}