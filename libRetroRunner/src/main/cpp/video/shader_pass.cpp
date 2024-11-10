//
// Created by aidoo on 2024/11/5.
//
#include "shader_pass.h"
#include "../rr_log.h"
#include "../libretro-common/include/libretro.h"

#define LOGD_SP(...) LOGD("[VIDEO]:[SHADERPASS] " __VA_ARGS__)
#define LOGW_SP(...) LOGW("[VIDEO]:[SHADERPASS] " __VA_ARGS__)
#define LOGE_SP(...) LOGE("[VIDEO]:[SHADERPASS] " __VA_ARGS__)
#define LOGI_SP(...) LOGI("[VIDEO]:[SHADERPASS] " __VA_ARGS__)

//静态变量与公共方法
namespace libRetroRunner {

    const char *default_vertex_shader = R"delimiter(
            #extension GL_OES_standard_derivatives : enable

            attribute vec4 a_position;  // position of the quad
            attribute vec2 a_texCoord;  // texture coordinates
            varying vec2 v_texCoord;    // pass texture coords to fragment shader

            void main() {
                gl_Position = a_position;
                v_texCoord = a_texCoord;
            }
            )delimiter";
    const char *default_fragment_shader = R"delimiter(
            #extension GL_OES_standard_derivatives : enable

            precision mediump float;
            uniform sampler2D u_texture;
            varying vec2 v_texCoord;

            void main() {
                gl_FragColor = texture2D(u_texture, v_texCoord);
            }
            )delimiter";

    GLfloat gBufferObjectData[24] = {
            -1.0F, -1.0F, 0.0F, 0.0F,  //左下
            -1.0F, +1.0F, 0.0F, 1.0F, //左上
            +1.0F, +1.0F, 1.0F, 1.0F, //右上
            +1.0F, -1.0F, 1.0F, 0.0F, //右下

            -1.0F, -1.0F, 0.0F, 0.0F, //左下
            -1.0F, +1.0F, 0.0F, 1.0F, //左上
    };

    GLfloat gFlipBufferObjectData[24] = {
            -1.0F, -1.0F,  0.0F, 1.0F,
            -1.0F, +1.0F,  0.0F, 0.0F,
            +1.0F, +1.0F,  1.0F, 0.0F,
            +1.0F, -1.0F,  1.0F, 1.0F,

            -1.0F, -1.0F,  0.0F, 1.0F,
            -1.0F, +1.0F,  0.0F, 0.0F,
    };

    GLfloat gTriangleVertices[12] = {
            -1.0F, -1.0F,   //左下
            -1.0F, +1.0F,   //左上
            +1.0F, +1.0F,   //右上
            +1.0F, -1.0F,   //右下

            -1.0F, -1.0F,   //左下
            -1.0F, +1.0F,   //左上

    };

    GLfloat gFlipTextureCoords[12] = {
            0.0F, 1.0F,     //左上
            0.0F, 0.0F,      //左下

            1.0F, 0.0F,      //右下
            1.0F, 1.0F,     //右上

            0.0F, 1.0F,     //左上
            0.0F, 0.0F,      //左下
    };

    GLfloat gTextureCoords[12] = {
            0.0F, 0.0F,      //左下
            0.0F, 1.0F,     //左上
            1.0F, 1.0F,     //右上
            1.0F, 0.0F,      //右下

            0.0F, 0.0F,      //左下
            0.0F, 1.0F,     //左上

    };


    static GLuint _loadShader(GLenum shaderType, const char *source) {
        GLuint shader = glCreateShader(shaderType);
        if (shader) {
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char *buf = (char *) malloc(infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                        LOGE_SP("Could not compile shader %d:\n%s\n",
                                shaderType, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
        return shader;
    }

    static GLuint _createFullViewFBOs() {
        GLuint buffer = 0;
        glGenBuffers(1, &buffer);
        if (buffer == 0) {
            LOGE_SP("create buffer failed.");
            return 0;
        }
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gBufferObjectData), gBufferObjectData, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return buffer;
    }
}

//GLShaderPass定义
namespace libRetroRunner {
    GLShaderPass::GLShaderPass(const char *vertexShaderCode, char *fragmentShaderCode) {
        pixelFormat = RETRO_PIXEL_FORMAT_UNKNOWN;
        const char *finalVertexShaderCode = vertexShaderCode ? vertexShaderCode : default_vertex_shader;
        const char *finalFragmentShaderCode = fragmentShaderCode ? fragmentShaderCode : default_fragment_shader;

        vertexShader = _loadShader(GL_VERTEX_SHADER, finalVertexShaderCode);
        if (!vertexShader) {
            return;
        }

        fragmentShader = _loadShader(GL_FRAGMENT_SHADER, finalFragmentShaderCode);
        if (!fragmentShader) {
            return;
        }

        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, nullptr, buf);
                        LOGE_SP("Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }

        if (program) {
            programId = program;
            attr_position = glGetAttribLocation(program, "a_position");
            attr_coordinate = glGetAttribLocation(program, "a_texCoord");
            attr_texture = glGetUniformLocation(program, "u_texture");
            LOGD_SP("shader pass created, program id: %d", programId);
        }
    }

    GLShaderPass::~GLShaderPass() {
        Destroy();
    }

    void GLShaderPass::Destroy() {
        if (vertexBuffer) {
            glDeleteBuffers(1, &vertexBuffer);
            vertexBuffer = 0;
        }
        if (programId) {
            glDeleteProgram(programId);
            programId = 0;
        }
        if (vertexShader) {
            glDeleteShader(vertexShader);
            vertexShader = 0;
        }
        if (fragmentShader) {
            glDeleteShader(fragmentShader);
            fragmentShader = 0;
        }

    }

    void GLShaderPass::CreateFrameBuffer(int width, int height, bool linear, bool includeDepth, bool includeStencil) {
        if (pixelFormat == RETRO_PIXEL_FORMAT_UNKNOWN) {
            LOGE_SP("pixel format is unknow, can't create frame buffer for %d", pixelFormat);
            return;
        }
        if (frameBuffer != nullptr && width == frameBuffer->GetWidth() && height == frameBuffer->GetHeight()) {
            LOGW_SP("frame buffer not change, reuse it.");
            return;
        }
        frameBuffer = std::make_unique<GLFrameBufferObject>();
        frameBuffer->SetSize(width, height);
        frameBuffer->SetLinear(linear);
        frameBuffer->Create(includeDepth, includeStencil);
        GL_CHECK("GLShaderPass::CreateFrameBuffer");

        vertexBuffer = _createFullViewFBOs();
    }

    void GLShaderPass::DrawToScreen(unsigned int viewWidth, unsigned int viewHeight) {
        DrawTexture(0, viewWidth, viewHeight);
    }

    //如果textureId大于0，则表示把textureId的内容绘制到frameBuffer上
    //否则，直接绘制frameBuffer的内容到屏幕上
    //这里应该添加大小，位置，旋转等控制
    void GLShaderPass::DrawTexture(GLuint textureId, unsigned viewWidth, unsigned viewHeight) {
        bool renderToScreen = textureId == 0;
        if (!renderToScreen)   //如果不是渲染到屏幕，则绘制到当前这个frameBuffer上
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->GetFrameBuffer());
        else
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(programId);

        if (viewWidth == 0) viewWidth = frameBuffer->GetWidth();
        if (viewHeight == 0) viewHeight = frameBuffer->GetHeight();
        glViewport(0, 0, viewWidth, viewHeight);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        //set position
        glEnableVertexAttribArray(attr_position);
        //glVertexAttribPointer(attr_position, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
        glVertexAttribPointer(attr_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

        //set vertex
        glEnableVertexAttribArray(attr_coordinate);
        //如果是渲染到屏幕，因为opengl(左下为原点)与libretro(左上为原点)的图像坐标系不一样的原因，需要翻转图片
        //glVertexAttribPointer(attr_coordinate, 2, GL_FLOAT, GL_FALSE, 0, renderToScreen ? gFlipTextureCoords : gTextureCoords);
        glVertexAttribPointer(attr_coordinate, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));
        //set texture
        glActiveTexture(GL_TEXTURE0);
        if (renderToScreen) {
            glBindTexture(GL_TEXTURE_2D, frameBuffer->GetTexture());
            glUniform1i(attr_texture, 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(attr_texture, 0);
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

        glDisableVertexAttribArray(attr_position);
        glDisableVertexAttribArray(attr_coordinate);

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

//SoftwareRender定义，会被弃用
namespace libRetroRunner {
    //以下SoftwareRender为测试代码，会被弃用

    SoftwareRender::SoftwareRender() {
        vertexShader = _loadShader(GL_VERTEX_SHADER, default_vertex_shader);
        if (!vertexShader) {
            return;
        }

        fragmentShader = _loadShader(GL_FRAGMENT_SHADER, default_fragment_shader);
        if (!fragmentShader) {
            return;
        }

        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, nullptr, buf);
                        LOGE_SP("Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }

        if (program) {
            programId = program;
            attr_position = glGetAttribLocation(program, "a_position");
            attr_coordinate = glGetAttribLocation(program, "a_texCoord");
            attr_texture = glGetUniformLocation(program, "u_texture");
            LOGD_SP("software renderer created, program id: %d", programId);
        }
    }

    SoftwareRender::~SoftwareRender() {
        if (vertexShader) {
            glDeleteShader(vertexShader);
            vertexShader = 0;
        }
        if (fragmentShader) {
            glDeleteShader(fragmentShader);
            fragmentShader = 0;
        }
        if (programId) {
            glDeleteProgram(programId);
            programId = 0;
        }
    }

    void SoftwareRender::Render(unsigned int viewportWidth, unsigned int viewportHeight, unsigned int textureId) {
        glUseProgram(programId);


        glViewport(0, 0, viewportWidth, viewportHeight);

        //set position
        glEnableVertexAttribArray(attr_position);
        glVertexAttribPointer(attr_position, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);

        //set vertex
        glEnableVertexAttribArray(attr_coordinate);
        glVertexAttribPointer(attr_coordinate, 2, GL_FLOAT, GL_FALSE, 0, gTextureCoords);


        //set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(attr_texture, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

        glDisableVertexAttribArray(attr_position);
        glDisableVertexAttribArray(attr_coordinate);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


