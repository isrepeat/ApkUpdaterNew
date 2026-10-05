cmake_minimum_required(VERSION 3.22.1)
project(ApkUpdaterNew LANGUAGES C CXX)

# Desktop presets не проходят через AndroidToolchain.cmake, поэтому до
# подключения CMake-модулей восстанавливаем тот же закреплённый пакет tools.
if (NOT ANDROID_BUILD_TOOLS_ROOT)
    find_program(apkupdaternew_bootstrap_powershell NAMES powershell.exe REQUIRED)
    execute_process(
        COMMAND "${apkupdaternew_bootstrap_powershell}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_CURRENT_SOURCE_DIR}/build.ps1" -Command restore
        RESULT_VARIABLE apkupdaternew_bootstrap_result
        OUTPUT_VARIABLE apkupdaternew_bootstrap_output
        ERROR_VARIABLE apkupdaternew_bootstrap_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT apkupdaternew_bootstrap_result EQUAL 0)
        message(FATAL_ERROR "AndroidBuildTools restore failed: ${apkupdaternew_bootstrap_error}")
    endif()
    string(REGEX MATCH "[^\r\n]+$" apkupdaternew_bootstrap_root
        "${apkupdaternew_bootstrap_output}")
    file(TO_CMAKE_PATH "${apkupdaternew_bootstrap_root}" ANDROID_BUILD_TOOLS_ROOT)
endif()

include("${ANDROID_BUILD_TOOLS_ROOT}/tools/cmake/AndroidBuildConfiguration.cmake")

function(apkupdaternew_get_configuration_value configuration_name output_variable)
    find_program(apkupdaternew_powershell NAMES powershell.exe REQUIRED)
    execute_process(
        COMMAND "${apkupdaternew_powershell}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_CURRENT_SOURCE_DIR}/build.ps1"
            -Command get-configuration-value -Name "${configuration_name}"
        RESULT_VARIABLE apkupdaternew_configuration_result
        OUTPUT_VARIABLE apkupdaternew_configuration_output
        ERROR_VARIABLE apkupdaternew_configuration_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT apkupdaternew_configuration_result EQUAL 0)
        message(FATAL_ERROR "Could not read ${configuration_name}: ${apkupdaternew_configuration_error}")
    endif()
    string(REGEX MATCH "[^\r\n]+$" apkupdaternew_configuration_value
        "${apkupdaternew_configuration_output}")
    set(${output_variable} "${apkupdaternew_configuration_value}" PARENT_SCOPE)
endfunction()

fn_androidappkit_get_configuration_path("${CMAKE_CURRENT_SOURCE_DIR}" "PackageDirectories.XamlRuntime"
    apkupdaternew_xaml_runtime_packages_root)
set(apkupdaternew_native_package_source "${ANDROID_PACKAGES_FEED}")
if (NOT apkupdaternew_native_package_source)
    # Visual Studio запускает CMake напрямую, без аргументов сборочного скрипта.
    find_program(apkupdaternew_feed_powershell NAMES powershell.exe REQUIRED)
    execute_process(
        COMMAND "${apkupdaternew_feed_powershell}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_CURRENT_SOURCE_DIR}/build.ps1" -Command get-packages-feed
        RESULT_VARIABLE apkupdaternew_feed_result
        OUTPUT_VARIABLE apkupdaternew_feed_output
        ERROR_VARIABLE apkupdaternew_feed_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT apkupdaternew_feed_result EQUAL 0)
        message(FATAL_ERROR "Could not read PackagesFeed: ${apkupdaternew_feed_error}")
    endif()
    string(REGEX MATCH "[^\r\n]+$" apkupdaternew_native_package_source
        "${apkupdaternew_feed_output}")
    if (NOT apkupdaternew_native_package_source)
        message(FATAL_ERROR "Paths.PackagesFeed must be defined in Android.SharedProps.json.")
    endif()
endif()
include("${ANDROID_BUILD_TOOLS_ROOT}/tools/cmake/InstallXamlRuntime.cmake")
fn_androidappkit_install_xaml_runtime("${apkupdaternew_xaml_runtime_packages_root}"
    "${apkupdaternew_native_package_source}")

add_subdirectory(ApkUpdaterNew.Application)
add_subdirectory(ApkUpdaterNew.AndroidHost)
if (NOT ANDROID)
    fn_androidappkit_get_configuration_path("${CMAKE_CURRENT_SOURCE_DIR}" "PackageDirectories.AndroidAppPreviewerPluginSdk"
        apkupdaternew_preview_plugin_sdk_packages_root)
    include("${ANDROID_BUILD_TOOLS_ROOT}/tools/cmake/InstallAndroidAppPreviewerPluginSdk.cmake")
    fn_androidappkit_install_android_app_previewer_plugin_sdk(
        "${apkupdaternew_preview_plugin_sdk_packages_root}"
        "${apkupdaternew_native_package_source}")
    add_subdirectory(ApkUpdaterNew.PreviewPlugin)
endif()