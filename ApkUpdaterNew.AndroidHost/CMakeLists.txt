add_library(apkupdaternew SHARED
    Native.cpp
    AndroidCommandDispatcher.cpp
    AndroidCommandDispatcher.h
    Jni/Jni.cpp
    Jni/Jni.h)
target_compile_features(apkupdaternew PRIVATE cxx_std_20)
target_include_directories(apkupdaternew PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(apkupdaternew PRIVATE
    apkupdaternew_application
    XamlRuntime::Runtime)
if (ANDROID)
    # В CMake 4.3 Android-модуль не добавляет c++abi для standalone-NDK preset.
    # Линкуем рантайм статически, не требуя отдельный libc++_shared.so.
    target_link_libraries(apkupdaternew PRIVATE XamlRuntime::OpenGLESRenderer android GLESv3 c++_static c++abi)
    set_target_properties(apkupdaternew PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/ApkUpdaterNew.AndroidHost/android/jniLibs/${ANDROID_ABI}")
endif()