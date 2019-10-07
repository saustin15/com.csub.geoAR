//
//  ErrorHandling.hpp
//  CommonLibrary
//
//  Created by Andreas Schacherbauer on 23.02.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef ErrorHandling_hpp
#define ErrorHandling_hpp

#ifdef __cplusplus

#include "CompilerAttributes.hpp"
#include "Error.hpp"


namespace wikitude { namespace sdk {

    namespace impl {


        struct CallStatus {
        public:
            static CallStatus Success() {
                return {true, {0, "", ""}};
            }
            
        public:
            CallStatus(bool success_, sdk::Error error_)
            :
            _success(success_),
            _error(error_)
            { /* Intentionally Left Blank */ }

            explicit operator bool () const {
                return _success;
            }

            bool        _success;
            sdk::Error  _error;
        };


        template <typename T>
        struct CallValue {
        public:
            static CallValue<T> SuccessCallValue(T value_) {
                return {value_, {true, {0, "", ""}}};
            }

        public:
            CallValue<T>(T value_, CallStatus status_)
            :
            _value(value_),
            _status(status_)
            { /* Intentionally Left Blank */ }

            explicit operator bool () const {
                return _status._success;
            }


        public:
            T           _value;
            CallStatus  _status;
        };
    }
    using impl::CallStatus;
    using impl::CallValue;
}}

#endif /* __cplusplus */

#endif /* ErrorHandling_hpp */
