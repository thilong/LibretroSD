//
// Created by aidoo on 2024/10/31.
//

#ifndef _APP_H
#define _APP_H

#include <string>
#include "rr_types.h"
#include "core.h"
#include "environment.h"
#include "video.h"
#include "input.h"

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

        void SetFiles(const std::string &romPath, const std::string &corePath, const std::string &systemPath,
                      const std::string &savePath);

        void SetVariable(const std::string &key, const std::string &value);

        void AddCommand(int command);

        void Start();

        void Pause();

        void Resume();

        void Reset();

        void Stop();

        int GetState();

        void ThreadLoop();

    public:
        static AppContext *NewInstance();

        static AppContext *Instance() {
            return instance.get();
        };

        Environment *GetEnvironment() const;

        VideoContext *GetVideo();

        Input* GetInput();

    private: //以下函数只在模拟线程中调用
        void processCommand();

        void cmdLoadCore();

        void cmdLoadContent();

        void cmdInitVideo();


    private:

        std::string rom_path;
        std::string core_path;
        std::string system_path;
        std::string save_path;

        int state;
        threadsafe_queue<int> commands;   //等待处理的命令

        std::unique_ptr<Core> core;
        std::unique_ptr<Environment> environment;
        std::unique_ptr<VideoContext> video;
        std::unique_ptr<Input> input;

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
