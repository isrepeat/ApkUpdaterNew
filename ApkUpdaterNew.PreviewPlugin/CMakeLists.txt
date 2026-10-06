file(GLOB_RECURSE apkupdaternew_preview_plugin_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${apkupdaternew_preview_plugin_sources})
set(apkupdaternew_preview_generated_root
    "${CMAKE_CURRENT_SOURCE_DIR}/../!Generated/ApkUpdaterNew.Application/Xaml")
file(GLOB_RECURSE apkupdaternew_preview_generated_headers CONFIGURE_DEPENDS
    "${apkupdaternew_preview_generated_root}/*.h")
if (NOT apkupdaternew_preview_generated_headers)
    message(FATAL_ERROR "XamlCompiler did not generate headers for ApkUpdaterNew.PreviewPlugin.")
endif()
add_library(apkupdaternew_preview_plugin SHARED ${apkupdaternew_preview_plugin_sources})
set_target_properties(apkupdaternew_preview_plugin PROPERTIES
    OUTPUT_NAME ApkUpdaterNew.PreviewPlugin
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/ApkUpdaterNew.PreviewPlugin/Build/$<CONFIG>/x64/ApkUpdaterNew.PreviewPlugin")
android_app_previewer_add_runtime(apkupdaternew_preview_plugin)

target_include_directories(apkupdaternew_preview_plugin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(apkupdaternew_preview_plugin PRIVATE
    AndroidAppPreviewerPlugin::SDK
    apkupdaternew_application
    XamlRuntime::Logging
    XamlRuntime::Runtime
    XamlRuntime::OpenGLESRenderer)
if (MSVC)
    target_compile_options(apkupdaternew_preview_plugin PRIVATE /utf-8)
endif()
target_compile_definitions(apkupdaternew_preview_plugin PRIVATE
    APKUPDATERNEW_PREVIEW_SOURCE_MARKUP_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../ApkUpdaterNew.Application/UI"
    APKUPDATERNEW_PREVIEW_SOURCE_ENTRY_MARKUP_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../ApkUpdaterNew.Application/UI/Page/MainPage.xaml"
    APKUPDATERNEW_PREVIEW_SOURCE_CONTROLS_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../ApkUpdaterNew.UI/Control")

# ANGLE ищет свои DLL рядом с загруженным плагином.
get_target_property(apkupdaternew_angle_runtime_library XamlRuntime::OpenGLESRenderer IMPORTED_LOCATION)
get_filename_component(apkupdaternew_angle_runtime_directory "${apkupdaternew_angle_runtime_library}" DIRECTORY)
set(apkupdaternew_angle_runtime_files libEGL.dll libGLESv2.dll z.dll)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(apkupdaternew_angle_runtime_files libEGL.dll libGLESv2.dll zd.dll)
endif()
foreach (apkupdaternew_angle_runtime_file IN LISTS apkupdaternew_angle_runtime_files)
    add_custom_command(TARGET apkupdaternew_preview_plugin POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${apkupdaternew_angle_runtime_directory}/${apkupdaternew_angle_runtime_file}"
            "$<TARGET_FILE_DIR:apkupdaternew_preview_plugin>/${apkupdaternew_angle_runtime_file}")
endforeach()

set(apkupdaternew_preview_resources "${CMAKE_CURRENT_SOURCE_DIR}/../ApkUpdaterNew.Application/Resources")
if (EXISTS "${apkupdaternew_preview_resources}")
    file(GLOB_RECURSE apkupdaternew_preview_resource_files CONFIGURE_DEPENDS "${apkupdaternew_preview_resources}/*")
    source_group(TREE "${apkupdaternew_preview_resources}" PREFIX "Resources" FILES ${apkupdaternew_preview_resource_files})
    target_sources(apkupdaternew_preview_plugin PRIVATE ${apkupdaternew_preview_resource_files})
    add_custom_command(TARGET apkupdaternew_preview_plugin POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${apkupdaternew_preview_resources}" "$<TARGET_FILE_DIR:apkupdaternew_preview_plugin>/Resources")
else()
    add_custom_command(TARGET apkupdaternew_preview_plugin POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:apkupdaternew_preview_plugin>/Resources")
endif()