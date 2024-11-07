package com.aidoo.retrorunner;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

@SuppressLint("ViewConstructor")
public class RetroRunnerView extends SurfaceView implements SurfaceHolder.Callback {

    private final RunConfig config;

    public RetroRunnerView(Context context, RunConfig config) {
        super(context);
        this.config = config;
        setupView();
    }

    private void setupView() {
        NativeRunner.initEnv();
        startEmu(config);
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        NativeRunner.setSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.e("RetroRunner", "surfaceChanged " + width + "x" + height);
        NativeRunner.setSurfaceSize(width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        NativeRunner.setSurface(null);
    }


    private void startEmu(RunConfig config) {
        NativeRunner.create(config.getRomPath(), config.getCorePath(), config.getSystemPath(), config.getSavePath());
        if (config.haveVariables()) {
            for (String key : config.getVariables().keySet()) {
                NativeRunner.setVariable(key, config.getVariables().get(key));
            }
        }
        NativeRunner.start();
    }

    public void stopEmu() {
        NativeRunner.stop();
    }

    public void pauseEmu(boolean pause) {
        if (pause) {
            NativeRunner.pause();
        } else {
            NativeRunner.resume();
        }
    }

    public void resetEmu() {
        NativeRunner.reset();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        NativeRunner.updateButtonState(0, event.getKeyCode(), event.getAction() == KeyEvent.ACTION_DOWN);
        return true;
    }
}
