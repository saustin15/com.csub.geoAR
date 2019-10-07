//
//  StrokedRectangle.hpp
//  ArchitectCore
//
//  Created by Andreas Schacherbauer on 25.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef StrokedRectangle_hpp
#define StrokedRectangle_hpp

#ifdef __ANDROID__
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#elif _WIN32
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    typedef unsigned int uint;
#elif PLATFORM_BB10
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_OSX
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
    #elif TARGET_OS_IOS
        #include <OpenGLES/ES3/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#endif

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "Matrix4.hpp"
    #include "Geometry.hpp"
#else
    #include <WikitudeSDK/Matrix4.hpp>
    #include <WikitudeSDK/Geometry.hpp>
#endif


namespace wikitude { namespace sdk {
    
    namespace opengl {
        
        
        class StrokedRectangle {
        public:
            void setEnabled(bool enabled_);
            void setScale(wikitude::sdk::Scale2D<float> scale_);
            void setUniformScale(float uniformScale_);
            
            void render(wikitude::sdk::Matrix4& matrix_, wikitude::sdk::Matrix4& projection_);
            void release();
            
        protected:
            GLuint compileShader(std::string shaderName_, GLenum shaderType_);
            void compileShaders();
            void deleteShaders();
            
        protected:
            bool                            _enabled = true;
            wikitude::sdk::Scale2D<float>   _scale = {1.f, 1.f};
            
            GLuint          _augmentationProgram = 0;
            GLuint          _positionSlot = 0;
            GLint           _projectionUniform = 0;
            GLint           _modelViewUniform = 0;
        };
    }
}}

#endif /* StrokedRectangle_hpp */
