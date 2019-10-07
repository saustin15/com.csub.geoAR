
//
// OpenGLESScanningEffectRenderingPluginModule
//
// Created by Alexander Bendl on 22.06.18.
// Copyright (c) 2018 Wikitude. All rights reserved.
//

#include "OpenGLESScanningEffectRenderingPluginModule.hpp"

#include <android/log.h>

#include <CameraFramePlane.hpp>
#include <ManagedCameraFrame.hpp>
#include <RecognizedTargetsBucket.hpp>
#include <RenderableCameraFrameBucket.hpp>
#include <RenderableCameraFrame.hpp>
#include <TrackingParameters.hpp>

#include "YUVFrameShaderSourceObject.hpp"


#define WT_GL_ASSERT( __gl_code__ ) { \
    __gl_code__; \
    GLenum __wt_gl_error_code__ = glGetError(); \
    if ( __wt_gl_error_code__ != GL_NO_ERROR ) { \
        __android_log_print(ANDROID_LOG_ERROR, "CustomCamera", "OpenGL error %d occurred at line %d inside function %s", __wt_gl_error_code__, __LINE__, __PRETTY_FUNCTION__); \
    } \
}
#define WT_GL_ASSERT_AND_RETURN( __assign_to__, __gl_code__ ) { \
    __assign_to__ = __gl_code__; \
    GLenum __wt_gl_error_code__ = glGetError(); \
    if ( __wt_gl_error_code__ != GL_NO_ERROR ) { \
        __android_log_print(ANDROID_LOG_ERROR, "CustomCamera", "OpenGL error %d occurred at line %d inside function %s", __wt_gl_error_code__, __LINE__, __PRETTY_FUNCTION__); \
    } \
}

const std::string scanlineVertexShaderSource = "\
attribute vec3 vPosition;\
attribute vec2 vTexCoords;\
\
varying mediump vec2 fTexCoords;\
\
uniform mat4 uModelMatrix;\
\
void main(void)\
{\
    gl_Position = uModelMatrix * vec4(vPosition, 1.0);\
    fTexCoords = vTexCoords;\
}";

const std::string scanlineFragmentShaderSource = "\
uniform lowp sampler2D uCameraFrameTexture;\
uniform mediump vec2 uViewportResolution;\
uniform lowp float uTimeInSeconds;\
uniform lowp float uScanningAreaHeightInPixels;\
\
varying mediump vec2 fTexCoords;\
\
const lowp vec3 rgb2luma = vec3(0.216, 0.7152, 0.0722);\
\
const lowp vec3 ones = vec3(1.0);\
\
const lowp vec3 sobelPositive = vec3(1.0, 2.0, 1.0);\
const lowp vec3 sobelNegative = vec3(-1.0, -2.0, -1.0);\
\
const lowp float animationSpeed = 3.0;\
\
lowp float RGB2Luminance(in lowp vec3 rgb)\
{\
    return dot(rgb2luma * rgb, ones);\
}\
\
void main()\
{\
    mediump vec2 sspPixelSize = vec2(1.0) / uViewportResolution.xy;\
    \
    lowp vec4 cameraFrameColor = texture2D(uCameraFrameTexture, fTexCoords);\
    \
    lowp float tl = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + -sspPixelSize.xy).rgb);\
    lowp float t = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(0.0, -sspPixelSize.y)).rgb);\
    lowp float tr = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(sspPixelSize.x, -sspPixelSize.y)).rgb);\
    \
    lowp float cl = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(-sspPixelSize.x, 0.0)).rgb);\
    lowp float c = RGB2Luminance(cameraFrameColor.rgb);\
    lowp float cr = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(sspPixelSize.x, 0.0)).rgb);\
    \
    lowp float bl = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(-sspPixelSize.x, sspPixelSize.y)).rgb);\
    lowp float b = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(0.0, sspPixelSize.y)).rgb);\
    lowp float br = RGB2Luminance(texture2D(uCameraFrameTexture, fTexCoords + vec2(sspPixelSize.x, sspPixelSize.y)).rgb);\
    \
    lowp float sobelX = dot(sobelNegative * vec3(tl, cl, bl) + sobelPositive * vec3(tr, cr, br), ones);\
    lowp float sobelY = dot(sobelNegative * vec3(tl, t, tr) + sobelPositive * vec3(bl, b, br), ones);\
    \
    lowp float sobel = length(vec2(sobelX, sobelY));\
    \
    mediump float sspScanlineY = sin(uTimeInSeconds * animationSpeed) * 0.5 + 0.5;\
    mediump float sspFragmentCenterY = gl_FragCoord.y / uViewportResolution.y;\
    \
    lowp float sspScanWindowHeight = uScanningAreaHeightInPixels * sspPixelSize.y;\
    \
    mediump float distanceToScanline = clamp(0.0, sspScanWindowHeight, distance(sspScanlineY, sspFragmentCenterY)) / sspScanWindowHeight;\
    \
    gl_FragColor = vec4(mix(mix(vec3(c), vec3(1.0, 0.549, 0.0392), step(0.15, sobel)), cameraFrameColor.rgb, smoothstep(0.3, 0.7, distanceToScanline)), 1.0);\
}";

const std::string fullscreenTextureFragmentShader = "\
uniform lowp sampler2D uCameraFrameTexture;\
\
varying mediump vec2 fTexCoords;\
\
void main() {\
    gl_FragColor = vec4(texture2D(uCameraFrameTexture, fTexCoords).rgb, 1.0);\
}";

const std::string augmentationVertexShaderSource = "\
uniform mat4 uModelViewProjectionMatrix;\
\
attribute vec3 vPosition;\
attribute vec2 vTexCoords;\
\
varying mediump vec2 fTexCoords;\
\
void main() {\
    gl_Position = uModelViewProjectionMatrix * vec4(vPosition, 1.0);\
    fTexCoords = vTexCoords;\
}";

const std::string augmentationFragmentShaderSource = "\
varying mediump vec2 fTexCoords;\
\
void main()\
{\
    gl_FragColor = vec4(vec3(1.0, 0.549, 0.0392), 1.0);\
}";


OpenGLESScanningEffectRenderingPluginModule::OpenGLESScanningEffectRenderingPluginModule(wikitude::sdk::TrackingParameters& trackingParameters_) :
_trackingParameters(trackingParameters_),
_renderingInitialized(false),
_cameraToSurfaceAngleInitialized(false),
_cameraToSurfaceScalingInitialized(false),
_surfaceInitialized(false),
_scanlineShaderHandle(0),
_fullscreenTextureShader(0),
_augmentationShaderHandle(0),
_programHandle(0),
_vertexBuffer(0),
_indexBuffer(0),
_positionAttributeLocation(0),
_texCoordAttributeLocation(0),
_lumaTexture(0),
_chromaTexture(0),
_frameBufferObject(0),
_frameBufferTexture(0),
_defaultFrameBuffer(0),
_indices{0, 1, 2, 2, 3, 0},
_trackingImageTargets(false),
_cameraToSurfaceAngle(0),
_cameraToSurfaceScaling({0, 0})
{
    startTime = std::chrono::system_clock::now();

    _fboCorrectionMatrix.scale(1.0f, -1.0f, 1.0f);

    // scaled to be able to re-use the screen aligned quad (side length = 2) and thus
    // avoid having to create additional vertex data and correspondig code
    _toSizeInPixelsScaling.scale(0.5f, 0.5f, 1.0f);
}

OpenGLESScanningEffectRenderingPluginModule::~OpenGLESScanningEffectRenderingPluginModule() {
    releaseFramebufferObject();
    releaseShaderProgram();
    releaseVertexBuffers();
    releaseFrameTextures();
}

void OpenGLESScanningEffectRenderingPluginModule::pause() {
    releaseFramebufferObject();
    releaseFrameTextures();
    releaseVertexBuffers();
    releaseShaderProgram();

    _renderingInitialized = false;
}

void OpenGLESScanningEffectRenderingPluginModule::update(const wikitude::sdk::RecognizedTargetsBucket& recognizedTargetsBucket_) {
    _trackingImageTargets = !recognizedTargetsBucket_.getImageTargets().empty();
    if ( _trackingImageTargets ) {
        std::lock_guard<std::mutex> imageTargetMatrixUpdateLock(_imageTargetMutex);
        const wikitude::sdk::ImageTarget& imageTarget = *recognizedTargetsBucket_.getImageTargets().front();
        const wikitude::sdk::Matrix4& modelView = imageTarget.getMatrix();

        wikitude::sdk::Matrix4 scaleMatrix;
        scaleMatrix.scale(imageTarget.getTargetScale().x, imageTarget.getTargetScale().y, 1.0);

        _imageTargetMatrix = _projectionMatrix * modelView * scaleMatrix * _toSizeInPixelsScaling;
    }
}

void OpenGLESScanningEffectRenderingPluginModule::startRender(wikitude::sdk::RenderableCameraFrameBucket& frameBucket_) {    
    long processedFrameId_ = _trackingParameters.getProcessedFrameId();
    frameBucket_.getRenderableCameraFrameForId(processedFrameId_, [this](wikitude::sdk::RenderableCameraFrame& frame_) {
        render(frame_);
    }, [](wikitude::sdk::Error& error_) {
        std::cout << error_;
    });
}

void OpenGLESScanningEffectRenderingPluginModule::endRender(wikitude::sdk::RenderableCameraFrameBucket& frameBucket_) {
    /* intentionally left blank */
}

void OpenGLESScanningEffectRenderingPluginModule::cameraToSurfaceAngleChanged(float cameraToSurfaceAngle_) {
    _cameraToSurfaceAngle = cameraToSurfaceAngle_;

    std::lock_guard<std::mutex> l(_surfaceInitializedMutex);

    _cameraToSurfaceAngleInitialized = true;

    if (_cameraToSurfaceScalingInitialized) {
        _surfaceInitialized = true;

        updateMatrices();
    }
}

void OpenGLESScanningEffectRenderingPluginModule::cameraToSurfaceScalingChanged(wikitude::sdk::Scale2D<float> cameraToSurfaceScaling_) {
    _cameraToSurfaceScaling = cameraToSurfaceScaling_;

    std::lock_guard<std::mutex> l(_surfaceInitializedMutex);

    _cameraToSurfaceScalingInitialized = true;

    if (_cameraToSurfaceAngleInitialized) {
        _surfaceInitialized = true;

        updateMatrices();
    }
}


void OpenGLESScanningEffectRenderingPluginModule::cameraToSurfaceCorrectedFieldOfViewChanged(float cameraToSurfaceCorrectedFieldOfView_) {
    _cameraToSurfaceCorrectedFieldOfView = cameraToSurfaceCorrectedFieldOfView_;
    std::lock_guard<std::mutex> projectionModificationLock(_projectionMatrixMutex);

    _projectionMatrix.perspective(_cameraToSurfaceCorrectedFieldOfView, _cameraFrameAspectRatio, 0.01f, 500.0f);
}

void OpenGLESScanningEffectRenderingPluginModule::cameraFrameSizeChanged(wikitude::sdk::Size<int> cameraFrameSize_) {
    _cameraFrameAspectRatio = static_cast<float>(cameraFrameSize_.width) / cameraFrameSize_.height;

    std::lock_guard<std::mutex> projectionModificationLock(_projectionMatrixMutex);
    _projectionMatrix.perspective(_cameraToSurfaceCorrectedFieldOfView, _cameraFrameAspectRatio, 0.05f, 5000.0f);
}

void OpenGLESScanningEffectRenderingPluginModule::setupRendering(wikitude::sdk::Size<int> currentCameraFrameSize_) {
    _vertices[0] = (Vertex){{1.0f, -1.0f, 0}, {1.0f, 0.0f}};
    _vertices[1] = (Vertex){{1.0f, 1.0f, 0}, {1.0f, 1.0f}};
    _vertices[2] = (Vertex){{-1.0f, 1.0f, 0}, {0.0f, 1.0f}};
    _vertices[3] = (Vertex){{-1.0f, -1.0f, 0}, {0.0f, 0.0f}};

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_defaultFrameBuffer);

    _scanlineShaderHandle = createShaderProgram(scanlineVertexShaderSource, scanlineFragmentShaderSource);
    _fullscreenTextureShader = createShaderProgram(scanlineVertexShaderSource, fullscreenTextureFragmentShader);
    _augmentationShaderHandle = createShaderProgram(augmentationVertexShaderSource, augmentationFragmentShaderSource);
    createShaderProgram(YUVFrameShaderSourceObject());
    createVertexBuffers();

    createFrameTextures(currentCameraFrameSize_.width, currentCameraFrameSize_.height);
    createFrameBufferObject(currentCameraFrameSize_.width, currentCameraFrameSize_.height);
}

void OpenGLESScanningEffectRenderingPluginModule::render(wikitude::sdk::RenderableCameraFrame& frame_) {

    if (!_renderingInitialized.load()) {
        setupRendering(frame_.getColorMetadata().getPixelSize());
        _renderingInitialized.store(true);
    }

    {
        std::lock_guard<std::mutex> l(_surfaceInitializedMutex);
        if (!_surfaceInitialized) {
            return;
        }
    }

    const std::vector<wikitude::sdk::CameraFramePlane>& planes = frame_.getData();
    const wikitude::sdk::ColorCameraFrameMetadata& metadata = frame_.getColorMetadata();

    WT_GL_ASSERT(glDisable(GL_DEPTH_TEST));

    WT_GL_ASSERT(glUseProgram(_programHandle));

    WT_GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer));
    WT_GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer));

    WT_GL_ASSERT(glEnableVertexAttribArray(_positionAttributeLocation));
    WT_GL_ASSERT(glEnableVertexAttribArray(_texCoordAttributeLocation));

    WT_GL_ASSERT(glVertexAttribPointer(_positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));
    WT_GL_ASSERT(glVertexAttribPointer(_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(float) * 3)));

    if (metadata.getFrameColorSpace() == wikitude::sdk::ColorSpace::YUV_420_NV21) {
        updateFrameTexturesData(metadata.getPixelSize().width, metadata.getPixelSize().height, reinterpret_cast<const unsigned char*>(planes[0].getData()));
    } else if (metadata.getFrameColorSpace() == wikitude::sdk::ColorSpace::YUV_420_888) {
        wikitude::sdk::CameraFramePlane luminancePlane = planes[0];
        wikitude::sdk::CameraFramePlane chromaBluePlane = planes[1];
        wikitude::sdk::CameraFramePlane chromaRedPlane = planes[2];

        const int rowStrideLuminance = luminancePlane.getRowStride();
        const int pixelStrideLuminance = luminancePlane.getPixelStride();

        const int rowStrideBlue = chromaBluePlane.getRowStride();
        const int pixelStrideBlue = chromaBluePlane.getPixelStride();

        const int rowStrideRed = chromaRedPlane.getRowStride();
        const int pixelStrideRed = chromaRedPlane.getPixelStride();

        const std::size_t luminanceDataSize = luminancePlane.getDataSize();
        const std::size_t chromaBlueDataSize = chromaBluePlane.getDataSize();
        const std::size_t chromaRedDataSize = chromaRedPlane.getDataSize();

        const int widthLuminance = metadata.getPixelSize().width;
        const int heightLuminance = metadata.getPixelSize().height;

        const int widthChrominance = widthLuminance / 2;
        const int heightChrominance = heightLuminance / 2;

        // frame data may be interlaced or padded with dummy data
        // divide by the pixel stride to get the count of useable frame data bytes
        const int correctedLuminanceDataSize = widthLuminance * heightLuminance;
        const int correctedBlueDataSize = widthChrominance * heightChrominance;
        const int correctedRedDataSize = widthChrominance * heightChrominance;

        const int frameDataSize = correctedLuminanceDataSize + correctedBlueDataSize + correctedRedDataSize;

        unsigned char* frameData = new unsigned char[frameDataSize];

        // as Y, Cb, and Cr originate from different image planes they could have different
        // memory alignments (pixelStride, rowStride), therefore each channel has a dedicated
        // loop although Cb and Cr could be handled simultaneously in the majority of cases

        // Y
        unsigned char* const luminanceStart = &frameData[0];

        unsigned char* YDestPtr = luminanceStart;

        const unsigned char* luminanceData = static_cast<const unsigned char*>(luminancePlane.getData());

        const unsigned char* YSrcPtr = luminanceData;
        const unsigned char* const YSrcPtrEnd = luminanceData + luminanceDataSize;

        for (; YSrcPtr < YSrcPtrEnd; YDestPtr += widthLuminance, YSrcPtr += rowStrideLuminance) {
            for (unsigned int i = 0; i < widthLuminance; ++i) {
                YDestPtr[i] = YSrcPtr[pixelStrideLuminance * i];
            }
        }


        // We require NV21 style memory alignment, thus the Cb and Cr are interlaced such that
        // Cr_0, Cb_0, Cr_1, Cb_1, ... Cr_n, Cb_n are consecutively placed in memory

        // Cb

        unsigned char* const chromaStart = &frameData[0] + correctedLuminanceDataSize;

        unsigned char* CbDestPtr = chromaStart;

        const unsigned char* chromaBlueData = static_cast<const unsigned char*>(chromaBluePlane.getData());

        const unsigned char* CbSrcPtr = chromaBlueData;
        const unsigned char* const CbSrcPtrEnd = chromaBlueData + chromaBlueDataSize;

        for (; CbSrcPtr < CbSrcPtrEnd; CbDestPtr += widthChrominance * 2, CbSrcPtr += rowStrideBlue) {
            for (unsigned int i = 0; i < widthChrominance; ++i) {
                CbDestPtr[2 * i + 1] = CbSrcPtr[pixelStrideBlue * i];
            }
        }

        // Cr
        unsigned char* CrDestPtr = chromaStart;

        const unsigned char* chromaRedData = static_cast<const unsigned char*>(chromaRedPlane.getData());

        const unsigned char* CrSrcPtr = chromaRedData;
        const unsigned char* const CrSrcPtrEnd = chromaRedData + chromaRedDataSize;

        for (; CrSrcPtr < CrSrcPtrEnd; CrDestPtr += widthChrominance * 2, CrSrcPtr += rowStrideRed) {
            for (unsigned int i = 0; i < widthChrominance; ++i) {
                CrDestPtr[2 * i] = CrSrcPtr[pixelStrideRed * i];
            }
        }

        updateFrameTexturesData(metadata.getPixelSize().width, metadata.getPixelSize().height, frameData);

        delete[] frameData;
    } else {
        assert(false && "Unsupported frame format; {YUV_420_NV21,YUV_420_888} is the set of valid values.");
    }

    bindFrameTextures();

    bindFrameBufferObject();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    const float viewportWidth = viewport[2] - viewport[0];
    const float viewportHeight = viewport[3] - viewport[1];

    WT_GL_ASSERT(glViewport(0, 0, 640, 480));
    WT_GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));
    WT_GL_ASSERT(glDrawElements(GL_TRIANGLES, sizeof(_indices)/sizeof(_indices[0]), GL_UNSIGNED_SHORT, 0));
    unbindFrameBufferObject();

    WT_GL_ASSERT(glViewport(viewport[0], viewport[1], viewport[2], viewport[3]));

    WT_GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));

    WT_GL_ASSERT(glActiveTexture(GL_TEXTURE0));
    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _frameBufferTexture));

    if ( _trackingImageTargets ) {
        /* In case an image target is tracked, we render the camera frame plus an augmentation */

        /* Camera frame rendering */
        WT_GL_ASSERT(glUseProgram(_fullscreenTextureShader));
        setVertexShaderUniforms(_fullscreenTextureShader);

        GLint fullscreenTextureUniformLocation;
        WT_GL_ASSERT_AND_RETURN(fullscreenTextureUniformLocation, glGetUniformLocation(_fullscreenTextureShader, "uCameraFrameTexture"));
        WT_GL_ASSERT(glUniform1i(fullscreenTextureUniformLocation, 0));

        setVertexAttributes(_fullscreenTextureShader);

        WT_GL_ASSERT(glDrawElements(GL_TRIANGLES, sizeof(_indices)/sizeof(_indices[0]), GL_UNSIGNED_SHORT, 0));

        /* Augmentation rendering */
        WT_GL_ASSERT(glUseProgram(_augmentationShaderHandle));
        GLint modelViewProjectionLocation;
        WT_GL_ASSERT_AND_RETURN(modelViewProjectionLocation, glGetUniformLocation(_augmentationShaderHandle, "uModelViewProjectionMatrix"));
        std::unique_lock<std::mutex> imageTargetMatrixAccessLock(_imageTargetMutex);
        WT_GL_ASSERT(glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, _imageTargetMatrix.get()));
        imageTargetMatrixAccessLock.unlock();

        setVertexAttributes(_augmentationShaderHandle);

        WT_GL_ASSERT(glDrawElements(GL_TRIANGLES, sizeof(_indices)/sizeof(_indices[0]), GL_UNSIGNED_SHORT, 0));
    }
    else {
        /* In case nothing is tracked, the camera frame with a scanning animation is rendered */
        WT_GL_ASSERT(glUseProgram(_scanlineShaderHandle));
        setVertexShaderUniforms(_scanlineShaderHandle);

        setFragmentShaderUniforms(viewportWidth, viewportHeight);
        setVertexAttributes(_scanlineShaderHandle);

        WT_GL_ASSERT(glDrawElements(GL_TRIANGLES, sizeof(_indices)/sizeof(_indices[0]), GL_UNSIGNED_SHORT, 0));
    }

    WT_GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
    WT_GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    WT_GL_ASSERT(glUseProgram(0));

    WT_GL_ASSERT(glEnable(GL_DEPTH_TEST));
}

void OpenGLESScanningEffectRenderingPluginModule::createVertexBuffers()
{
    WT_GL_ASSERT(glGenBuffers(1, &_vertexBuffer));
    WT_GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer));
    WT_GL_ASSERT(glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), &_vertices, GL_STATIC_DRAW));

    WT_GL_ASSERT(glGenBuffers(1, &_indexBuffer));
    WT_GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer));
    WT_GL_ASSERT(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices), _indices, GL_STATIC_DRAW));
}

void OpenGLESScanningEffectRenderingPluginModule::releaseVertexBuffers()
{
    if (_vertexBuffer) {
        WT_GL_ASSERT(glDeleteBuffers(1, &_vertexBuffer));
        _vertexBuffer = 0;
    }

    if (_indexBuffer) {
        WT_GL_ASSERT(glDeleteBuffers(1, &_indexBuffer));
        _vertexBuffer = 0;
    }
}

GLuint OpenGLESScanningEffectRenderingPluginModule::compileShader(const std::string& shaderSource, const GLenum shaderType)
{
    GLuint shaderHandle;
    WT_GL_ASSERT_AND_RETURN(shaderHandle, glCreateShader(shaderType));

    const char* shaderString = shaderSource.c_str();
    GLint shaderStringLength = static_cast<GLint>(shaderSource.length());

    WT_GL_ASSERT(glShaderSource(shaderHandle, 1, &shaderString, &shaderStringLength));
    WT_GL_ASSERT(glCompileShader(shaderHandle));

    GLint compileSuccess;
    WT_GL_ASSERT(glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess));
    if (GL_FALSE == compileSuccess)
    {
        GLchar messages[256];
        WT_GL_ASSERT(glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]));
        std::cout << "Error compiling shader: " << messages << std::endl;
    }

    return shaderHandle;
}

GLuint OpenGLESScanningEffectRenderingPluginModule::createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
    GLuint vertexShaderHandle = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShaderHandle = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    GLuint programHandle;
    WT_GL_ASSERT_AND_RETURN(programHandle, glCreateProgram());

    WT_GL_ASSERT(glAttachShader(programHandle, vertexShaderHandle));
    WT_GL_ASSERT(glAttachShader(programHandle, fragmentShaderHandle));
    WT_GL_ASSERT(glLinkProgram(programHandle));

    GLint linkSuccess;
    WT_GL_ASSERT(glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess));
    if (linkSuccess == GL_FALSE)
    {
        GLchar messages[256];
        WT_GL_ASSERT(glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]));
        std::cout << "Error linking program: " << messages << std::endl;
    }

    WT_GL_ASSERT(glDeleteShader(vertexShaderHandle));
    WT_GL_ASSERT(glDeleteShader(fragmentShaderHandle));

    return programHandle;
}

void OpenGLESScanningEffectRenderingPluginModule::createShaderProgram(const YUVFrameShaderSourceObject& shaderObject)
{
    _programHandle = createShaderProgram(shaderObject.getVertexShaderSource(), shaderObject.getFragmentShaderSource());

    WT_GL_ASSERT(glUseProgram(_programHandle));

    WT_GL_ASSERT_AND_RETURN(_positionAttributeLocation, glGetAttribLocation(_programHandle, shaderObject.getVertexPositionAttributeName().c_str()));
    WT_GL_ASSERT_AND_RETURN(_texCoordAttributeLocation, glGetAttribLocation(_programHandle, shaderObject.getTextureCoordinateAttributeName().c_str()));

    std::vector<std::string> uniformNames = shaderObject.getTextureUniformNames();
    std::size_t uniformNamesSize = uniformNames.size();
    for (int i = 0; i < uniformNamesSize; ++i)
    {
        WT_GL_ASSERT(glUniform1i(glGetUniformLocation(_programHandle, uniformNames[i].c_str()), i));
    }
}

void OpenGLESScanningEffectRenderingPluginModule::releaseShaderProgram()
{
    if (_scanlineShaderHandle) {
        WT_GL_ASSERT(glDeleteProgram(_scanlineShaderHandle));
        _scanlineShaderHandle = 0;
    }

    if (_fullscreenTextureShader) {
        WT_GL_ASSERT(glDeleteProgram(_fullscreenTextureShader));
        _fullscreenTextureShader = 0;
    }

    if (_augmentationShaderHandle) {
        WT_GL_ASSERT(glDeleteProgram(_augmentationShaderHandle));
        _augmentationShaderHandle = 0;
    }

    if (_programHandle) {
        WT_GL_ASSERT(glDeleteProgram(_programHandle));
        _programHandle = 0;
    }
}

void OpenGLESScanningEffectRenderingPluginModule::setVertexShaderUniforms(GLuint shaderHandle)
{
    GLint deviceOrientationLocation;
    WT_GL_ASSERT_AND_RETURN(deviceOrientationLocation, glGetUniformLocation(shaderHandle, "uModelMatrix"));
    WT_GL_ASSERT(glUniformMatrix4fv(deviceOrientationLocation, 1, GL_FALSE, _modelMatrix.get()));
}

void OpenGLESScanningEffectRenderingPluginModule::setFragmentShaderUniforms(float viewportWidth, float viewportHeight)
{
    GLint scanlineUniformLocation;
    WT_GL_ASSERT_AND_RETURN(scanlineUniformLocation, glGetUniformLocation(_scanlineShaderHandle, "uCameraFrameTexture"));
    WT_GL_ASSERT(glUniform1i(scanlineUniformLocation, 0));

    GLint scanlineUniformResolutionLocation;
    WT_GL_ASSERT_AND_RETURN(scanlineUniformResolutionLocation, glGetUniformLocation(_scanlineShaderHandle, "uViewportResolution"));
    WT_GL_ASSERT(glUniform2f(scanlineUniformResolutionLocation, viewportWidth, viewportHeight));

    currentTime = std::chrono::system_clock::now();
    std::chrono::duration<float> durationInSeconds = currentTime - startTime;
    GLint scanlineUniformTimeLocation;
    WT_GL_ASSERT_AND_RETURN(scanlineUniformTimeLocation, glGetUniformLocation(_scanlineShaderHandle, "uTimeInSeconds"));
    WT_GL_ASSERT(glUniform1f(scanlineUniformTimeLocation, durationInSeconds.count()));

    const float scanningAreaHeight = viewportHeight * 0.25;

    GLint scanlineUniformAreaHeight;
    WT_GL_ASSERT_AND_RETURN(scanlineUniformAreaHeight, glGetUniformLocation(_scanlineShaderHandle, "uScanningAreaHeightInPixels"));
    WT_GL_ASSERT(glUniform1f(scanlineUniformAreaHeight, scanningAreaHeight));
}

void OpenGLESScanningEffectRenderingPluginModule::setVertexAttributes(GLuint shaderHandle)
{
    GLuint vertexPositionAttributeLocation;
    WT_GL_ASSERT_AND_RETURN(vertexPositionAttributeLocation, glGetAttribLocation(shaderHandle, "vPosition"));
    WT_GL_ASSERT(glEnableVertexAttribArray(vertexPositionAttributeLocation));
    WT_GL_ASSERT(glVertexAttribPointer(vertexPositionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

    GLuint vertexTexCoordAttributeLocation;
    WT_GL_ASSERT_AND_RETURN(vertexTexCoordAttributeLocation, glGetAttribLocation(shaderHandle, "vTexCoords"));
    WT_GL_ASSERT(glEnableVertexAttribArray(vertexTexCoordAttributeLocation));
    WT_GL_ASSERT(glVertexAttribPointer(vertexTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(float) * 3)));
}

void OpenGLESScanningEffectRenderingPluginModule::createFrameTextures(GLsizei width, GLsizei height)
{
    WT_GL_ASSERT(glGenTextures(1, &_lumaTexture));
    WT_GL_ASSERT(glGenTextures(1, &_chromaTexture));

    WT_GL_ASSERT(glActiveTexture(GL_TEXTURE0));
    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _lumaTexture));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    WT_GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr));

    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _chromaTexture));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    WT_GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width / 2, height / 2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, nullptr));
}

void OpenGLESScanningEffectRenderingPluginModule::updateFrameTexturesData(GLsizei width, GLsizei height, const unsigned char* const frameData)
{
    WT_GL_ASSERT(glActiveTexture(GL_TEXTURE0));

    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _lumaTexture));
    WT_GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData));

    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _chromaTexture));
    WT_GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width / 2, height / 2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, frameData + width * height));
}

void OpenGLESScanningEffectRenderingPluginModule::bindFrameTextures()
{
    WT_GL_ASSERT(glActiveTexture(GL_TEXTURE0));
    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _lumaTexture));

    WT_GL_ASSERT(glActiveTexture(GL_TEXTURE1));
    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _chromaTexture));
}

void OpenGLESScanningEffectRenderingPluginModule::releaseFrameTextures()
{
    if (_lumaTexture) {
        WT_GL_ASSERT(glDeleteTextures(1, &_lumaTexture));
        _lumaTexture = 0;
    }

    if (_chromaTexture) {
        WT_GL_ASSERT(glDeleteTextures(1, &_chromaTexture));
        _chromaTexture = 0;
    }
}

void OpenGLESScanningEffectRenderingPluginModule::createFrameBufferObject(GLsizei width, GLsizei height)
{
    WT_GL_ASSERT(glGenFramebuffers(1, &_frameBufferObject));
    WT_GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferObject));

    WT_GL_ASSERT(glGenTextures(1, &_frameBufferTexture));

    WT_GL_ASSERT(glActiveTexture(GL_TEXTURE0));
    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, _frameBufferTexture));

    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    WT_GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    WT_GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));

    WT_GL_ASSERT(glBindTexture(GL_TEXTURE_2D, 0));

    WT_GL_ASSERT(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _frameBufferTexture, 0));

    GLenum framebufferStatus;
    WT_GL_ASSERT_AND_RETURN(framebufferStatus, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer incomplete!" << std::endl;
    }
}

void OpenGLESScanningEffectRenderingPluginModule::bindFrameBufferObject()
{
    WT_GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferObject));
}

void OpenGLESScanningEffectRenderingPluginModule::unbindFrameBufferObject()
{
    WT_GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _defaultFrameBuffer));
}

void OpenGLESScanningEffectRenderingPluginModule::releaseFramebufferObject()
{
    if (_frameBufferTexture) {
        WT_GL_ASSERT(glDeleteTextures(1, &_frameBufferTexture));
        _frameBufferTexture = 0;
    }

    if (_frameBufferObject) {
        WT_GL_ASSERT(glDeleteTextures(1, &_frameBufferObject));
        _frameBufferObject = 0;
    }
}

void OpenGLESScanningEffectRenderingPluginModule::updateMatrices() {
    wikitude::sdk::Matrix4 rotationMatrix;
    rotationMatrix.rotateZ(-_cameraToSurfaceAngle);
    _orientationMatrix = rotationMatrix;

    wikitude::sdk::Matrix4 scaleMatrix;
    scaleMatrix.scale(_cameraToSurfaceScaling.x, _cameraToSurfaceScaling.y, 1.0f);

    _modelMatrix = scaleMatrix * _orientationMatrix * _fboCorrectionMatrix;

    const float fov = 60.0f;
    const float aspect = 640.0f / 480.0f;
    _projectionMatrix.perspective(fov, aspect, 0.01f, 500.0f);
}
