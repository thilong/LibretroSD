//
// Created by aidoo on 2024/11/5.
//
#include "shader_pass_gl.h"
#include "../rr_log.h"

#define SP_LOGD(...) LOGD("[VIDEO] " __VA_ARGS__)
#define SP_LOGW(...) LOGW("[VIDEO] " __VA_ARGS__)
#define SP_LOGE(...) LOGE("[VIDEO] " __VA_ARGS__)
#define SP_LOGI(...) LOGI("[VIDEO] " __VA_ARGS__)


namespace libRetroRunner {

    const char *default_vertex_shader = R"delimiter(
            attribute vec4 a_position;  // position of the quad
            attribute vec2 a_texCoord;  // texture coordinates
            varying vec2 v_texCoord;    // pass texture coords to fragment shader

            void main() {
                gl_Position = a_position;
                v_texCoord = a_texCoord;
            }
            )delimiter";
    const char *default_fragment_shader = R"delimiter(
            precision mediump float;
            uniform sampler2D u_texture;
            varying vec2 v_texCoord;

            void main() {
                gl_FragColor = texture2D(u_texture, v_texCoord);
            }
            )delimiter";

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
                        SP_LOGE("Could not compile shader %d:\n%s\n",
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

    GLShaderPass::GLShaderPass(const char *vertexShaderCode, char *fragmentShaderCode) {
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
                        SP_LOGE("Could not link program:\n%s\n", buf);
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
        }
    }

    GLShaderPass::~GLShaderPass() {
        Destroy();
    }

    void GLShaderPass::Destroy() {
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
        frameBuffer = std::make_unique<GLHardwareFrameBuffer>();
        frameBuffer->SetSize(width, height);
        frameBuffer->SetLinear(linear);
        frameBuffer->Create(includeDepth, includeStencil);
        GL_CHECK("GLShaderPass::CreateFrameBuffer");
    }

    //如果textureId大于0，则表示把textureId的内容绘制到frameBuffer上
    //否则，直接绘制frameBuffer的内容到屏幕上
    void GLShaderPass::DrawTexture(GLuint textureId, unsigned viewWidth, unsigned viewHeight) {
        bool renderToScreen = textureId == 0;

        glUseProgram(programId);

        if (viewWidth == 0) viewWidth = frameBuffer->GetWidth();
        if (viewHeight == 0) viewHeight = frameBuffer->GetHeight();
        glViewport(0, 0, viewWidth, viewHeight);


        //set position
        glEnableVertexAttribArray(attr_position);
        glVertexAttribPointer(attr_position, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);

        //set vertex
        glEnableVertexAttribArray(attr_coordinate);
        if (renderToScreen) {
            glVertexAttribPointer(attr_coordinate, 2, GL_FLOAT, GL_FALSE, 0, gFlipTextureCoords);
        } else {
            glVertexAttribPointer(attr_coordinate, 2, GL_FLOAT, GL_FALSE, 0, gTextureCoords);
        }

        //set texture
        glActiveTexture(GL_TEXTURE0);
        if (renderToScreen) {
            glBindTexture(GL_TEXTURE_2D, frameBuffer->GetTexture());
            glUniform1i(attr_texture, 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glUniform1i(attr_texture, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->GetFrameBuffer());
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

        glDisableVertexAttribArray(attr_position);
        glDisableVertexAttribArray(attr_coordinate);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


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
                        SP_LOGE("Could not link program:\n%s\n", buf);
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


