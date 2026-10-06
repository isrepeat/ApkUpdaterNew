#pragma once
#include <AndroidBuildTools/ApplicationFramework/AppSessionControllerBase.h>

#include "../Interface/IHostCommandDispatcher.h"

namespace apkupdaternew::application::core {
    class AppSessionController final : public application_framework::AppSessionControllerBase<
        interface::IHostCommandDispatcher, HostCommand, HostCommandData> {
    protected:
        void OnUnhandledHostCommand(HostCommand hostCommand, const HostCommandData& hostCommandData) override;
    };
}