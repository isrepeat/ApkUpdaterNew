#include "preview_PackageInstaller.h"

#include <algorithm>
#include <stdexcept>
#include <array>
#include <cmath>

namespace apkupdaternew::preview::session {
    preview_PackageInstaller::preview_PackageInstaller(application::core::UpdateController& updateController)
        : updateController(updateController) {
        this->updateController.Attach(*this);
        this->Configure("success");
    }

    //
    // IPackageInstaller
    //
    void preview_PackageInstaller::Prepare() {
        this->work = Work::prepare;
        this->elapsed = std::chrono::milliseconds::zero();
        this->previous = std::chrono::steady_clock::now();
    }

    void preview_PackageInstaller::Commit() {
        this->work = Work::none;
        this->updateController.AwaitSystemConfirmation();
    }

    void preview_PackageInstaller::ConfirmSystem(bool accepted) {
        if (!accepted) {
            this->Cancel();
            this->updateController.Closed();
            return;
        }
        this->updateController.InstallationStarted();
        this->work = Work::install;
        this->elapsed = std::chrono::milliseconds::zero();
        this->previous = std::chrono::steady_clock::now();
    }

    void preview_PackageInstaller::Launch() {
        this->work = Work::launch;
        this->elapsed = std::chrono::milliseconds::zero();
    }

    void preview_PackageInstaller::Cancel() {
        this->work = Work::none;
        this->elapsed = std::chrono::milliseconds::zero();
    }

    //
    // API
    //
    void preview_PackageInstaller::Configure(std::string_view value) {
        if (value != "success" && value != "reinstall" && value != "invalid-apk" && value != "install-error"
            && value != "launch-error" && value != "cancel") {
            throw std::invalid_argument("Unknown update preview scenario");
        }
        this->scenario = value;
        this->updateController.Reset(true, "DocumentTranslator", "1.12.0", value == "reinstall" ? "1.12.0" : "1.13.0");
    }

    void preview_PackageInstaller::SetRate(float value) {
        if (!std::isfinite(value) || value <= 0) {
            throw std::invalid_argument("Invalid preview playback rate");
        }
        this->rate = value;
    }

    void preview_PackageInstaller::Tick() {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>((now - this->previous) * this->rate);
        this->previous = now;
        this->Advance(elapsed);
    }

    void preview_PackageInstaller::Advance(std::chrono::milliseconds elapsed) {
        if (elapsed < std::chrono::milliseconds::zero()) {
            throw std::invalid_argument("Invalid preview elapsed time");
        }
        this->elapsed += elapsed;
        if (this->work == Work::prepare && this->elapsed >= std::chrono::milliseconds{700}) {
            this->work = Work::none;
            if (this->scenario == "invalid-apk") {
                this->updateController.Failed("Подпись APK не соответствует установленному приложению.");
            } else {
                this->updateController.Prepared(this->scenario == "reinstall" || this->scenario == "cancel");
            }
        } else if (this->work == Work::install) {
            if (this->scenario == "install-error" && this->elapsed >= std::chrono::milliseconds{2500}) {
                this->work = Work::none;
                this->updateController.Failed("Недостаточно места для установки. Освободите память и повторите обновление.");
                return;
            }
            // Прогресс имитирует скачки PackageInstaller; 100% ещё не означает успех.
            constexpr std::array progress{ -1, 4, 18, 18, 42, 65, 65, 87, 100, 100 };
            const auto index = std::min(static_cast<size_t>(this->elapsed / std::chrono::milliseconds{500}), progress.size() - 1);
            this->updateController.Progress(progress[index]);
            if (this->elapsed >= std::chrono::milliseconds{5200}) {
                this->work = Work::none;
                this->updateController.Installed();
            }
        } else if (this->work == Work::launch && this->elapsed >= std::chrono::milliseconds{1000}) {
            this->work = Work::none;
            if (this->scenario == "launch-error") {
                this->updateController.Failed("Обновление установлено, но приложение не удалось запустить.");
            } else {
                this->updateController.Closed(true);
            }
        }
    }
}