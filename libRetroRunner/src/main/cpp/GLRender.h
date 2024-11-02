//
// Created by aidoo on 2024/10/31.
//

#ifndef LIBRETROSD_GLRENDER_H
#define LIBRETROSD_GLRENDER_H
#include <GLES2/gl2.h>

class GLRender {


public:
    GLuint createGlProgram();
    bool checkGlError(const char *op);
};


#endif //LIBRETROSD_GLRENDER_H
