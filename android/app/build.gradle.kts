plugins { id("com.android.application") }

android {
    buildFeatures { buildConfig = true }
    namespace = "io.github.lwy988998.gangyiai"
    compileSdk = 36
    ndkVersion = "28.2.13676358"
    defaultConfig {
        applicationId = "io.github.lwy988998.gangyiai"
        minSdk = 29
        targetSdk = 36
        versionCode = 1
        versionName = "1.0.0-preview.1"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        ndk { abiFilters += (providers.gradleProperty("gangyiAbi").orNull ?: "arm64-v8a") }
        externalNativeBuild { cmake { arguments += listOf("-DBUILD_TESTING=OFF", "-DANDROID_STL=c++_shared", "-DGANGYI_ANDROID_DEPS=" + (System.getenv("GANGYI_ANDROID_DEPS") ?: file("../.native-cache").absolutePath).replace('\\', '/')) } }
    }
    signingConfigs {
        if (System.getenv("GANGYI_ANDROID_KEYSTORE") != null) {
            create("stable") {
                storeFile = file(System.getenv("GANGYI_ANDROID_KEYSTORE"))
                storePassword = System.getenv("GANGYI_ANDROID_STORE_PASSWORD")
                keyAlias = "gangyi-android"
                keyPassword = System.getenv("GANGYI_ANDROID_KEY_PASSWORD")
            }
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = false
            if (signingConfigs.findByName("stable") != null) signingConfig = signingConfigs.getByName("stable")
        }
    }
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    externalNativeBuild { cmake {
        path = file("../CMakeLists.txt"); version = "3.22.1"
        buildStagingDirectory = file(System.getenv("GANGYI_ANDROID_BUILD") ?: ".cxx")
    } }
    sourceSets.getByName("main").assets.srcDirs("../../public", (System.getenv("GANGYI_ANDROID_DEPS") ?: file("../.native-cache").absolutePath) + "/assets")
    packaging { jniLibs.useLegacyPackaging = false }
}

dependencies {
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test:runner:1.6.2")
    androidTestImplementation("androidx.test.ext:junit:1.2.1")
}
System.getenv("GANGYI_ANDROID_OUTPUT")?.let { layout.buildDirectory.set(file(it)) }
