//
// Created by aidoo on 2024/10/31.
//

#include "GLRender.h"
#include "rr_log.h"

GLuint GLRender::createGlProgram() {
    const char *vertexShaderCode = R"delimiter(
            precision mediump float;
            attribute vec4 a_Position;
            void main() {
                gl_Position = a_Position;
            }
            )delimiter";

    const char *fragmentShaderCode = R"delimiter(
            precision mediump float;
            void main() {
                gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
            }
            )delimiter";
    GLuint programId = glCreateProgram();
    if (programId == 0) {
        LOGE("[GL] Could not create program");
        return 0;
    }
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    glCompileShader(vertexShader);

    if(!checkGlError("glCompileShader")) {
        return -1;
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    glCompileShader(fragmentShader);
    if(!checkGlError("glCompileShader")) {
        return -1;
    }
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);
    return programId;
}

bool GLRender::checkGlError(const char *op) {
    if (glGetError() != GL_NO_ERROR) {
        LOGE("[GL] error: %s", op);
        return false;
    }
    return true;
}
