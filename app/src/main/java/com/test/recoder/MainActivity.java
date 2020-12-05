package com.test.recoder;



import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.test.recoder.permission.Permissions;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import androidx.annotation.NonNull;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("recoder-lib");
    }


    Button startRecord;
    Button stopRecord;
    Button play;
    static BufferedOutputStream bos;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        startRecord = (Button) findViewById(R.id.start);
        stopRecord = (Button) findViewById(R.id.stop);
        play = (Button) findViewById(R.id.play);

        Permissions.requestPermissionAll(this);
        startRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread() {
                    public void run() {

                        initOutputStream();

                        startRecord();
                    }
                }.start();

            }
        });


        stopRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread() {
                    public void run() {
                        stopRecord();

                        try {
                            Thread.sleep(1000 * 2);
                            if (bos != null) {
                                try {
                                    bos.close();
                                } catch (IOException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                }
                            }
                        } catch (Exception e) {

                        }
                    }
                }.start();

            }
        });

        play.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread() {
                    public void run() {
                        play();
                    }
                }.start();
            }
        });

    }


    public void initOutputStream() {
        File file = new File("/storage/emulated/0/RecoderTest/temp.pcm");
        try {
            bos = new BufferedOutputStream(new FileOutputStream(file));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

    }

    public static void receiveAudioData(byte[] data, int size) {
        try {
            bos.write(data);
            bos.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            Permissions.changePermissionState(this,permissions[0],true);
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native void startRecord();
    public native void stopRecord();
    public native void play();
}
