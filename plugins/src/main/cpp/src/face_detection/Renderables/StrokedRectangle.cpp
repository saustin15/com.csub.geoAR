//
//  StrokedRectangle.cpp
//  ArchitectCore
//
//  Created by Andreas Schacherbauer on 25.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "StrokedRectangle.hpp"


#ifdef ASSERT_OPENGL
    #ifndef WT_GL_ASSERT
    #define WT_GL_ASSERT( __gl_code__ ) { \
        __gl_code__; \
        GLenum __wt_gl_error_code__ = glGetError(); \
        if ( __wt_gl_error_code__ != GL_NO_ERROR ) { \
            printf("OpenGL error '%x' occured at line %d inside function %s\n", __wt_gl_error_code__, __LINE__, __PRETTY_FUNCTION__); \
        } \
    }
    #endif

    #ifndef WT_GL_ASSERT_AND_RETURN
    #define WT_GL_ASSERT_AND_RETURN( __assign_to__, __gl_code__ ) { \
        __assign_to__ = __gl_code__; \
        GLenum __wt_gl_error_code__ = glGetError(); \
        if ( __wt_gl_error_code__ != GL_NO_ERROR ) { \
            printf("OpenGL error '%x' occured at line %d inside function %s\n", __wt_gl_error_code__, __LINE__, __PRETTY_FUNCTION__); \
        } \
    }
    #endif
#else
    #ifndef WT_GL_ASSERT
    #define WT_GL_ASSERT( __gl_code__ ) __gl_code__
    #endif

    #ifndef WT_GL_ASSERT_AND_RETURN
    #define WT_GL_ASSERT_AND_RETURN( __assign_to__, __gl_code__ ) __assign_to__ = __gl_code__
    #endif
#endif

namespace wikitude { namespace sdk {
    
    namespace opengl {


        void StrokedRectangle::setEnabled(bool enabled_) {
            _enabled = enabled_;
        }
        
        void StrokedRectangle::setScale(sdk::Scale2D<float> scale_) {
            _scale = scale_;
        }
        
        void StrokedRectangle::setUniformScale(float uniformScale_) {
            _scale = {uniformScale_, uniformScale_};
        }
        
        void StrokedRectangle::render(sdk::Matrix4& matrix_, wikitude::sdk::Matrix4& projection_) {
            if ( _enabled )
            {
                if ( !_augmentationProgram )
                {
                    compileShaders();
                    
                    WT_GL_ASSERT_AND_RETURN( _positionSlot, static_cast<GLuint>(glGetAttribLocation(_augmentationProgram, "v_position")) );
                    
                    WT_GL_ASSERT_AND_RETURN( _projectionUniform, glGetUniformLocation(_augmentationProgram, "Projection") );
                    WT_GL_ASSERT_AND_RETURN( _modelViewUniform, glGetUniformLocation(_augmentationProgram, "Modelview") );
                    
                    WT_GL_ASSERT( glDisable(GL_DEPTH_TEST) );
                    WT_GL_ASSERT( glLineWidth(10.0f) );
                }
                
                WT_GL_ASSERT( glDisable(GL_DEPTH_TEST) );
                WT_GL_ASSERT( glUseProgram(_augmentationProgram) );
                
                /* reset any previously bound buffer */
                WT_GL_ASSERT( glBindBuffer(GL_ARRAY_BUFFER, 0) );
                WT_GL_ASSERT( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
                
                static GLfloat rectVerts[] = {  -0.5f,  -0.5f, 0.0f,
                    -0.5f,  0.5f, 0.0f,
                    0.5f, 0.5f, 0.0f,
                    0.5f, -0.5f, 0.0f };
                
                
                // Load the vertex position
                WT_GL_ASSERT( glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, 0, rectVerts) );
                WT_GL_ASSERT( glEnableVertexAttribArray(_positionSlot) );
                
                WT_GL_ASSERT( glUniformMatrix4fv(_projectionUniform, 1, 0, projection_.get()) );
                
                sdk::Matrix4 scaleMatrix;
                scaleMatrix.scale(_scale.x, _scale.y , 1.f);
                
                sdk::Matrix4 finalModelViewMatrix = matrix_ * scaleMatrix;
                WT_GL_ASSERT( glUniformMatrix4fv(_modelViewUniform, 1, 0, finalModelViewMatrix.get()) );
                
                static GLushort lindices[4] = {0,1,2,3};
                GLsizei numberOfIndices = sizeof(lindices)/sizeof(lindices[0]);
                WT_GL_ASSERT( glDrawElements(GL_LINE_LOOP, numberOfIndices, GL_UNSIGNED_SHORT, lindices) );
            }
        }
        
        void StrokedRectangle::release() {
            deleteShaders();
        }
        
#pragma mark - Private Methods
        GLuint StrokedRectangle::compileShader(std::string shaderSource_, GLenum shaderType_) {
            
            GLuint shaderHandle = glCreateShader(shaderType_);
            
            const char * shaderStringUTF8 = shaderSource_.c_str();
            int shaderStringLength = static_cast<int>(shaderSource_.size());
            WT_GL_ASSERT( glShaderSource(shaderHandle, 1, &shaderStringUTF8, &shaderStringLength) );
            
            WT_GL_ASSERT( glCompileShader(shaderHandle) );
            
            GLint compileSuccess;
            WT_GL_ASSERT( glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess) );
            if (compileSuccess == GL_FALSE) {
                GLchar messages[256];
                WT_GL_ASSERT( glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]) );
                std::string errorMessage(messages);
                std::cout << "Error compiling shader source. " << errorMessage << "\n";
                exit(1);
            }
            
            return shaderHandle;
        }
        
        void StrokedRectangle::compileShaders() {
            const std::string vertexShaderSource(R"(
                                           attribute vec4 v_position;
                                           uniform mat4 Projection;
                                           uniform mat4 Modelview;
                                           
                                           void main()
                                            {
                                                gl_Position = Projection * Modelview * v_position;
                                            }
                                           )");
            GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
            
            const std::string fragmentShaderSource(R"(
                                             precision mediump float;
                                             void main()
                                            {
                                                gl_FragColor = vec4(1.0, 0.58, 0.16, 1.0);
                                            }
                                             )");
            GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
            
            _augmentationProgram = glCreateProgram();
            WT_GL_ASSERT( glAttachShader(_augmentationProgram, vertexShader) );
            WT_GL_ASSERT( glAttachShader(_augmentationProgram, fragmentShader) );
            WT_GL_ASSERT( glLinkProgram( _augmentationProgram) );
            
            
            GLint linkSuccess;
            WT_GL_ASSERT( glGetProgramiv(_augmentationProgram, GL_LINK_STATUS, &linkSuccess) );
            if (linkSuccess == GL_FALSE) {
                GLchar messages[256];
                WT_GL_ASSERT( glGetProgramInfoLog(_augmentationProgram, sizeof(messages), 0, &messages[0]) );
                std::string errorMessage(messages);
                std::cout << "Error compiling shader source. " << errorMessage << "\n";
                exit(1);
            }
            
            WT_GL_ASSERT( glDeleteShader(vertexShader) );
            WT_GL_ASSERT( glDeleteShader(fragmentShader) );
        }
        
        void StrokedRectangle::deleteShaders() {
            if ( _augmentationProgram ) {
                WT_GL_ASSERT( glDeleteProgram(_augmentationProgram) );
                _augmentationProgram = 0;
            }
        }
    }
}}
