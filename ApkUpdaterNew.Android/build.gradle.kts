plugins {
    id("com.isrepeat.android.application")
}

android {
    namespace = "com.isrepeat.apkupdaternew"
    defaultConfig.applicationId = "com.isrepeat.apkupdaternew"
    // Отображаемое имя каждой сборки задаётся самим приложением.
    buildTypes {
        getByName("debug") {
            manifestPlaceholders["appTitle"] = "ApkUpdaterNew ${defaultConfig.versionName} (debug)"
        }
        getByName("release") {
            manifestPlaceholders["appTitle"] = "ApkUpdaterNew"
        }
    }
    sourceSets.getByName("main").jniLibs.srcDir("../Build/ApkUpdaterNew.AndroidHost/android/jniLibs")
}

dependencies {
    implementation("com.isrepeat:androidappkit:1.0.17")
    implementation("com.google.android.gms:play-services-auth:21.3.0")
}