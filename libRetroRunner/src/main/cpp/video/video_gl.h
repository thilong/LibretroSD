//
// Created by aidoo on 2024/11/5.
//

#ifndef _VIDEO_GL_H
#define _VIDEO_GL_H

#include <vector>
#include <memory>

#include <EGL/egl.h>
#include "../video.h"
#include "shader_pass_gl.h"
#include "software_texture_buffer.h"

namespace libRetroRunner {


    class GLVideoContext : public VideoContext {

    public:
        GLVideoContext();

        ~GLVideoContext() override;


        void Init() override;
        void Reinit() override;

        void Destroy() override;

        void SetSurface(void *envObj, void *surfaceObj) override;

        void SetSurfaceSize(unsigned int width, unsigned int height) override;

        void OnGameGeometryChanged() override;

        void Prepare() override;

        void OnFrameArrive(const void *data, unsigned int width, unsigned int height, size_t pitch) override;

        void DrawFrame() override;

        unsigned int GetCurrentFramebuffer() override;

    private:
        bool eglContextMakeCurrent();
        void makeBackBuffer();
    private:
        int current_width;
        int current_height;

        uint64_t frame_count;

        //游戏渲染目标
        std::unique_ptr<SoftwareRender> gameRender;
        std::unique_ptr<SoftwareTextureBuffer> gameTexture;  //用于非硬件加速的模拟核心渲染
        std::unique_ptr<GLShaderPass> gamePass;

        std::vector<std::unique_ptr<GLShaderPass> > shaderPasses;

        EGLDisplay eglDisplay;
        EGLSurface eglSurface;
        EGLContext eglContext;
        int internalPixelFormat;

        bool is_ready;
    };
}
#endif
