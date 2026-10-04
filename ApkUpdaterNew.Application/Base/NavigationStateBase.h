#pragma once
#include <string_view>

namespace apkupdaternew::application::base {
    class NavigationStateBase {
    public:
        virtual ~NavigationStateBase() = default;
        virtual std::string_view TypeId() const = 0;
    };
}