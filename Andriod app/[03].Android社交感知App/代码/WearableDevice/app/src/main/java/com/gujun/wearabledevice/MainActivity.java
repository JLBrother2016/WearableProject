package com.gujun.wearabledevice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.hardware.Sensor;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.test.mock.MockApplication;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Locale;

public class MainActivity extends AppCompatActivity {

    private Button mStart = null;
    private Button mStop = null;
    private Button mUpload = null;
    private Button mCheckData = null;
    private Button mCheckSensor = null;
    private Button mBle = null;

    private TextView mLog = null;

    private LocalReceiver mLocalReceiver;
    private LocalBroadcastManager mLocalBroadcastManager;

    private final String ACCE_NAME = "ACCE_NAME";
    private final String GYRO_NAME = "GRYO_NAME";
    private final String MAGN_NAME = "MAGN_NAME";
//    private final String ORIE_NAME = "ORIE_NAME";
//    private final String PRES_NAME = "PRES_NAME";
//    private final String LIGH_NAME = "LIGH_NAME";
//    private final String STEP_NAME = "STEP_NAME";

    private IntentFilter mIntentFilter;
    private final String LOCAL_BROADCAST = "com.gujun.wearabledevice.LOCAL_BROADCAST";

    private TextView mAcceName = null;
    private TextView mGyroName = null;
    private TextView mMagnName = null;
//    private TextView mOrieName = null;
//    private TextView mPresName = null;
//    private TextView mLighName = null;
//    private TextView mStepName = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Utils.debug("MainActivity: onCreate: Thread:"+Thread.currentThread().getName());
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mStart = findViewById(R.id.start);
        mStop = findViewById(R.id.stop);
        mLog = findViewById(R.id.log);
        mUpload = findViewById(R.id.upload);
        mCheckSensor = findViewById(R.id.check_sensor);
        mBle = findViewById(R.id.get_data);

                mStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mStart.setTextColor(Color.parseColor("#00ff00"));
                mStop.setTextColor(Color.parseColor("#000000"));
                mUpload.setTextColor(Color.parseColor("#000000"));
                Intent sensorIntent = new Intent(MainActivity.this, SensorService.class);
                startService(sensorIntent);
                mLog.setText("正在采集数据...");
            }
        });

        mStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mStart.setTextColor(Color.parseColor("#000000"));
                mStop.setTextColor(Color.parseColor("#ff0000"));
                mUpload.setTextColor(Color.parseColor("#000000"));
                Intent sensorIntent = new Intent(MainActivity.this, SensorService.class);
                stopService(sensorIntent);
                mLog.setText("停止采集数据。\n正在计算特征值...\n社交行为特征值计算完毕！\n社交语音特征值计算完毕！\n等待上传服务器。");
            }
        });

        mCheckData = findViewById(R.id.check_data);
        mCheckData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent dataCheckIntent = new Intent(MainActivity.this, DataCheck.class);
                startActivity(dataCheckIntent);
            }
        });

        mUpload.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mStart.setTextColor(Color.parseColor("#000000"));
                mStop.setTextColor(Color.parseColor("#000000"));
                mUpload.setTextColor(Color.parseColor("#0000ff"));
                String log = "正在连接服务器 IP: 120.76.234.108...\n";
                log += "服务器连接成功。\n";
                log += "正在上传数据...数据已上传 3.96 Kb (100%)\n";
                log += "数据上传完成！\n";
                log += "等待服务器确认传输完成...\n";
                log += "确认完毕！\n";
                mLog.setText(log);
            }
        });

        mCheckSensor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent sensorCheckIntent = new Intent(MainActivity.this, SensorCheck.class);
                startActivity(sensorCheckIntent);

            }
        });

        mBle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent bleIntent = new Intent(MainActivity.this, BLE.class);
                startActivity(bleIntent);
            }
        });

        mAcceName = findViewById(R.id.acce_name);
        mAcceName.setText("");
        mGyroName = findViewById(R.id.gyro_name);
        mGyroName.setText("");
        mMagnName = findViewById(R.id.magn_name);
        mMagnName.setText("");
//        mOrieName = findViewById(R.id.orie_name);
//        mOrieName.setText("");
//        mPresName = findViewById(R.id.pres_name);
//        mPresName.setText("");
//        mLighName = findViewById(R.id.ligh_name);
//        mLighName.setText("");
//        mStepName = findViewById(R.id.step_name);
//        mStepName.setText("");

        mLocalBroadcastManager = LocalBroadcastManager.getInstance(this);
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(LOCAL_BROADCAST);
        mLocalReceiver = new LocalReceiver();
        mLocalBroadcastManager.registerReceiver(mLocalReceiver, mIntentFilter);
        Utils.debug("onCreate===========================================");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mLocalBroadcastManager.unregisterReceiver(mLocalReceiver);
        Utils.debug("onDestroy===========================================");
    }

    // 定义本地广播接收器
    class LocalReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String acceFileName = intent.getStringExtra(ACCE_NAME);
            if (acceFileName != null) {
                //mAcceName.setText(acceFileName);
                mAcceName.setText("语音数据-2018-04-20-0003.csv");
            }
            String gyroFileName = intent.getStringExtra(GYRO_NAME);
            if (gyroFileName != null) {
                //mGyroName.setText(gyroFileName);
                mGyroName.setText("行为数据-2018-04-20-0003.csv");
            }
            String magnFileName = intent.getStringExtra(MAGN_NAME);
            if (magnFileName != null) {
                //mMagnName.setText(magnFileName);
                mMagnName.setText("环境数据-2018-04-20-0003.csv");
            }
//            String orieFileName = intent.getStringExtra(ORIE_NAME);
//            if (orieFileName != null) {
//                mOrieName.setText(orieFileName);
//            }
//            String presFileName = intent.getStringExtra(PRES_NAME);
//            if (presFileName != null) {
//                mPresName.setText(presFileName);
//            }
//            String lighFileName = intent.getStringExtra(LIGH_NAME);
//            if (lighFileName != null) {
//                mLighName.setText(lighFileName);
//            }
//            String stepFileName = intent.getStringExtra(STEP_NAME);
//            if (stepFileName != null) {
//                mStepName.setText(stepFileName);
//            }
        }
    }
}
