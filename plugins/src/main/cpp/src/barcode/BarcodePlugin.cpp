//
//  BarcodePlugin.cpp
//  DevApplication
//
//  Created by Andreas Schacherbauer on 15/05/15.
//  Copyright (c) 2015 Wikitude. All rights reserved.
//

#include "BarcodePlugin.h"

#include <iostream>
#include <sstream>

#include "BarcodeScannerJavaScriptPluginModule.hpp"


BarcodePlugin::BarcodePlugin()
:
ArchitectPlugin("com.wikitude.plugins.barcode_scanner_demo")
{
    setJavaScriptPluginModule(std::make_unique<BarcodeScannerJavaScriptPluginModule>([&](long id_) {
        _registeredBarcodeScanner.emplace(id_, std::make_unique<BarcodeScanner>(id_, *this));
    }));
}

void BarcodePlugin::cameraFrameAvailable(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_) {
    
    for ( auto& barcodeScannerPair : _registeredBarcodeScanner ) {
        barcodeScannerPair.second->processCameraFrame(managedCameraFrame_);
    }
}

void BarcodePlugin::recognizedBarcode(long id_, const std::string& barcode_) {
    getJavaScriptPluginModule()->callBarcodeRecognizedCallback(id_, barcode_);
}

void BarcodePlugin::lostBarcode(long id_) {
    getJavaScriptPluginModule()->callBarcodeLostCallback(id_);
}

BarcodeScannerJavaScriptPluginModule* BarcodePlugin::getJavaScriptPluginModule() {
    return dynamic_cast<BarcodeScannerJavaScriptPluginModule*>(ArchitectPlugin::getJavaScriptPluginModule());
}
