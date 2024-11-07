//
// Created by aidoo on 2024/11/4.
//

#ifndef _VIDEO_H
#define _VIDEO_H

#include <memory>

#include <jni.h>


namespace libRetroRunner {


    class VideoContext {
    public:
        VideoContext();

        virtual ~VideoContext();

        static std::unique_ptr<VideoContext> NewInstance();

        virtual void Init() = 0;

        virtual void Destroy() = 0;

        virtual void DrawFrame() = 0;

        virtual void OnFrameArrive(const void *data, unsigned int width, unsigned int height, size_t pitch) = 0;

        virtual void OnGameGeometryChanged() = 0;

        virtual void SetSurface(void *envObj, void *surfaceObj) = 0;

        virtual void SetSurfaceSize(unsigned width, unsigned height) = 0;

        virtual unsigned int GetCurrentFramebuffer() = 0;
    };
}
#endif