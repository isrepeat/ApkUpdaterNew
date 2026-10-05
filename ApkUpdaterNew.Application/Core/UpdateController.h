#pragma once
#include "../Interface/IPackageInstaller.h"

#include <string>

namespace apkupdaternew::application::core {
    enum class UpdatePhase {
        idle,
        preparing,
        confirmation,
        covering,
        systemConfirmation,
        installing,
        launching,
        failed,
        closed,
    };

    struct UpdateState {
        UpdatePhase phase = UpdatePhase::idle;
        std::string application;
        std::string installedVersion;
        std::string availableVersion;
        std::string error;
        int progress = -1;
        bool opaque = false;
        bool installed = false;
        bool launched = false;
        bool preview_isEnabled = false;
        int revision = 0;
    };

    class UpdateController final {
    public:
        const UpdateState& State() const;
        void Attach(interface::IPackageInstaller& packageInstallerImpl);
        void Reset(bool preview_isEnabled, std::string application, std::string installedVersion, std::string availableVersion);
        void Start();
        void Prepared(bool requireConfirmation);
        void Accept();
        void Cancel();
        void OpaqueFramePresented();
        void AwaitSystemConfirmation();
        void InstallationStarted();
        void Progress(int value);
        void Installed();
        void Failed(std::string message);
        void Closed(bool launched = false);

    private:
        void SetPhase(UpdatePhase updatePhase);
        void AdvanceRevision();

    private:
        UpdateState updateState;
        interface::IPackageInstaller* packageInstallerImpl = nullptr;
    };
}