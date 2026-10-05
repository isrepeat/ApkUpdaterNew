#pragma once

namespace apkupdaternew::application::interface {
    class IPackageInstaller {
    public:
        virtual ~IPackageInstaller() = default;
        virtual void Prepare() = 0;
        virtual void Commit() = 0;
        virtual void ConfirmSystem(bool accepted) = 0;
        virtual void Launch() = 0;
        virtual void Cancel() = 0;
    };
}