//
//  SimpleInputPlugin.cpp
//
//  Created by Alexander Bendl on 02/03/17.
//  Copyright © 2017 Wikitude. All rights reserved.
//

#include "SimpleInputPlugin.h"

#include "FrameInputPluginModule.hpp"
#include "jniHelper.h"


SimpleInputPlugin* SimpleInputPlugin::instance;
jobject simpleInputPluginExtension;

SimpleInputPlugin::SimpleInputPlugin() :
        wikitude::sdk::Plugin("plugin.input.yuv.simple"){
    SimpleInputPlugin::instance = this;

    setCameraFrameInputPluginModule(std::make_unique<FrameInputPluginModule>(true /*reguest rendering*/, [&]{
        callInitializedJNIMethod(_sdkCameraReleasedMethodId);
    }));
}

SimpleInputPlugin::~SimpleInputPlugin() {
    JavaVMResource vm;
    vm.env->DeleteGlobalRef(simpleInputPluginExtension);
}

/**
 * Will be called once after your Plugin was successfully added to the Wikitude Engine. Initialize your plugin here.
 */
void SimpleInputPlugin::initialize(const std::string& temporaryDirectory_, wikitude::sdk::PluginParameterCollection& pluginParameterCollection_) {

}

/**
 * Will be called every time the Wikitude Engine pauses.
 */
void SimpleInputPlugin::pause() {
    callInitializedJNIMethod(_pluginPausedMethodId);
}

/**
 * Will be called when the Wikitude Engine starts for the first time and after every pause.
 *
 * @param pausedTime_ the duration of the pause in milliseconds
 */
void SimpleInputPlugin::resume(unsigned int /*pausedTime_*/) {
    callInitializedJNIMethod(_pluginResumedMethodId);
}

/**
 * Will be called when the Wikitude Engine shuts down.
 */
void SimpleInputPlugin::destroy() {
    callInitializedJNIMethod(_pluginDestroyedMethodId);
}

/**
 * Will be called after every image recognition cycle.
 *
 * @param recognizedTargets_ list of recognized targets, empty if no target was recognized
 */
void SimpleInputPlugin::update(const wikitude::sdk::RecognizedTargetsBucket& recognizedTargetsBucket_) {
    /* Intentionally Left Blank */
}

void SimpleInputPlugin::cameraFrameAvailable(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_) {
    /* Intentionally Left Blank */
}

void SimpleInputPlugin::callInitializedJNIMethod(jmethodID methodId_) {
    JavaVMResource vm;
    vm.env->CallVoidMethod(simpleInputPluginExtension, methodId_);
}

void SimpleInputPlugin::initializeJni(JNIEnv* env_) {
    jclass simpleInputPluginActivityClass = env_->GetObjectClass(simpleInputPluginExtension);
    _sdkCameraReleasedMethodId = env_->GetMethodID(simpleInputPluginActivityClass, "onSDKCameraReleased", "()V");
    _pluginPausedMethodId = env_->GetMethodID(simpleInputPluginActivityClass, "onInputPluginPaused", "()V");
    _pluginResumedMethodId = env_->GetMethodID(simpleInputPluginActivityClass, "onInputPluginResumed", "()V");
    _pluginDestroyedMethodId = env_->GetMethodID(simpleInputPluginActivityClass, "onInputPluginDestroyed", "()V");
}

/**
 * Initialize c++->java connection.
 */
extern "C" JNIEXPORT void JNICALL
Java_com_wikitude_samples_advanced_plugins_input_SimpleInputPluginExtension_initNative(JNIEnv* env_, jobject obj) {
    simpleInputPluginExtension = env_->NewGlobalRef(obj);
    SimpleInputPlugin::instance->initializeJni(env_);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_wikitude_samples_advanced_plugins_input_SimpleInputPluginExtension_getInputModuleHandle(JNIEnv*, jobject) {
    return reinterpret_cast<jlong>(SimpleInputPlugin::instance->getCameraFrameInputPluginModule());
}