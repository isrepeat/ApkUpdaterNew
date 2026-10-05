#include "UpdateController.h"

#include <algorithm>
#include <utility>
#include <limits>

namespace apkupdaternew::application::core {
    //
    // API
    //
    const UpdateState& UpdateController::State() const {
        return this->updateState;
    }

    void UpdateController::Attach(interface::IPackageInstaller& packageInstallerImpl) {
        this->packageInstallerImpl = &packageInstallerImpl;
    }

    void UpdateController::Reset(bool preview_isEnabled, std::string application, std::string installedVersion, std::string availableVersion) {
        if (this->packageInstallerImpl) {
            this->packageInstallerImpl->Cancel();
        }
        this->AdvanceRevision();
        const auto revision = this->updateState.revision;
        this->updateState = {};
        this->updateState.preview_isEnabled = preview_isEnabled;
        this->updateState.revision = revision;
        this->updateState.application = std::move(application);
        this->updateState.installedVersion = std::move(installedVersion);
        this->updateState.availableVersion = std::move(availableVersion);
    }

    void UpdateController::Start() {
        if (!this->packageInstallerImpl || (this->updateState.phase != UpdatePhase::idle && this->updateState.phase != UpdatePhase::closed)) {
            return;
        }
        this->updateState.error.clear();
        this->updateState.progress = -1;
        this->updateState.opaque = false;
        this->updateState.installed = false;
        this->updateState.launched = false;
        this->SetPhase(UpdatePhase::preparing);
        this->packageInstallerImpl->Prepare();
    }

    void UpdateController::Prepared(bool requireConfirmation) {
        if (this->updateState.phase != UpdatePhase::preparing) {
            return;
        }
        this->SetPhase(UpdatePhase::confirmation);
        if (!requireConfirmation) {
            this->Accept();
        }
    }

    void UpdateController::Accept() {
        if (this->updateState.phase == UpdatePhase::confirmation) {
            this->updateState.opaque = true;
            this->SetPhase(UpdatePhase::covering);
        } else if (this->updateState.phase == UpdatePhase::systemConfirmation && this->packageInstallerImpl) {
            this->packageInstallerImpl->ConfirmSystem(true);
        }
    }

    void UpdateController::Cancel() {
        if (this->updateState.phase == UpdatePhase::systemConfirmation && this->packageInstallerImpl) {
            this->packageInstallerImpl->ConfirmSystem(false);
        } else if (this->updateState.phase == UpdatePhase::preparing || this->updateState.phase == UpdatePhase::confirmation
            || this->updateState.phase == UpdatePhase::failed) {
            if (this->packageInstallerImpl) {
                this->packageInstallerImpl->Cancel();
            }
            this->Closed();
        }
    }

    void UpdateController::OpaqueFramePresented() {
        if (this->updateState.phase != UpdatePhase::covering || !this->packageInstallerImpl) {
            return;
        }
        // Установка разрешена только после реально отрисованной непрозрачной подложки.
        this->SetPhase(UpdatePhase::installing);
        this->packageInstallerImpl->Commit();
    }

    void UpdateController::AwaitSystemConfirmation() {
        if (this->updateState.phase == UpdatePhase::installing) {
            this->SetPhase(UpdatePhase::systemConfirmation);
        }
    }

    void UpdateController::InstallationStarted() {
        if (this->updateState.phase == UpdatePhase::systemConfirmation) {
            this->SetPhase(UpdatePhase::installing);
        }
    }

    void UpdateController::Progress(int value) {
        if (this->updateState.phase != UpdatePhase::installing) {
            return;
        }
        value = std::clamp(value, -1, 100);
        if (value != this->updateState.progress) {
            this->updateState.progress = value;
            this->AdvanceRevision();
        }
    }

    void UpdateController::Installed() {
        if (this->updateState.phase != UpdatePhase::installing) {
            return;
        }
        this->updateState.installed = true;
        this->SetPhase(UpdatePhase::launching);
        if (this->packageInstallerImpl) {
            this->packageInstallerImpl->Launch();
        }
    }

    void UpdateController::Failed(std::string message) {
        this->updateState.error = std::move(message);
        this->SetPhase(UpdatePhase::failed);
    }

    void UpdateController::Closed(bool launched) {
        this->updateState.launched = launched;
        this->updateState.opaque = false;
        this->SetPhase(UpdatePhase::closed);
    }

    //
    // Internal
    //
    void UpdateController::SetPhase(UpdatePhase updatePhase) {
        this->updateState.phase = updatePhase;
        this->AdvanceRevision();
    }

    void UpdateController::AdvanceRevision() {
        this->updateState.revision = this->updateState.revision == std::numeric_limits<int>::max()
            ? 0 : this->updateState.revision + 1;
    }
}