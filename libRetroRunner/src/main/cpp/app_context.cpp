//
// Created by aidoo on 2024/10/31.
//

#include <pthread.h>
#include <unistd.h>

#include "app_context.h"
#include "rr_log.h"

namespace libRetroRunner {

    static void *appThread(void *args) {
        ((AppContext *) args)->ThreadLoop();
        return nullptr;
    }


    AppContext *AppContext::NewInstance() {
        instance =  std::make_unique<AppContext>();
        return instance.get();
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

    AppState AppContext::GetState() {
        return state;
    }

    void AppContext::Start() {
        if (state != AppState::INIT) {
            LOGE("AppContext is not initialized");
            return;
        }
        pthread_t thread;
        pthread_create(&thread, nullptr, libRetroRunner::appThread, this);
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
        while (state != AppState::STOP) {
            if (state == AppState::PAUSE) {
                //sleep for 16ms, for 60fps
                usleep(16000);
                continue;
            }
            processCommand();
            //TODO: do something
        }
        state = AppState::STOP;
    }

    void AppContext::processCommand() {
        int command;
        while (commands.try_pop(command)) {
            switch (command) {
                case AppCommands::kLoadCore:
                    loadCore();
                    break;
                case AppCommands::kNone:
                default:
                    break;
            }
        }
    }

    void AppContext::loadCore() {
        try {
            core = std::make_unique<Core>(this->core_path);

            core->retro_set_video_refresh(&RetroCallbacks::libretro_callback_hw_video_refresh);
            core->retro_set_environment(&RetroCallbacks::libretro_callback_set_environment);
            core->retro_set_audio_sample(&RetroCallbacks::libretro_callback_audio_sample);
            core->retro_set_audio_sample_batch(&RetroCallbacks::libretro_callback_audio_sample_batch);
            core->retro_set_input_poll(&RetroCallbacks::libretro_callback_input_poll);
            core->retro_set_input_state(&RetroCallbacks::libretro_callback_input_state);
            //TODO: update variables
            core->retro_init();
        } catch (std::exception &exception) {
            LOGE("load core failed");
        }
    }

    void AppContext::SetFiles(const std::string &rom_path, const std::string &core_path, const std::string &system_path, const std::string &save_path) {
        this->rom_path = rom_path;
        this->core_path = core_path;
        this->system_path = system_path;
        this->save_path = save_path;
        this->environment = std::make_unique<Environment>();
        environment->SetSavePath(save_path);
        environment->SetSystemPath(system_path);
    }

    void AppContext::SetVariable(const std::string &key, const std::string &value) {
        environment->UpdateVariable(key, value);
    }


    /*-----RETRO CALLBACKS--------------------------------------------------------------*/

    void RetroCallbacks::libretro_callback_hw_video_refresh(const void *data, unsigned int width, unsigned int height, size_t pitch) {

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

    }

    int16_t RetroCallbacks::libretro_callback_input_state(unsigned int port, unsigned int device, unsigned int index, unsigned int id) {
        return 0;
    }
}