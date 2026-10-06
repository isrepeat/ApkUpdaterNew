#include <AndroidAppPreviewer.PluginSDK/Session/PluginDefinition.h>

#include "./Session/PreviewSession.h"

#include <memory>
#include <format>

namespace preview_sdk {
    std::unique_ptr<session::PreviewSessionBase> CreatePreviewSession(int width, int height) {
        return std::make_unique<apkupdaternew::preview::session::PreviewSession>(width, height);
    }

    std::string PluginInfoJson() {
        return std::format(
            R"({{"applicationId":"ApkUpdaterNew","displayName":"ApkUpdaterNew",)"
            R"("resourceRootRelativePath":"Resources","sourceMarkupDirectory":"{}",)"
            R"("sourceEntryMarkupPath":"{}","sourceControlsDirectory":"{}"}})",
            APKUPDATERNEW_PREVIEW_SOURCE_MARKUP_DIRECTORY,
            APKUPDATERNEW_PREVIEW_SOURCE_ENTRY_MARKUP_PATH,
            APKUPDATERNEW_PREVIEW_SOURCE_CONTROLS_DIRECTORY
        );
    }
}