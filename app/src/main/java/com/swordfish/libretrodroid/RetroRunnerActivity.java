package com.swordfish.libretrodroid;

import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.ViewGroup;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.aidoo.retrorunner.RetroRunnerView;
import com.aidoo.retrorunner.RunConfig;

public class RetroRunnerActivity extends AppCompatActivity {
    private RetroRunnerView retroRunnerView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String platform = "dc";
        String testRom = "";
        String testCore = "";
        if (platform.equals("nes")) {
            testRom = getExternalFilesDir(null).getAbsolutePath() + "/test.nes";
            testCore = "libfceumm.so";
            FileUtil.copyFromAsses(this, "demo.nes", testRom);
        } else if (platform.equals("dc")) {
            testRom = getFilesDir().getAbsolutePath() + "/lc.chd";
            testCore = "libflycast.so";
        }

        RunConfig runConfig = new RunConfig();
        runConfig.setRomPath(testRom);
        runConfig.setCorePath(testCore);
        runConfig.setSystemPath(getFilesDir().getAbsolutePath());
        runConfig.setSavePath(getExternalFilesDir(null).getAbsolutePath());
        retroRunnerView = new RetroRunnerView(this, runConfig);
        retroRunnerView.setLayoutParams(new ViewGroup.LayoutParams(-1, -1));
        setContentView(retroRunnerView);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {

        return retroRunnerView.dispatchKeyEvent(event);
    }
}
