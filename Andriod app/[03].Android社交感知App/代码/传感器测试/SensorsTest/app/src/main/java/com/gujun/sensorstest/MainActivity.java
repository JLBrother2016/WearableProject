package com.gujun.sensorstest;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import org.w3c.dom.Text;

import java.util.List;

public class MainActivity extends AppCompatActivity {

    private TextView tv;
    private TextView tv_direct;
    private TextView tv_linear;
    private TextView tv_light;
    private TextView tv_gyro;
    private TextView tv_magne;
    private TextView tv_baro;
    private TextView tv_step_counter;

    private SensorManager sensorManager;

    private SensorEventListener mSensorListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            switch (event.sensor.getType()) {
                case Sensor.TYPE_ORIENTATION:
                    float z = event.values[0];
                    float x = event.values[1];
                    float y = event.values[2];
                    tv_direct.setText("z轴的方向：" + z + "\n"
                            + "x轴的方向：" + x + "\n"
                            + "y轴的方向：" + y + "\n");
                    break;
                case Sensor.TYPE_LINEAR_ACCELERATION:
                    float x1 = event.values[0];
                    float y1 = event.values[1];
                    float z1 = event.values[2];
                    tv_linear.setText("x轴的加速度：" + x1 + "\n"
                            + "y轴的加速度：" + y1 + "\n"
                            + "z轴的加速度：" + z1 + "\n");
                    break;

                case Sensor.TYPE_LIGHT:
                    float light = event.values[0];
                    tv_light.setText("光强：" + light);
                    break;
                case Sensor.TYPE_GYROSCOPE:
                    float x3 = event.values[0];
                    float y3 = event.values[1];
                    float z3 = event.values[2];
                    tv_gyro.setText("x轴角速度：" + x3 + "\n"
                            + "y轴角速度：" + y3 + "\n"
                            + "z轴角速度：" + z3 + "\n");
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD:
                    float x4 = event.values[0];
                    float y4 = event.values[1];
                    float z4 = event.values[2];
                    tv_magne.setText("x轴角速度：" + x4 + "\n"
                            + "y轴角速度：" + y4 + "\n"
                            + "z轴角速度：" + z4 + "\n");
                    break;
                case Sensor.TYPE_PRESSURE:
                    float baro = event.values[0];
                    tv_baro.setText("气压：" + baro + "\n");
                    break;

                case Sensor.TYPE_STEP_COUNTER:
                    float stepCounter = event.values[0];
                    tv_step_counter.setText("步数："+stepCounter+"\n");
                default:
                    break;
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int i) {

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 1. 获取SensorManager服务
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        tv = findViewById(R.id.tv);
        tv_direct = findViewById(R.id.tv_direct);
        tv_linear = findViewById(R.id.tv_linear);
        tv_light = findViewById(R.id.tv_light);
        tv_gyro = findViewById(R.id.tv_gyro);
        tv_magne = findViewById(R.id.tv_magne);
        tv_baro = findViewById(R.id.tv_baro);
        tv_step_counter = findViewById(R.id.tv_step_counter);

        // 获取手机上支持的传感器
        List<Sensor> list = sensorManager.getSensorList(Sensor.TYPE_ALL);
        tv.append("本机上共有" + list.size() + "个传感器。" + "\n");
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
