//
// Created by aidoo on 2024/11/1.
//

#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include "rr_types.h"
#include <map>
#include <unordered_map>

#include "libretro-common/include/libretro.h"
#include "video.h"

namespace libRetroRunner {

    class Environment {
        friend class AppContext;
        friend class VideoContext;
        friend class GLVideoContext;

    public:
        Environment();

        ~Environment();

        bool HandleCoreCallback(unsigned int cmd, void *data);

        void UpdateVariable(const std::string &key, const std::string &value, bool notifyCore = false);

        void SetSystemPath(const std::string &path);

        void SetSavePath(const std::string &path);

        static uintptr_t CoreCallbackGetCurrentFrameBuffer();

        static bool CoreCallbackSetRumbleState(unsigned port, enum retro_rumble_effect effect, uint16_t strength);

        static void CoreCallbackLog(enum retro_log_level level, const char *fmt, ...);

        static void CoreCallbackNotifyAudioState(bool active, unsigned occupancy, bool underrun_likely);

        static retro_proc_address_t CoreCallbackGetProcAddress(const char* sym);

    private:
        bool cmdSetPixelFormat(void *data);

        bool cmdSetHardwareRender(void *data);

        bool cmdGetVariable(void *data);

        bool cmdSetVariables(void *data);

        bool cmdSetVariable(void *data);

        bool cmdSetGeometry(void *data);

        bool cmdGetCurrentFrameBuffer(void *data);

    private:
        std::unordered_map<std::string, struct Variable> variables;
        bool variablesChanged = false;
        std::string systemPath;
        std::string savePath;

        unsigned int language = RETRO_LANGUAGE_ENGLISH;
        bool audioEnabled = true;
        bool videoEnabled = true;
        bool fastForwarding = false;
        unsigned int maxUserCount = 4;

        bool coreSupportNoGame;

        int pixelFormat;

        int renderContextType;
        unsigned int renderMajorVersion;
        unsigned int renderMinorVersion;

        bool renderUseHWAcceleration;
        bool renderUseDepth;
        bool renderUseStencil;

        retro_hw_context_reset_t renderContextReset;
        retro_hw_context_reset_t renderContextDestroy;

        unsigned int gameGeometryHeight;
        unsigned int gameGeometryWidth;
        float gameGeometryAspectRatio;
        bool gameGeometryUpdated;

        retro_disk_control_callback *diskControllerCallback;
    };

}

#endif
