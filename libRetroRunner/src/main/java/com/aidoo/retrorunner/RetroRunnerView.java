package com.aidoo.retrorunner;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class RetroRunnerView extends SurfaceView implements SurfaceHolder.Callback {
    public RetroRunnerView(Context context) {
        super(context);
        setupView();
    }

    public RetroRunnerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setupView();
    }

    public RetroRunnerView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setupView();
    }

    private void setupView() {
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        NativeRunner.setSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        NativeRunner.setSurfaceSize(width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        NativeRunner.setSurface(null);
    }


    public void startEmu(RunConfig config) {
        NativeRunner.create(config.getRomPath(), config.getCorePath(), config.getSystemPath(), config.getSavePath());
        if (config.haveVariables()) {
            NativeRunner.setVariables(, config.getVariables(), );
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

}
