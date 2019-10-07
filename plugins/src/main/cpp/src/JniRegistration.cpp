#include <jni.h>

#include "jniHelper.h"
#include "FaceTrackerPlugin.hpp"
#include "BarcodePlugin.h"
#include "MarkerTrackerPlugin.hpp"
#include "YUVFrameInputPlugin.h"
#include "SimpleInputPlugin.h"


extern "C" JNIEXPORT jlongArray JNICALL Java_com_wikitude_common_plugins_internal_PluginManagerInternal_createNativePlugins(JNIEnv *env, jobject thisObj, jstring jPluginName) {

    env->GetJavaVM(&JavaVMResource::JAVA_VM);

    int numberOfPlugins = 1;

    jlong cPluginsArray[numberOfPlugins];

    JavaStringResource pluginName(env, jPluginName);

    if (pluginName.str == "face_detection") {
        cPluginsArray[0] = reinterpret_cast<jlong>(new FaceTrackerPlugin());
    } else if (pluginName.str == "barcode") {
        cPluginsArray[0] = reinterpret_cast<jlong>(new BarcodePlugin);
    } else if ( pluginName.str == "customcamera" ) {
        cPluginsArray[0] = reinterpret_cast<jlong>(new YUVFrameInputPlugin());
    } else if ( pluginName.str == "markertracking") {
        cPluginsArray[0] = reinterpret_cast<jlong>(new MarkerTrackerPlugin());
    } else if ( pluginName.str == "simple_input_plugin" ) {
        cPluginsArray[0] = reinterpret_cast<jlong>(new SimpleInputPlugin());
    }

    jlongArray jPluginsArray = env->NewLongArray(numberOfPlugins);
    if (jPluginsArray != nullptr) {
        env->SetLongArrayRegion(jPluginsArray, 0, numberOfPlugins, cPluginsArray);
    }

    return jPluginsArray;
}
