
//
// OpenGLESScanningEffectRenderingPluginModule
//
// Created by Alexander Bendl on 22.06.18.
// Copyright (c) 2018 Wikitude. All rights reserved.
//

#ifndef OpenGLESScanningEffectRenderingPluginModule_hpp
#define OpenGLESScanningEffectRenderingPluginModule_hpp


#include <atomic>
#include <chrono>
#include <string>
#include <mutex>
#include <vector>

#include <GLES2/gl2.h>

#include <OpenGLESRenderingPluginModule.hpp>
#include <Matrix4.hpp>
#include <Geometry.hpp>


namespace wikitude {
    namespace sdk {
        namespace impl {
            class RecognizedTargetsBucket;
            class RenderableCameraFrame;
            class TrackingParameters;
        }
        using impl::RecognizedTargetsBucket;
        using impl::RenderableCameraFrame;
        using impl::TrackingParameters;
    }
}
class YUVFrameShaderSourceObject;

class OpenGLESScanningEffectRenderingPluginModule : public wikitude::sdk::OpenGLESRenderingPluginModule {
public:
    OpenGLESScanningEffectRenderingPluginModule(wikitude::sdk::TrackingParameters& trackingParameters_);
    ~OpenGLESScanningEffectRenderingPluginModule();

    void pause() override;

    void update(const wikitude::sdk::RecognizedTargetsBucket& recognizedTargetsBucket_);    

    void startRender(wikitude::sdk::RenderableCameraFrameBucket& frameBucket_) override ;
    void endRender(wikitude::sdk::RenderableCameraFrameBucket& frameBucket_) override;

    void cameraToSurfaceAngleChanged(float cameraToSurfaceAngle_);
    void cameraToSurfaceScalingChanged(wikitude::sdk::Scale2D<float> cameraToSurfaceScaling_);
    void cameraToSurfaceCorrectedFieldOfViewChanged(float cameraToSurfaceCorrectedFieldOfView_);
    void cameraFrameSizeChanged(wikitude::sdk::Size<int> cameraFrameSize_);

private:
    void setupRendering(wikitude::sdk::Size<int> currentCameraFrameSize_);
    void render(wikitude::sdk::RenderableCameraFrame& frame_);

    void createVertexBuffers();
    void releaseVertexBuffers();

    GLuint compileShader(const std::string& shaderSource, const GLenum shaderType);
    GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
    void createShaderProgram(const YUVFrameShaderSourceObject& shaderObject);
    void releaseShaderProgram();

    void setVertexShaderUniforms(GLuint shaderHandle);
    void setFragmentShaderUniforms(float viewportWidth, float viewportHeight);
    void setVertexAttributes(GLuint shaderHandle);

    void createFrameTextures(GLsizei width, GLsizei height);
    void updateFrameTexturesData(GLsizei width, GLsizei height, const unsigned char* const frameData);
    void bindFrameTextures();
    void releaseFrameTextures();

    void createFrameBufferObject(GLsizei width, GLsizei height);
    void bindFrameBufferObject();
    void unbindFrameBufferObject();
    void releaseFramebufferObject();

    void updateMatrices();

private:
    wikitude::sdk::TrackingParameters& _trackingParameters;

    std::atomic_bool _renderingInitialized;

    GLuint _scanlineShaderHandle;
    GLuint _fullscreenTextureShader;
    GLuint _augmentationShaderHandle;

    GLuint _programHandle;

    GLuint _vertexBuffer;
    GLuint _indexBuffer;

    GLuint _positionAttributeLocation;
    GLuint _texCoordAttributeLocation;

    GLuint _lumaTexture;
    GLuint _chromaTexture;

    GLint _defaultFrameBuffer;
    GLuint _frameBufferObject;
    GLuint _frameBufferTexture;

    struct Vertex
    {
        GLfloat position[3];
        GLfloat texCoord[2];
    };

    Vertex _vertices[4];

    const GLushort _indices[6];

    wikitude::sdk::Matrix4 _orientationMatrix;
    wikitude::sdk::Matrix4 _fboCorrectionMatrix;
    wikitude::sdk::Matrix4 _toSizeInPixelsScaling;
    wikitude::sdk::Matrix4 _modelMatrix;

    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> currentTime;

    std::mutex              _projectionMatrixMutex;
    wikitude::sdk::Matrix4  _projectionMatrix;

    std::atomic_bool        _trackingImageTargets;

    std::mutex              _imageTargetMutex;
    wikitude::sdk::Matrix4  _imageTargetMatrix;

    std::mutex                      _surfaceInitializedMutex;
    bool                            _cameraToSurfaceAngleInitialized;
    float                           _cameraToSurfaceAngle;
    bool                            _cameraToSurfaceScalingInitialized;
    wikitude::sdk::Scale2D<float>   _cameraToSurfaceScaling;
    bool                            _surfaceInitialized;

    std::atomic<float>              _cameraToSurfaceCorrectedFieldOfView;
    std::atomic<float>              _cameraFrameAspectRatio;
};

#endif //OpenGLESScanningEffectRenderingPluginModule_hpp
