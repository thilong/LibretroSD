//
// Created by aidoo on 2024/11/5.
//

#ifndef _SOFTWARE_TEXTURE_BUFFER_H
#define _SOFTWARE_TEXTURE_BUFFER_H


namespace libRetroRunner {
    class SoftwareTextureBuffer {
    public :
        SoftwareTextureBuffer() = default;

        ~SoftwareTextureBuffer();

        void Create(unsigned int width, unsigned int height);

        void WriteTextureData(const void *data, unsigned int width, unsigned int height, int pixelFormat);

        void Destroy();

    public:
        inline unsigned int GetTexture() {
            return textureId;
        }

        inline unsigned int GetWidth() {
            return texture_width;
        }

        inline unsigned int GetHeight() {
            return texture_height;
        }

    private:
        unsigned int texture_width;
        unsigned int texture_height;
        unsigned char *buffer;
        unsigned int textureId;
    };



}
#endif
