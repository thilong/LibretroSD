//
// Created by aidoo on 2024/10/31.
//

#include <pthread.h>
#include <unistd.h>

#include "app.h"
#include "rr_log.h"
#include "utils/utils.h"

#define APPLOGD(...) LOGD("[RUNNER] " __VA_ARGS__)
#define APPLOGW(...) LOGW("[RUNNER] " __VA_ARGS__)
#define APPLOGE(...) LOGE("[RUNNER] " __VA_ARGS__)
#define APPLOGI(...) LOGI("[RUNNER] " __VA_ARGS__)

namespace libRetroRunner {
    extern "C" JavaVM *gVm;

    static void *appThread(void *args) {
        ((AppContext *) args)->ThreadLoop();
        return nullptr;
    }

    std::unique_ptr<AppContext> AppContext::instance = nullptr;

    AppContext *AppContext::NewInstance() {
        instance = std::make_unique<AppContext>();
        return instance.get();
    }

    Environment *AppContext::GetEnvironment() const {
        return environment.get();
    }

    AppContext::AppContext() {
        state = AppState::INIT;
    }

    AppContext::~AppContext() {
        if (instance != nullptr && instance.get() == this) {
            instance = nullptr;
        }
        state = AppState::STOP;
    }

    int AppContext::GetState() {
        return state;
    }

    void AppContext::Start() {
        if (!(state & AppState::INIT)) {
            LOGE("AppContext is not initialized");
            return;
        }
        pthread_t thread;
        pthread_create(&thread, nullptr, libRetroRunner::appThread, this);
        AddCommand(AppCommands::kLoadCore);
        AddCommand(AppCommands::kLoadContent);
    }

    void AppContext::Pause() {

    }

    void AppContext::Resume() {

    }

    void AppContext::Reset() {

    }

    void AppContext::Stop() {
        state = AppState::STOP;
    }

    void AppContext::ThreadLoop() {
        state = AppState::RUNNING;
        JNIEnv *env;
        try {
            while (state != AppState::STOP) {
                if (state == AppState::PAUSE) {
                    //sleep for 16ms, for 60fps
                    usleep(16000);
                    continue;
                }


                processCommand();

                gVm->AttachCurrentThread(&env, nullptr);

                if (video != nullptr)
                    video->Prepare();

                if (core != nullptr) {
                    core->retro_run();
                    gVm->DetachCurrentThread();
                } else {
                    gVm->DetachCurrentThread();
                    usleep(16000);
                }

            }
        } catch (std::exception &exception) {
            APPLOGE("emu end with error: %s", exception.what());
        }
        APPLOGE("emu stopped");
        state = AppState::STOP;
    }

    //这个函数只在模拟线程(绘图线程)中调用
    void AppContext::processCommand() {
        int command;
        while (commands.try_pop(command)) {
            switch (command) {
                case AppCommands::kLoadCore:
                    cmdLoadCore();
                    break;
                case AppCommands::kLoadContent:
                    cmdLoadContent();
                    break;
                case AppCommands::kInitVideo:
                    cmdInitVideo();
                    break;
                case AppCommands::kNone:
                default:
                    break;
            }
        }
    }

    void AppContext::SetFiles(const std::string &romPath, const std::string &corePath, const std::string &systemPath, const std::string &savePath) {
        this->rom_path = romPath;
        this->core_path = corePath;
        this->system_path = systemPath;
        this->save_path = savePath;
        this->environment = std::make_unique<Environment>();
        environment->SetSavePath(save_path);
        environment->SetSystemPath(system_path);
    }

    void AppContext::SetVariable(const std::string &key, const std::string &value) {
        environment->UpdateVariable(key, value);
    }

    void AppContext::AddCommand(int command) {
        commands.push(command);
    }

    void AppContext::cmdLoadCore() {
        APPLOGD("command: load core");
        try {
            core = std::make_unique<Core>(this->core_path);

            core->retro_set_video_refresh(&RetroCallbacks::libretro_callback_hw_video_refresh);
            core->retro_set_environment(&RetroCallbacks::libretro_callback_set_environment);
            core->retro_set_audio_sample(&RetroCallbacks::libretro_callback_audio_sample);
            core->retro_set_audio_sample_batch(&RetroCallbacks::libretro_callback_audio_sample_batch);
            core->retro_set_input_poll(&RetroCallbacks::libretro_callback_input_poll);
            core->retro_set_input_state(&RetroCallbacks::libretro_callback_input_state);

            core->retro_init();

            struct retro_system_av_info avInfo;
            core->retro_get_system_av_info(&avInfo);
            environment->gameGeometryWidth = avInfo.geometry.base_width;
            environment->gameGeometryHeight = avInfo.geometry.base_height;
            environment->gameGeometryAspectRatio = avInfo.geometry.aspect_ratio;
            environment->gameGeometryUpdated = true;

        } catch (std::exception &exception) {
            core = nullptr;
            APPLOGE("load core failed");
        }
    }

    void AppContext::cmdLoadContent() {
        APPLOGD("command: load content: %s", rom_path.c_str());
        if (core == nullptr) {
            APPLOGE("try to load content, but core is not loaded yet!!!");
            return;
        }

        struct retro_system_info system_info{};
        core->retro_get_system_info(&system_info);

        struct retro_game_info game_info{};
        game_info.path = rom_path.c_str();
        game_info.meta = nullptr;

        if (system_info.need_fullpath) {
            game_info.data = nullptr;
            game_info.size = 0;
        } else {
            struct Utils::ReadResult file = Utils::readFileAsBytes(rom_path.c_str());
            game_info.data = file.data;
            game_info.size = file.size;
        }

        bool result = core->retro_load_game(&game_info);

        if (input == nullptr) {
            input = Input::NewInstance();
            input->Init();
        }
        for (int player = 0; player < MAX_PLAYER; player++) {
            core->retro_set_controller_port_device(player, RETRO_DEVICE_JOYPAD);
        }

        if (!result) {
            LOGE("Cannot load game. Leaving.");
            throw std::runtime_error("Cannot load game");
        }
    }

    void AppContext::cmdInitVideo() {
        APPLOGD("command: init video");
        JNIEnv *env;
        gVm->AttachCurrentThread(&env, nullptr);
        video->Init();
        if (environment->renderUseHWAcceleration) {
            environment->renderContextReset();
        }
        gVm->DetachCurrentThread();
    }

    VideoContext *AppContext::GetVideo() {
        if (this->video == nullptr) {
            this->video = VideoContext::NewInstance();
        }
        return video.get();
    }

    Input *AppContext::GetInput() {
        return input.get();
    }

    /*-----RETRO CALLBACKS--------------------------------------------------------------*/

    void RetroCallbacks::libretro_callback_hw_video_refresh(const void *data, unsigned int width, unsigned int height, size_t pitch) {
        AppContext *appContext = AppContext::Instance();
        if (appContext) {
            VideoContext *video = appContext->GetVideo();
            if (video) video->OnFrameArrive(data, width, height, pitch);
        }
    }

    bool RetroCallbacks::libretro_callback_set_environment(unsigned int cmd, void *data) {
        AppContext *appContext = AppContext::Instance();
        if (appContext) {
            return appContext->environment->HandleCoreCallback(cmd, data);
        }
        return false;
    }

    void RetroCallbacks::libretro_callback_audio_sample(int16_t left, int16_t right) {

    }

    size_t RetroCallbacks::libretro_callback_audio_sample_batch(const int16_t *data, size_t frames) {
        return 0;
    }

    void RetroCallbacks::libretro_callback_input_poll(void) {
        auto appContext = AppContext::Instance();
        if (appContext) {
            auto input = appContext->input.get();
            if (input != nullptr) input->Poll();
        }
    }

    int16_t RetroCallbacks::libretro_callback_input_state(unsigned int port, unsigned int device, unsigned int index, unsigned int id) {
        int16_t ret = 0;
        auto appContext = AppContext::Instance();
        if (appContext) {
            auto input = appContext->input.get();
            if (input != nullptr)
                ret = input->State(port, device, index, id);
        }
        if (ret > 0)
            LOGW("input state: %d", ret);
        return ret;
    }
}