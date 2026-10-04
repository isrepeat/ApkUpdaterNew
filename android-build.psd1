@{
    AndroidBuildToolsVersion = '1.0.63.22'
    ArtifactName = 'ApkUpdaterNew'
    PackageId = 'com.isrepeat.apkupdaternew'
    AndroidModule = 'ApkUpdaterNew.Android'
    AndroidHost = 'ApkUpdaterNew.AndroidHost'
    Application = 'ApkUpdaterNew.Application'
    UI = 'ApkUpdaterNew.UI'
    NativeLibrary = 'libapkupdaternew.so'
    AndroidPresetPrefix = 'android-arm64'
    CMakeVersionVariable = 'APKUPDATERNEW_PACKAGE_VERSION'
    GradleRoot = 'Tools\Gradle'
    VersionFile = 'version.properties'
    DistributionDirectory = 'Build\distribution'
    PackageDirectories = @{
        AndroidBuildTools = 'Build\Packages\ApkUpdaterNew'
        XamlRuntime = 'Build\Packages\ApkUpdaterNew.AndroidHost'
        AndroidAppPreviewerPluginSdk = 'Build\Packages\ApkUpdaterNew.PreviewPlugin'
    }
    Xaml = @{
        Namespace = 'urn:apkupdaternew:xaml'
        ControlNamespace = 'apkupdaternew::ui::control'
        ControlIncludePrefix = 'ApkUpdaterNew.UI/Control'
    }
    Preview = @{
        ArtifactDirectory = 'Build\ApkUpdaterNew.PreviewPlugin'
        Plugin = 'Build\{Configuration}\x64\ApkUpdaterNew.PreviewPlugin\ApkUpdaterNew.PreviewPlugin.dll'
        Target = 'apkupdaternew_preview_plugin'
    }
    Drive = @{
        Path = @('Android', 'ApkUpdaterNew')
    }
}