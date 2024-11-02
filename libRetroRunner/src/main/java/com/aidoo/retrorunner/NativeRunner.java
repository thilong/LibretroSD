package com.aidoo.retrorunner;

import android.view.Surface;

import java.util.HashMap;

public class NativeRunner {
    static {
        System.loadLibrary("RetroRunner");
    }

    private static NativeRunner instance;

    public static NativeRunner getInstance() {
        if (instance == null)
            instance = new NativeRunner();
        return instance;
    }

    public void initIfNeeded() {

    }

    /*jni methods*/
    /*设置全局的环境变量*/
    public static native void initEnv();

    /*创建一个新的模拟实例，如果已经有在模拟的，则停止*/
    public static native void create(String romPath, String corePath, String systemPath, String savePath);

    /*设置模拟的初始变量*/
    public static native void setVariables(HashMap<String, String> variables);

    /*开始模拟*/
    public static native void start();
    /*暂停*/
    public static native void pause();
    /*恢复*/
    public static native void resume();
    /*重置*/
    public static native void reset();
    /*停止，停止后不可恢复*/
    public static native void stop();

    /*设置模拟的输出显示目标*/
    public static native void setSurface(Surface surface);

    /*更新显示输出的尺寸*/
    public static native void setSurfaceSize(int width, int height);
}
