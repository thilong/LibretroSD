//
// Created by aidoo on 2024/11/5.
//

#ifndef _SHADER_PASS_GL_H
#define _SHADER_PASS_GL_H

#include <GLES2/gl2.h>
#include <string>
#include "hw_frame_buffer_gl.h"

namespace libRetroRunner {
    class GLShaderPass {
    public:

        GLShaderPass(const char *vertexShaderCode, char *fragmentShaderCode);

        ~GLShaderPass();

        void Destroy();

        void CreateFrameBuffer(int width, int height, bool linear, bool includeDepth, bool includeStencil);

        void DrawTexture(GLuint textureId , unsigned viewWidth = 0, unsigned viewHeight = 0);

        void DrawToScreen(unsigned viewWidth, unsigned viewHeight);
    public:
        inline void SetPixelFormat(int format){
            pixelFormat = format;
        }

        //以下为shader相关
        inline GLuint GetProgramId() {
            return programId;
        }

        inline GLuint GetVertexShader() {
            return vertexShader;
        }

        inline GLuint GetFragmentShader() {
            return fragmentShader;
        }

        inline GLuint GetAttrPosition() {
            return attr_position;
        }

        inline GLuint GetAttrCoordinate() {
            return attr_coordinate;
        }

        inline GLuint GetAttrTexture() {
            return attr_texture;
        }
        //以下为framebuffer相关

        inline GLuint GetFrameBuffer() {
            if (frameBuffer == nullptr) {
                return 0;
            }
            return frameBuffer->GetFrameBuffer();
        }

        inline GLuint GetTexture() {
            if (frameBuffer == nullptr) {
                return 0;
            }
            return frameBuffer->GetTexture();
        }

        inline unsigned GetWidth() {
            if (frameBuffer == nullptr) {
                return 0;
            }
            return frameBuffer->GetWidth();
        }

        inline unsigned GetHeight() {
            if (frameBuffer == nullptr) {
                return 0;
            }
            return frameBuffer->GetHeight();
        }

    private:
        std::unique_ptr<GLHardwareFrameBuffer> frameBuffer;

        unsigned int pixelFormat;

        GLuint programId = 0;
        GLuint vertexShader = 0;
        GLuint fragmentShader = 0;

        GLuint attr_position;
        GLuint attr_coordinate;
        GLuint attr_texture;
    };


    //for test
    class SoftwareRender {
    public:
        SoftwareRender();

        ~SoftwareRender();

        void Render(unsigned viewportWidth, unsigned viewportHeight, unsigned int textureId);

    private:
        GLuint programId = 0;
        GLuint vertexShader = 0;
        GLuint fragmentShader = 0;

        GLuint attr_position;
        GLuint attr_coordinate;
        GLuint attr_texture;
    };
}
#endif
