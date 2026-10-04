#pragma once
#include "../Base/NavigationStateBase.h"

#include <memory>

namespace apkupdaternew::application::core {
    struct NavigationRequest;
}

namespace apkupdaternew::application::interface {
    class INavigationPage {
    public:
        virtual ~INavigationPage() = default;
        virtual std::unique_ptr<base::NavigationStateBase> OnNavigatingFrom(const core::NavigationRequest& request) = 0;
        virtual bool OnNavigatingTo(const core::NavigationRequest& request, std::unique_ptr<base::NavigationStateBase> state) = 0;
    };
}