package com.gujun.wearabledevice;

import android.hardware.*;
import android.hardware.Sensor;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.util.List;

public class SensorCheck extends AppCompatActivity {

    private TextView tv;
    private TextView tv_direct;
    private TextView tv_linear;
    private TextView tv_light;
    private TextView tv_gyro;
    private TextView tv_magne;
    private TextView tv_baro;
    //private TextView tv_step_counter;

    private SensorManager sensorManager;

    private SensorEventListener mSensorListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            switch (event.sensor.getType()) {

                case android.hardware.Sensor.TYPE_LINEAR_ACCELERATION:
                    float x1 = event.values[0];
                    float y1 = event.values[1];
                    float z1 = event.values[2];
                    tv_linear.setText(String.format("x轴加速度 %.2f\ny轴加速度 %.2f\nz轴加速度 %.2f\n", x1, y1, z1));
                    break;


                case android.hardware.Sensor.TYPE_GYROSCOPE:
                    float x3 = event.values[0];
                    float y3 = event.values[1];
                    float z3 = event.values[2];
                    tv_gyro.setText(String.format("x轴角速度 %.2f\ny轴角速度：%.2f\nz轴角速度 %.2f\n", x3, y3, z3));
                    break;

                case android.hardware.Sensor.TYPE_LIGHT:
                    float light = event.values[0];
                    tv_light.setText(String.format("光强 %.2f\n", light));
                    break;

                case android.hardware.Sensor.TYPE_PRESSURE:
                    float baro = event.values[0];
                    tv_baro.setText(String.format("气压 %.2f\n", baro));
                    break;

                case android.hardware.Sensor.TYPE_STEP_COUNTER:
                    float stepCounter = event.values[0];
                    //tv_step_counter.setText(String.format("步数 %.2f\n", stepCounter));
                default:
                    break;
            }
        }

        @Override
        public void onAccuracyChanged(android.hardware.Sensor sensor, int i) {

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor_check);

        // 1. 获取SensorManager服务
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        tv = findViewById(R.id.tv);
        tv_linear = findViewById(R.id.tv_linear);
        tv_light = findViewById(R.id.tv_light);
        tv_gyro = findViewById(R.id.tv_gyro);
        tv_baro = findViewById(R.id.tv_baro);
        //tv_step_counter = findViewById(R.id.tv_step_counter);

    }

    @Override
    protected void onResume() {
        super.onResume();
        //23.获得相应传感器并注册监听器
        //第三个参数表示精度
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION), sensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION), sensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT), sensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), sensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD), sensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_PRESSURE), sensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(mSensorListener, sensorManager.getDefaultSensor(Sensor.TYPE_STEP_COUNTER), sensorManager.SENSOR_DELAY_UI);
    }

    protected void onStop() {
        //4.解除绑定
        sensorManager.unregisterListener(mSensorListener);
        super.onStop();
    }
}
