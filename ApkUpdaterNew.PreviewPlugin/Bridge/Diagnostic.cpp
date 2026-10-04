#include "Diagnostic.h"

namespace apkupdaternew::preview::bridge {
    std::string& LastError() {
        static thread_local std::string lastError;
        return lastError;
    }
}