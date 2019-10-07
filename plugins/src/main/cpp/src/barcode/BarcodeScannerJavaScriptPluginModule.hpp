//
//  BarcodeScannerJavaScriptPluginModule.hpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef BarcodeScannerJavaScriptPluginModule_hpp
#define BarcodeScannerJavaScriptPluginModule_hpp

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "JavaScriptPluginModule.hpp"
#else
    #import <WikitudeSDK/JavaScriptPluginModule.hpp>
#endif

#include "BarcodeScanner.hpp"


class BarcodeScannerJavaScriptPluginModule : public wikitude::sdk::JavaScriptPluginModule {
public:
    using CreateBarcodeScannerHandle = std::function<void(long id_)>;

public:
    BarcodeScannerJavaScriptPluginModule(CreateBarcodeScannerHandle createBarcodeScannerHandle_);

    /* From JavaScriptPluginModule */
    JavaScriptPluginModule::JavaScriptAPI getJavaScriptAPI() override;
    void createInstance(const std::string& className_, long id_, const std::string& parameter_) override;

    /* Public barcode plugin API */
    void callBarcodeRecognizedCallback(long id_, const std::string& barcode_);
    void callBarcodeLostCallback(long id_);

protected:
    CreateBarcodeScannerHandle  _createBarcodeScannerHandle;
};

#endif /* BarcodeScannerJavaScriptPluginModule_hpp */
