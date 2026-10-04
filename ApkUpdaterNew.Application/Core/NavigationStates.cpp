#include "NavigationStates.h"

namespace apkupdaternew::application::core {
#if defined(ANDROID_APP_PREVIEWER)
    //
    // API
    //
    std::unique_ptr<base::NavigationStateBase> GreetingNavigationState::preview_CreatePreviewDefault() {
        auto state = std::make_unique<GreetingNavigationState>();
        state->Message = "Hello from preview!";
        return state;
    }
#endif
}