//
// Created by aidoo on 2024/10/31.
//

#ifndef _APP_CONTEXT_H
#define _APP_CONTEXT_H

#include <string>
#include "rr_types.h"
#include "core.h"
#include "environment.h"
#include "utils/threadsafe_queue.hpp"

namespace libRetroRunner {

    enum AppState {
        UNKNOWN = 0,
        INIT = 1 << 1,
        RUNNING = 1 << 2,
        PAUSE = 1 << 3,
        STOP = 1 << 4
    };

    class AppContext {


    public:
        AppContext();

        ~AppContext();

        void SetFiles(const std::string &rom_path, const std::string &core_path, const std::string &system_path,
                      const std::string &save_path);

        void SetVariable(const std::string &key, const std::string &value);

        void Start();

        void Pause();

        void Resume();

        void Reset();

        void Stop();

        AppState GetState();

        void ThreadLoop();

    public:
        static AppContext *Instance() {
            return instance.get();
        };


    private:
        void processCommand();

        void loadCore();

    private:

        std::string rom_path;
        std::string core_path;
        std::string system_path;
        std::string save_path;

        AppState state;
        threadsafe_queue<int> commands;   //等待处理的命令

        std::unique_ptr<Core> core;
        std::unique_ptr<Environment> environment;

        static std::unique_ptr<AppContext> instance;


        friend class RetroCallbacks;
    };

    class RetroCallbacks {

    public:
        static void libretro_callback_hw_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch);

        static bool libretro_callback_set_environment(unsigned cmd, void *data);

        static void libretro_callback_audio_sample(int16_t left, int16_t right);

        static size_t libretro_callback_audio_sample_batch(const int16_t *data, size_t frames);

        static void libretro_callback_input_poll(void);

        static int16_t libretro_callback_input_state(unsigned port, unsigned device, unsigned index, unsigned id);
    };

}


#endif
