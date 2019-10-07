//
//  YUVFrameShaderSourceObject.cpp
//  FoundationSDK
//
//  Created by Daniel Guttenberg on 14/03/16.
//  Copyright © 2016 Wikitude. All rights reserved.
//

#include "YUVFrameShaderSourceObject.hpp"


YUVFrameShaderSourceObject::YUVFrameShaderSourceObject()
{}

YUVFrameShaderSourceObject::~YUVFrameShaderSourceObject()
{}

std::string YUVFrameShaderSourceObject::getVertexShaderSource() const {
    std::string vertexShaderSource = "  \
    attribute vec4 Position; \
    \
    attribute vec2 TexCoordIn; \
    varying vec2 TexCoordOut; \
    \
    void main(void) { \
    gl_Position = Position; \
    TexCoordOut = TexCoordIn; \
    }";

    return vertexShaderSource;
}

std::string YUVFrameShaderSourceObject::getFragmentShaderSource() const {
    std::string fragmentShaderSource = "\
    \
    varying lowp vec2 TexCoordOut; \
    uniform sampler2D texture_y; \
    uniform sampler2D texture_uv; \
    \
    void main() \
    { \
    mediump vec3 yuv; \
    lowp vec3 rgb; \
    \
    yuv.x = texture2D(texture_y, TexCoordOut).r; \
    yuv.yz = texture2D(texture_uv, TexCoordOut).ar - vec2(0.5, 0.5); \
    \
    rgb = mat3(       1,       1,      1, \
    0, -.18732, 1.8556, \
    1.57481, -.46813,      0) * yuv; \
    \
    gl_FragColor = vec4(rgb, 1); \
    }";

    return fragmentShaderSource;
}

std::string YUVFrameShaderSourceObject::getVertexPositionAttributeName() const {
    return "Position";
}

std::string YUVFrameShaderSourceObject::getTextureCoordinateAttributeName() const {
    return "TexCoordIn";
}

std::vector<std::string> YUVFrameShaderSourceObject::getTextureUniformNames() const {
    std::vector<std::string> uniforms;
    uniforms.reserve(2);
    uniforms.push_back("texture_y");
    uniforms.push_back("texture_uv");
    return uniforms;
}
