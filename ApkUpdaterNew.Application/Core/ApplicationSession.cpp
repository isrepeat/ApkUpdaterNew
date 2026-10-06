#include "ApplicationSession.h"

#include <utility>

namespace apkupdaternew::application::core {
    ApplicationSession::ApplicationSession(
        model::ApplicationStateDocument applicationStateDocument,
        ApplicationStateStore::DocumentSaveHandler documentSaveHandler
    )
        : ApplicationSessionBase(std::move(applicationStateDocument), std::move(documentSaveHandler)) {
    }
}