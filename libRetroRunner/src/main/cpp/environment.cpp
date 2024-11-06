//
// Created by aidoo on 2024/11/1.
//
#include "environment.h"
#include "libretro-common/include/libretro.h"
#include "rr_log.h"
#include <EGL/egl.h>

#define POINTER_VAL(_TYPE_) (*((_TYPE_*)data))
#define LOGDCall(...) LOGD("[Environment] "  __VA_ARGS__)

namespace libRetroRunner {
    void Environment::UpdateVariable(const std::string &key, const std::string &value, bool notifyCore) {

    }

    Environment::Environment() {

    }

    Environment::~Environment() {

    }

    bool Environment::HandleCoreCallback(unsigned int cmd, void *data) {
        switch (cmd) {
            case RETRO_ENVIRONMENT_SET_ROTATION: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_ROTATION");
                break;
            }
            case RETRO_ENVIRONMENT_GET_CAN_DUPE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_CAN_DUPE");
                POINTER_VAL(bool) = false;
                return true;
            }
            case RETRO_ENVIRONMENT_SET_MESSAGE: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_MESSAGE");
                auto *msg = static_cast<struct retro_message *>(data);
                LOGD("Message: %s", msg->msg);
                return true;
            }
            case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY");
                POINTER_VAL(const char*) = systemPath.c_str();
                return !systemPath.empty();
            }
            case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_PIXEL_FORMAT");
                return cmdSetPixelFormat(data);
            }
            case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE");
                diskControllerCallback = static_cast<retro_disk_control_callback *>(data);
                return true;
            }
            case RETRO_ENVIRONMENT_SET_HW_RENDER: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_HW_RENDER");
                return cmdSetHardwareRender(data);
            }
            case RETRO_ENVIRONMENT_GET_VARIABLE: {
                //LOGDCall("call RETRO_ENVIRONMENT_GET_VARIABLE");
                return cmdGetVariable(data);
            }
            case RETRO_ENVIRONMENT_SET_VARIABLES: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_VARIABLES");
                return cmdSetVariables(data);
            }
            case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
                //LOGDCall("call RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
                POINTER_VAL(bool) = variablesChanged;
                variablesChanged = false;
                return true;
            }
            case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME");
                coreSupportNoGame = POINTER_VAL(bool);
                return true;
            }
            case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK");
                //auto callback = static_cast<const struct retro_audio_callback *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE");
                auto callback = static_cast<struct retro_rumble_interface *>(data);
                callback->set_rumble_state = &Environment::CoreCallbackSetRumbleState;
                return false;
            }
            case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES");
                POINTER_VAL(uint64_t) = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG) | (1 << RETRO_DEVICE_POINTER);
                return false;
            }
            case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
                auto callback = static_cast<struct retro_log_callback *>(data);
                callback->log = &Environment::CoreCallbackLog;
                return true;
            }
            case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
                //TODO:在这里添加性能计数器
                return false;
            }
            case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY , RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY");
                //TODO:在这里返回内容目录
                return false;
            }
            case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
                POINTER_VAL(const char*) = savePath.c_str();
                return !savePath.empty();
            }
            case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
                return cmdSetGeometry(data);
            }
            case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: {
                //TODO:通知前端支持的控制器信息，以方便用户选择不同的控制器,然后使用retro_set_controller_port_device进行设置
                LOGDCall("call RETRO_ENVIRONMENT_SET_CONTROLLER_INFO");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: {
                //TODO:通知前端核心所使用的内存空间
                LOGDCall("call RETRO_ENVIRONMENT_SET_MEMORY_MAPS");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_GEOMETRY: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_GEOMETRY");
                return cmdSetGeometry(data);
            }
            case RETRO_ENVIRONMENT_GET_USERNAME: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_USERNAME");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_LANGUAGE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_LANGUAGE");
                POINTER_VAL(unsigned) = language;
                return true;
            }
            case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER");
                return cmdGetCurrentFrameBuffer(data);
            }
            case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE: {
                //返回前端硬件渲染的类型，不是所有核心都需要这个回调
                LOGDCall("call RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE");
                //auto callback = static_cast<const struct retro_hw_render_interface **>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS: {
                //通知前端核心是否支持成就
                LOGDCall("call RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS");
                return true;
            }
            case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE: {
                //通知前端核心是否支持硬件渲染上下文协商
                LOGDCall("call RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS: {
                //通知前端核心是否支持序列化特性
                LOGDCall("call RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT: {
                //通知前端:核心是否支持共享硬件渲染上下文
                LOGDCall("call RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_VFS_INTERFACE: {
                //TODO:获取虚拟文件系统
                LOGDCall("call RETRO_ENVIRONMENT_GET_VFS_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_LED_INTERFACE: {
                //TODO:获取LED系统
                LOGDCall("call RETRO_ENVIRONMENT_GET_LED_INTERFACE");
                return false;

            }
            case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE");
                int ret = 0;
                if (audioEnabled)
                    ret = ret | RETRO_AV_ENABLE_VIDEO;
                if (videoEnabled)
                    ret = ret | RETRO_AV_ENABLE_AUDIO;
                POINTER_VAL(retro_av_enable_flags) = (retro_av_enable_flags) ret;
                return true;
            }
            case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE: {
                //TODO:获取MIDI系统
                LOGDCall("call RETRO_ENVIRONMENT_GET_MIDI_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_FASTFORWARDING: {
                //LOGDCall("call RETRO_ENVIRONMENT_GET_FASTFORWARDING");
                POINTER_VAL(bool) = fastForwarding;
                return true;
            }
            case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE: {
                //返回目标刷新率
                LOGDCall("call RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE");
                POINTER_VAL(float) = 60.0f;
                return false;
            }
            case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: {
                //TODO:返回前端是否支持以掩码的方式一次性获取所有的输入信息,如果返回true, 则需要在retro_input_state_t方法中检测RETRO_DEVICE_ID_JOYPAD_MASK并返回所有的输入
                LOGDCall("call RETRO_ENVIRONMENT_GET_INPUT_BITMASKS");
                return true;
            }
            case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: {
                //TODO:返回前端所支持的核心选项版本, 0, 1, 2, 不同的版本会有不同的核心选项组织方式
                LOGDCall("call RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION");
                POINTER_VAL(unsigned) = 0;
                return true;
            }
            case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: {
                /*TODO:通知前端核心选项，已经被当前版本的核心所弃用。应当使用 RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
                    这个回调是为了用于取代 RETRO_ENVIRONMENT_SET_VARIABLES (RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION 返回 >= 1时)，
                    如果核心使用了新的版本返回选项，则需要实现这个回调, 其结构体为retro_core_option_definition，类似于json的实现
                 */
                LOGDCall("call RETRO_ENVIRONMENT_SET_CORE_OPTIONS");
                //auto request = static_cast<const struct retro_core_option_definition *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL: {
                /*TODO:RETRO_ENVIRONMENT_SET_CORE_OPTIONS的变体，用于支持多语言*/
                LOGDCall("call RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL");
                //auto request = static_cast<const struct retro_core_options_intl *>(data);
                return false;

            }
            case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY: {
                //用于控制核心选项的可见性
                LOGDCall("call RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY");
                //auto request = static_cast<const struct retro_core_option_display *>(data);
                return true;
            }
            case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER: {
                //返回前端所期望的硬件渲染类型
                LOGDCall("call RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER");
                POINTER_VAL(retro_hw_context_type) = RETRO_HW_CONTEXT_OPENGLES3;
                return true;
            }
            case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION: {
                //返回前端所支持的磁盘控制接口版本, 如果值 >= 1, 核心会使用 RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
                LOGDCall("call RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION");
                POINTER_VAL(unsigned) = 0;
                return true;
            }
            case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE: {
                //通知前端核心所支持的磁盘控制扩展接口
                LOGDCall("call RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE");
                //auto request = static_cast<const struct retro_disk_control_ext_interface *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION: {
                //返回前端所支持的消息接口版本, 0表示只支持RETRO_ENVIRONMENT_SET_MESSAGE, 1表示还支持RETRO_ENVIRONMENT_SET_MESSAGE_EXT
                LOGDCall("call RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION");
                POINTER_VAL(unsigned) = 0;
                return true;
            }
            case RETRO_ENVIRONMENT_SET_MESSAGE_EXT: {
                //向前端发送一个用户需要关心的信息，其他消息使用日志接口来返回
                LOGDCall("call RETRO_ENVIRONMENT_SET_MESSAGE_EXT");
                auto request = static_cast<const struct retro_message_ext *>(data);
                LOGW("Important: %s", request->msg);
                return true;
            }
            case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS: {
                //返回前端所支持的最大用户数
                LOGDCall("call RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS");
                POINTER_VAL(unsigned) = maxUserCount;
                return true;
            }
            case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK: {
                //向核心注册一个回调，用于核心通知前端音频缓冲区的状态，比如有时核心需要跳过一些音频帧
                LOGDCall("call RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK");
                auto request = static_cast<struct retro_audio_buffer_status_callback *>(data);
                request->callback = &Environment::CoreCallbackNotifyAudioState;
                return false;
            }
            case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY: {
                //通知前端核心所需要的最小音频延迟
                LOGDCall("call RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE: {
                //通知前端核心是否应该快进, 比如有时核心需要跳过一些帧时
                LOGDCall("call RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE");
                //auto request = static_cast<const struct retro_fastforwarding_override *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_GAME_INFO_EXT");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2: {
                //TODO:通知前端核心选项，用于替代 RETRO_ENVIRONMENT_SET_VARIABLES， 只在RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION返回 >= 2时使用
                LOGDCall("call RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2");
                //auto request = static_cast<const struct retro_core_options_v2 *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL: {
                //TODO:通知前端核心选项，用于替代 RETRO_ENVIRONMENT_SET_VARIABLES， 只在RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION返回 >= 2时使用
                //RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 的变体，支持多语言
                LOGDCall("call RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL");
                //auto request = static_cast<const struct retro_core_options_v2 *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK: {
                //用于前端向核心通知哪些核心设置应该显示或者应该隐藏
                LOGDCall("call RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_VARIABLE: {
                //核心通知前端选项值发生变化。
                LOGDCall("call RETRO_ENVIRONMENT_SET_VARIABLE");
                return cmdSetVariable(data);
                return true;
            }
            case RETRO_ENVIRONMENT_GET_THROTTLE_STATE: {
                //用于核心获取前端的帧率运行情況
                LOGDCall("call RETRO_ENVIRONMENT_GET_THROTTLE_STATE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT: {
                //todo:用于核心获取前端想要的存档状态,在这里控制存档的类型，是用于对战还是正常游戏
                LOGDCall("call RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT");
                POINTER_VAL(retro_savestate_context) = RETRO_SAVESTATE_CONTEXT_NORMAL;
                //auto request = static_cast<retro_savestate_context *>(data);
                return true;
            }
            case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT: {
                //在SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE之前调用，用于确认所支持的类型
                LOGDCall("call RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT");
                auto request = static_cast<struct retro_hw_render_context_negotiation_interface *>(data);
                return false;
            }
            case RETRO_ENVIRONMENT_GET_JIT_CAPABLE: {
                //用于确认当前环境是否支持JIT,主要用于iOS, Javascript
                LOGDCall("call RETRO_ENVIRONMENT_GET_JIT_CAPABLE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_DEVICE_POWER: {
                //todo:返回设备的电量，有的核心有可能在低电量下运行效率缓慢。
                LOGDCall("call RETRO_ENVIRONMENT_GET_DEVICE_POWER");
                return false;
            }
            case RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE: {
                LOGDCall("call RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY -> false");
                return false;
            }
            case RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY: {
                LOGDCall("call RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY -> false");
                return false;
            }
            default:
                LOGDCall("not handled: %d -> false", cmd);
                break;
        }
        return false;
    }

    void Environment::SetSystemPath(const std::string &path) {
        this->systemPath = path;
    }

    void Environment::SetSavePath(const std::string &path) {
        this->savePath = path;
    }

    bool Environment::cmdSetPixelFormat(void *data) {
        auto format = static_cast<enum retro_pixel_format *>(data);
        this->pixelFormat = *format;
        return true;
    }

    bool Environment::cmdSetHardwareRender(void *data) {
        //TODO: 这里需要实现硬件渲染回调
        if (data == nullptr) return false;
        auto hwRender = static_cast<struct retro_hw_render_callback *>(data);
        renderMajorVersion = hwRender->version_major;
        renderMinorVersion = hwRender->version_minor;
        renderContextType = hwRender->context_type;

        renderUseHWAcceleration = true;
        renderUseDepth = hwRender->depth;
        renderUseStencil = hwRender->stencil;

        renderContextReset = hwRender->context_reset;
        renderContextDestroy = hwRender->context_destroy;
        hwRender->get_proc_address = &Environment::CoreCallbackGetProcAddress;
        hwRender->get_current_framebuffer = &Environment::CoreCallbackGetCurrentFrameBuffer;
        return true;
    }

    bool Environment::cmdGetVariable(void *data) {
        auto request = static_cast<struct retro_variable *>(data);
        auto foundVariable = variables.find(std::string(request->key));

        if (foundVariable == variables.end()) {
            LOGDCall("call RETRO_ENVIRONMENT_GET_VARIABLE: %s -> null", request->key);
            return false;
        }
        request->value = foundVariable->second.value.c_str();
        LOGDCall("call RETRO_ENVIRONMENT_GET_VARIABLE: %s -> %s", request->key, request->value);
        return true;
    }

    bool Environment::cmdSetVariables(void *data) {
        /*核心通知给前端的有可能的选项值*/
        auto request = static_cast<const struct retro_variable *>(data);
        unsigned idx = 0;
        while (request[idx].key != nullptr) {
            cmdSetVariable((void *) (&request[idx]));
            idx++;
        }
        return true;
    }

    bool Environment::cmdSetVariable(void *data) {
        auto request = static_cast<const struct retro_variable *>(data);
        if (request && request->key != nullptr) {
            std::string key(request->key);
            std::string description(request->value);
            std::string value(request->value);

            auto firstValueStart = value.find(';') + 2;
            auto firstValueEnd = value.find('|', firstValueStart);
            value = value.substr(firstValueStart, firstValueEnd - firstValueStart);

            auto currentVariable = variables[key];
            currentVariable.key = key;
            currentVariable.description = description;

            if (currentVariable.value.empty()) {
                currentVariable.value = value;
            }

            variables[key] = currentVariable;
            LOGDCall("core provide variable: %s -> %s", key.c_str(), value.c_str());
        }
        return true;
    }

    uintptr_t Environment::CoreCallbackGetCurrentFrameBuffer() {
        //TODO:已经被标记为过时，前端不应该返回预先分配的缓冲区
        return 0;
    }

    bool Environment::CoreCallbackSetRumbleState(unsigned int port, enum retro_rumble_effect effect, uint16_t strength) {
        return false;
    }

    void Environment::CoreCallbackLog(enum retro_log_level level, const char *fmt, ...) {
        va_list argv;
        va_start(argv, fmt);

        switch (level) {
#if CORE_DEUBG_LOG
            case RETRO_LOG_DEBUG:
                __android_log_vprint(ANDROID_LOG_DEBUG, MODULE_NAME_CORE, fmt, argv);
                break;
#endif
            case RETRO_LOG_INFO:
                __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, argv);
                break;
            case RETRO_LOG_WARN:
                __android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, fmt, argv);
                break;
            case RETRO_LOG_ERROR:
                __android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, fmt, argv);
                break;
            default:
                break;
        }
    }

    void Environment::CoreCallbackNotifyAudioState(bool active, unsigned int occupancy, bool underrun_likely) {
        //TODO: 核心通知前端音频状态
    }

    retro_proc_address_t Environment::CoreCallbackGetProcAddress(const char *sym) {
        LOGDCall("get proc address: %s", sym);
        return (retro_proc_address_t) eglGetProcAddress(sym);
    }

    bool Environment::cmdSetGeometry(void *data) {
        LOGDCall("Environment::cmdSetGeometry");
        auto geometry = static_cast<struct retro_game_geometry *>(data);
        gameGeometryHeight = geometry->base_height;
        gameGeometryWidth = geometry->base_width;
        gameGeometryAspectRatio = geometry->aspect_ratio;
        gameGeometryUpdated = true;
        return true;
    }

    bool Environment::cmdGetCurrentFrameBuffer(void *data) {
        //TODO: 用于返回当前的软件渲染帧缓冲区, 当使用软件渲染时，可用于性能调优
        auto callback = static_cast<struct retro_framebuffer *>(data);
        //callback->data = 0;  //frame buffer ptr
        callback->format = (enum retro_pixel_format) pixelFormat;
        return false;
    }


}


