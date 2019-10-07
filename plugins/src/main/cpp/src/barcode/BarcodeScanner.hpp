//
//  BarcodeScanner.hpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef BarcodeScanner_hpp
#define BarcodeScanner_hpp

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <zbar.h>
#pragma clang diagnostic pop


namespace wikitude {
    namespace sdk {
        namespace impl {
            class ManagedCameraFrame;
        }
        using impl::ManagedCameraFrame;
    }
}

class BarcodeScanner {
public:
    class Observer {
    public:
        virtual ~Observer() = default;

        virtual void recognizedBarcode(long id_, const std::string& barcode_) = 0;
        virtual void lostBarcode(long id_) = 0;
    };

public:
    BarcodeScanner(long id_, Observer& observer_);
    ~BarcodeScanner();

    void processCameraFrame(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_);

protected:
    long                _id;

    Observer&           _observer;

    int                 _numberOfPreviouslyRecognizedTargets;

    zbar::Image         _image;
    zbar::ImageScanner  _imageScanner;
};

#endif /* BarcodeScanner_hpp */
