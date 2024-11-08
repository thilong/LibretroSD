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

        void SetPixelFormat(unsigned int format);

        void Create(bool includeDepth, bool includeStencil);

        void Destroy();

        GLuint GetFrameBuffer();

        GLuint GetTexture();

    public:
        inline unsigned GetWidth() const {
            return width;
        }

        inline unsigned GetHeight() const {
            return height;
        }

    private:
        unsigned int pixel_format;

        bool linear;
        bool depth;

        GLuint frame_buffer = 0;
        GLuint texture_id = 0;
        GLuint depth_buffer = 0;
        unsigned width = 0;
        unsigned height = 0;

        uint32_t *buffer = nullptr;

    };
}

#endif
