//
//  YUVFrameShaderSourceObject.hpp
//
//  Copyright © 2016 Wikitude. All rights reserved.
//

#ifndef YUVFrameShaderSourceObject_hpp
#define YUVFrameShaderSourceObject_hpp

#include <string>
#include <vector>


class YUVFrameShaderSourceObject {
public:
    YUVFrameShaderSourceObject();
    virtual ~YUVFrameShaderSourceObject();

    virtual std::string getVertexShaderSource() const;
    virtual std::string getFragmentShaderSource() const;

    virtual std::string getVertexPositionAttributeName() const;
    virtual std::string getTextureCoordinateAttributeName() const;

    virtual std::vector<std::string> getTextureUniformNames() const;
};

#endif /* YUVFrameShaderSourceObject_hpp */
