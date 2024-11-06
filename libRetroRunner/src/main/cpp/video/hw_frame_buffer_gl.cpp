//
// Created by aidoo on 2024/11/5.
//
#include "hw_frame_buffer_gl.h"
#include <GLES3/gl3.h>
#include "../rr_log.h"
#include <stdexcept>

#define FBOLOGD(...) LOGD("[VIDEO] " __VA_ARGS__)
#define FBOLOGW(...) LOGW("[VIDEO] " __VA_ARGS__)
#define FBOLOGE(...) LOGE("[VIDEO] " __VA_ARGS__)
#define FBOLOGI(...) LOGI("[VIDEO] " __VA_ARGS__)


namespace libRetroRunner {
    GLHardwareFrameBuffer::GLHardwareFrameBuffer() {
        frame_buffer = 0;
        texture_id = 0;
        depth = 0;
        width = 0;
        height = 0;
    }

    GLHardwareFrameBuffer::~GLHardwareFrameBuffer() {
        Destroy();
    }

    void GLHardwareFrameBuffer::Create(bool includeDepth, bool includeStencil) {
        Destroy();
        depth = includeDepth;

        //create texture for fbo
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

        GL_CHECK("create texture storage");

        glGenFramebuffers(1, &frame_buffer);
        GL_CHECK("gen frame buffer.");

        if (depth) {
            glGenRenderbuffers(1, &depth_buffer);

            GL_CHECK("gen render buffer.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

        GL_CHECK("gen frame buffer texture.")

        if (depth) {
            glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
            glRenderbufferStorage(
                    GL_RENDERBUFFER,
                    includeStencil ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16,
                    width,
                    height
            );
            GL_CHECK("gen render buffer storage.")
            glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_RENDERBUFFER,
                    depth_buffer
            );
            if (includeStencil) {
                glFramebufferRenderbuffer(
                        GL_FRAMEBUFFER,
                        GL_DEPTH_STENCIL_ATTACHMENT,
                        GL_RENDERBUFFER,
                        depth_buffer
                );
            }
            GL_CHECK("bind render buffer to frame buffer.");
        }

        int frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            FBOLOGE("Error creating framebuffer %d not complete, status: %d, error: %d, %s", frame_buffer, frameBufferStatus, glGetError(), glGetString(glGetError()));
            throw std::runtime_error("Cannot create framebuffer");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        FBOLOGD("new frame buffer created: %d", frame_buffer);
    }

    void GLHardwareFrameBuffer::Destroy() {
        if (depth_buffer > 0) {
            glDeleteRenderbuffers(1, &depth_buffer);
            depth_buffer = 0;
            depth = false;
        }
        if (texture_id > 0) {
            glDeleteTextures(1, &texture_id);
            texture_id = 0;
        }
        if (frame_buffer > 0) {
            glDeleteFramebuffers(1, &frame_buffer);
            frame_buffer = 0;
        }
    }

    void GLHardwareFrameBuffer::SetSize(int w, int h) {
        width = w;
        height = h;
    }

    void GLHardwareFrameBuffer::SetLinear(bool flag) {
        this->linear = flag;
    }

    GLuint GLHardwareFrameBuffer::GetFrameBuffer() {
        return frame_buffer;
    }

    GLuint GLHardwareFrameBuffer::GetTexture() {
        return texture_id;
    }
}

