#include "AppSessionController.h"

namespace apkupdaternew::application::core {
    //
    // AppSessionControllerBase
    //
    void AppSessionController::OnUnhandledHostCommand(HostCommand, const HostCommandData&) {
        this->SetStatus("Application updates are available in the Android host.");
    }
}