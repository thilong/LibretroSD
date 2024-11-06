//
// Created by aidoo on 2024/11/5.
//

#ifndef _HW_FRAME_BUFFER_GL_H
#define _HW_FRAME_BUFFER_GL_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


namespace libRetroRunner {
    class GLHardwareFrameBuffer {
    public:
        GLHardwareFrameBuffer();

        ~GLHardwareFrameBuffer();

        void SetSize(int w, int h);

        void SetLinear(bool flag);

        void Create(bool includeDepth, bool includeStencil);

        void Destroy();

        GLuint GetFrameBuffer();

        GLuint GetTexture();

    public:
        inline unsigned GetWidth() {
            return width;
        }

        inline unsigned GetHeight() {
            return height;
        }

    private:
        bool linear;
        bool depth;

        GLuint frame_buffer = 0;
        GLuint texture_id = 0;
        GLuint depth_buffer = 0;
        unsigned width = 0;
        unsigned height = 0;

    };
}

#endif
