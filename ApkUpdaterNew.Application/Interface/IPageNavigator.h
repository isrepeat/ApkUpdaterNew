#pragma once
#include <string_view>
#include <memory>

namespace apkupdaternew::application::core {
    enum class NavigationTrigger;
}

namespace apkupdaternew::application::base {
    class NavigationStateBase;
}

namespace apkupdaternew::application::interface {
    class IPageNavigator {
    public:
        virtual ~IPageNavigator() = default;

        virtual bool Navigate(std::string_view pageName) = 0;
        virtual bool Trigger(core::NavigationTrigger navigationTrigger) = 0;
        virtual bool Trigger(
            core::NavigationTrigger navigationTrigger,
            std::unique_ptr<base::NavigationStateBase> navigationState) = 0;
        // result отделяет результат действия от самого возврата. Например, диалог выбора
        // передаёт выбранное значение предыдущей странице, а кнопка «Назад» не передаёт ничего.
        virtual bool NavigateBack(std::unique_ptr<base::NavigationStateBase> navigationResult = {}) = 0;

        template <typename TPage>
        bool Navigate() {
            return this->Navigate(TPage::PageName);
        }
    };
}