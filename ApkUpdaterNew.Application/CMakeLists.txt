set(apkupdaternew_project_root "${CMAKE_CURRENT_SOURCE_DIR}/..")
# Android-сборка передаёт фактическую версию; preview использует базовую версию проекта.
if (NOT DEFINED APKUPDATERNEW_PACKAGE_VERSION)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${apkupdaternew_project_root}/version.properties")
    file(STRINGS "${apkupdaternew_project_root}/version.properties" apkupdaternew_version_line REGEX "^VERSION_NAME_BASE=")
    string(REGEX REPLACE "^VERSION_NAME_BASE=" "" apkupdaternew_version_base "${apkupdaternew_version_line}")
    set(APKUPDATERNEW_PACKAGE_VERSION "${apkupdaternew_version_base}.0")
endif()
file(MAKE_DIRECTORY "${apkupdaternew_project_root}/!Generated/Build")
fn_androidappkit_write_if_changed("${apkupdaternew_project_root}/!Generated/Build/BuildVersion.h"
    "#pragma once\n#define APKUPDATERNEW_PACKAGE_VERSION \"${APKUPDATERNEW_PACKAGE_VERSION}\"")
set(apkupdaternew_generated_root "${apkupdaternew_project_root}/!Generated/ApkUpdaterNew.Application/Xaml")
fn_androidappkit_generate_xaml("${apkupdaternew_project_root}")
file(GLOB_RECURSE apkupdaternew_application_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Base/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Base/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Model/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Model/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/UI/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/UI/*.h")
file(GLOB_RECURSE apkupdaternew_generated_xaml CONFIGURE_DEPENDS "${apkupdaternew_generated_root}/*.xaml.cpp")
if (NOT apkupdaternew_generated_xaml)
    message(FATAL_ERROR "XamlCompiler did not generate application sources.")
endif()
# Генерируемые исходники и заголовки показываем отдельно от рукописного кода.
file(GLOB_RECURSE apkupdaternew_generated_headers CONFIGURE_DEPENDS "${apkupdaternew_generated_root}/*.xaml.h")
set(apkupdaternew_generated_files
    ${apkupdaternew_generated_xaml}
    ${apkupdaternew_generated_headers}
    "${apkupdaternew_project_root}/!Generated/Build/BuildVersion.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${apkupdaternew_application_sources})
source_group(TREE "${apkupdaternew_project_root}/!Generated" PREFIX "!Generated"
    FILES ${apkupdaternew_generated_files})
add_custom_target(apkupdaternew_generated SOURCES ${apkupdaternew_generated_files})
set_target_properties(apkupdaternew_generated PROPERTIES FOLDER "!Generated")
add_library(apkupdaternew_application STATIC ${apkupdaternew_application_sources} ${apkupdaternew_generated_files})
set_target_properties(apkupdaternew_application PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(apkupdaternew_application PUBLIC cxx_std_20)
target_include_directories(apkupdaternew_application PUBLIC "${apkupdaternew_project_root}")
target_link_libraries(apkupdaternew_application PUBLIC
    AndroidApplication::Framework
    XamlRuntime::Logging
    XamlRuntime::Runtime)
if (NOT ANDROID)
    target_compile_definitions(apkupdaternew_application PUBLIC ANDROID_APP_PREVIEWER)
endif()
if (MSVC)
    target_compile_options(apkupdaternew_application PRIVATE /utf-8)
endif()