#pragma once
#include "../../ApkUpdaterNew.Application/Core/UpdateController.h"

#include <string_view>
#include <chrono>

namespace apkupdaternew::preview::session {
    class preview_PackageInstaller final : public application::interface::IPackageInstaller {
    public:
        explicit preview_PackageInstaller(application::core::UpdateController& updateController);
        void Prepare() override;
        void Commit() override;
        void ConfirmSystem(bool accepted) override;
        void Launch() override;
        void Cancel() override;

        void Configure(std::string_view scenario);
        void Advance(std::chrono::milliseconds elapsed);
        void Tick();
        void SetRate(float value);

    private:
        enum class Work {
            none,
            prepare,
            install,
            launch,
        };
        application::core::UpdateController& updateController;
        std::string scenario = "success";
        Work work = Work::none;
        std::chrono::milliseconds elapsed{};
        float rate = 1;
        std::chrono::steady_clock::time_point previous = std::chrono::steady_clock::now();
    };
}