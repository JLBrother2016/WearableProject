package com.gujun.wearabledevice;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class SensorService extends Service {

    private static final String TAG = "WearableDevice";

    private static final String ACCELEROMETER = "ACCE";
    private static final String ORIENTATION = "ORIE";
    private static final String GYROSCOPE = "GYRO";
    private static final String MAGNETIC = "MAGN";
    private static final String LIGHT = "LIGH";
    private static final String PRESSURE = "PRES";
    private static final String STEP = "STEP";

    private String mOrieFileName = null;
    private String mOrieFilePath = null;
    private FileWriter mOrieWriter = null;

    private String mGyroFileName = null;
    private String mGyroFilePath = null;
    private FileWriter mGyroWriter = null;

    private String mMagnFileName = null;
    private String mMagnFilePath = null;
    private FileWriter mMagnWriter = null;

    private String mAcceFileName = null;
    private String mAcceFilePath = null;
    private FileWriter mAcceWriter = null;

    private String mLighFileName = null;
    private String mLighFilePath = null;
    private FileWriter mLighWriter = null;

    private String mPresFileName = null;
    private String mPresFilePath = null;
    private FileWriter mPresWriter = null;

    private String mStepFileName = null;
    private String mStepFilePath = null;
    private FileWriter mStepWriter = null;

    private boolean mLogging = false;
    private String  mSensorEventTimestamp = null;
    private Date mCurrentDate = null;
    private SensorManager mSensorManager = null;
    private static final int maxLines = 60000000; // 10分钟保存一个文件

    private static int acceLineCount = 0;
    private static int gyroLineCount = 0;
    private static int magnLineCount = 0;
    private static int orieLineCount = 0;
    private static int presLineCount = 0;
    private static int stepLineCount = 0;
    private static int lighLineCount = 0;

    private static final int BufferMaxSize = 100;
    private final String LOCAL_BROADCAST = "com.gujun.wearabledevice.LOCAL_BROADCAST";

    private final String ACCE_NAME = "ACCE_NAME";
    private final String GYRO_NAME = "GRYO_NAME";
    private final String MAGN_NAME = "MAGN_NAME";
    private final String ORIE_NAME = "ORIE_NAME";
    private final String PRES_NAME = "PRES_NAME";
    private final String LIGH_NAME = "LIGH_NAME";
    private final String STEP_NAME = "STEP_NAME";


    float [][] bufferAcce = new float[BufferMaxSize][3];
    float [][] bufferOrie = new float[BufferMaxSize][3];
    float [][] bufferGyro = new float[BufferMaxSize][3];
    float [][] bufferMagn = new float[BufferMaxSize][3];

    int bufferAcceIndex = 0;
    int bufferOrieIndex = 0;
    int bufferGyroIndex = 0;
    int bufferMagnIndex = 0;
    static float stepCount = 0;

    private SensorEventListener mSensorListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            if (mLogging) {
                // 超过最大行数就换一个文件继续写
                switch (event.sensor.getType()) {
                    // 加速度
                    case Sensor.TYPE_LINEAR_ACCELERATION:
                        acceLineCount++;
                        try {
                            for (int i = 0; i < 3; ++i) {
                                bufferAcce[bufferAcceIndex][i] = event.values[i];
                            }
                            bufferAcceIndex++;
                            if (bufferAcceIndex >= BufferMaxSize) {
                                long timeInMillis = System.currentTimeMillis();
                                mCurrentDate = new Date(timeInMillis);
                                SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                                mSensorEventTimestamp = date.format(mCurrentDate);
                                mAcceWriter.write(String.format(Locale.US,mSensorEventTimestamp+"\n"));
                                for (int i = 0; i < bufferAcceIndex; ++i) {
                                    mAcceWriter.write(String.format(Locale.US,"%.2f,%.2f,%.2f\n", bufferAcce[i][0], bufferAcce[i][1],bufferAcce[i][2]));
                                }
                                bufferAcceIndex = 0;
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (acceLineCount >= maxLines) {
                            if (mAcceWriter != null) {
                                try {
                                    mAcceWriter.close();
                                    Utils.debug("加速度数据文件行数为:"+acceLineCount+"，关闭旧文件，创建新文件继续采集~");
                                    mAcceFileName = "ACCE-"+Utils.getDateTimeString()+".csv";
                                    mAcceFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
                                    mAcceWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mAcceFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(ACCE_NAME, mAcceFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            acceLineCount = 0;
                        }
                        break;
                    // 陀螺仪
                    case Sensor.TYPE_GYROSCOPE:
                        gyroLineCount++;
                        try {
                            for (int i = 0; i < 3; ++i) {
                                bufferGyro[bufferGyroIndex][i] = event.values[i];
                            }
                            bufferGyroIndex++;
                            if (bufferGyroIndex >= BufferMaxSize) {
                                long timeInMillis = System.currentTimeMillis();
                                mCurrentDate = new Date(timeInMillis);
                                SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                                mSensorEventTimestamp = date.format(mCurrentDate);
                                mGyroWriter.write(String.format(Locale.US,mSensorEventTimestamp+"\n"));
                                for (int i = 0; i < bufferGyroIndex; ++i) {
                                    mGyroWriter.write(String.format(Locale.US,"%.2f,%.2f,%.2f\n", bufferGyro[i][0], bufferGyro[i][1],bufferGyro[i][2]));
                                }
                                bufferGyroIndex = 0;
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (gyroLineCount >= maxLines) {
                            if (mGyroWriter != null) {
                                try {
                                    mGyroWriter.close();
                                    Utils.debug("加速度数据文件行数为:"+gyroLineCount+"，关闭旧文件，创建新文件继续写~");
                                    mGyroFileName = "GYRO-"+Utils.getDateTimeString()+".csv";
                                    mGyroFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mGyroFileName;
                                    mGyroWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mGyroFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(GYRO_NAME, mGyroFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            gyroLineCount = 0;
                        }
                        break;
                    // 磁场
                    case Sensor.TYPE_MAGNETIC_FIELD:
                        magnLineCount++;
                        try {
                            for (int i = 0; i < 3; ++i) {
                                bufferMagn[bufferMagnIndex][i] = event.values[i];
                            }
                            bufferMagnIndex++;
                            if (bufferMagnIndex >= BufferMaxSize) {
                                long timeInMillis = System.currentTimeMillis();
                                mCurrentDate = new Date(timeInMillis);
                                SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                                mSensorEventTimestamp = date.format(mCurrentDate);
                                mMagnWriter.write(String.format(Locale.US,mSensorEventTimestamp+"\n"));
                                for (int i = 0; i < bufferMagnIndex; ++i) {
                                    mMagnWriter.write(String.format(Locale.US,"%.2f,%.2f,%.2f\n", bufferMagn[i][0], bufferMagn[i][1],bufferMagn[i][2]));
                                }
                                bufferMagnIndex = 0;
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (magnLineCount >= maxLines) {
                            if (mMagnWriter != null) {
                                try {
                                    mMagnWriter.close();
                                    Utils.debug("磁场数据文件行数为:"+magnLineCount+"，关闭旧文件，创建新文件继续写~");
                                    mMagnFileName = "MAGN-"+Utils.getDateTimeString()+".csv";
                                    mMagnFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mMagnFileName;
                                    mMagnWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mMagnFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(MAGN_NAME, mMagnFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            magnLineCount = 0;
                        }
                        break;
                    // 方向
                    case Sensor.TYPE_ORIENTATION:
                        orieLineCount++;
                        try {
                            for (int i = 0; i < 3; ++i) {
                                bufferOrie[bufferOrieIndex][i] = event.values[i];
                            }
                            bufferOrieIndex++;
                            if (bufferOrieIndex >= BufferMaxSize) {
                                long timeInMillis = System.currentTimeMillis();
                                mCurrentDate = new Date(timeInMillis);
                                SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                                mSensorEventTimestamp = date.format(mCurrentDate);
                                mOrieWriter.write(String.format(Locale.US,mSensorEventTimestamp+"\n"));
                                for (int i = 0; i < bufferOrieIndex; ++i) {
                                    mOrieWriter.write(String.format(Locale.US,"%.2f,%.2f,%.2f\n", bufferOrie[i][0], bufferOrie[i][1],bufferOrie[i][2]));
                                }
                                bufferOrieIndex = 0;
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (orieLineCount >= maxLines) {
                            if (mOrieWriter != null) {
                                try {
                                    mOrieWriter.close();
                                    Utils.debug("方向数据文件行数为:"+orieLineCount+"，关闭旧文件，创建新文件继续写~");
                                    mOrieFileName = "ORIE-"+Utils.getDateTimeString()+".csv";
                                    mOrieFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mOrieFileName;
                                    mOrieWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mOrieFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(ORIE_NAME, mOrieFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            orieLineCount = 0;
                        }
                        break;
                    // 光强
                    case Sensor.TYPE_LIGHT:
                        lighLineCount++;
                        try{
                            long timeInMillis = System.currentTimeMillis();
                            mCurrentDate = new Date(timeInMillis);
                            SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                            mSensorEventTimestamp = date.format(mCurrentDate);
                            mLighWriter.write(String.format(Locale.US,mSensorEventTimestamp+",%.2f\n", event.values[0]));
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (lighLineCount >= maxLines) {
                            if (mLighWriter != null) {
                                try {
                                    mLighWriter.close();
                                    Utils.debug("光强数据文件行数为:"+lighLineCount+"，关闭旧文件，创建新文件继续写~");
                                    mLighFileName = "LIGH-"+Utils.getDateTimeString()+".csv";
                                    mLighFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mLighFileName;
                                    mLighWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mLighFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(LIGH_NAME, mLighFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            lighLineCount = 0;
                        }
                        break;
                    // 气压
                    case Sensor.TYPE_PRESSURE:
                        presLineCount++;
                        try{
                            long timeInMillis = System.currentTimeMillis();
                            mCurrentDate = new Date(timeInMillis);
                            SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                            mSensorEventTimestamp = date.format(mCurrentDate);
                            mPresWriter.write(String.format(Locale.US,mSensorEventTimestamp+",%.2f\n", event.values[0]));
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (presLineCount >= maxLines) {
                            if (mPresWriter != null) {
                                try {
                                    mPresWriter.close();
                                    Utils.debug("气压数据文件行数为:"+presLineCount+"，关闭旧文件，创建新文件继续写~");
                                    mPresFileName = "PRES-"+Utils.getDateTimeString()+".csv";
                                    mPresFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mPresFileName;
                                    mPresWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mPresFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(PRES_NAME, mPresFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            presLineCount = 0;
                        }
                        break;
                    // 计步数
                    case Sensor.TYPE_STEP_COUNTER:
                        if (stepCount == 0) {
                            stepCount = event.values[0];
                            break;
                        }
                        stepLineCount++;
                        try{
                            long timeInMillis = System.currentTimeMillis();
                            mCurrentDate = new Date(timeInMillis);
                            SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss", Locale.CHINA);
                            mSensorEventTimestamp = date.format(mCurrentDate);
                            mStepWriter.write(String.format(Locale.US,mSensorEventTimestamp+",%.2f\n", event.values[0]-stepCount));
                            stepCount = event.values[0];
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        if (stepLineCount >= maxLines) {
                            if (mStepWriter != null) {
                                try {
                                    mStepWriter.close();
                                    Utils.debug("步伐数据文件行数为:"+stepLineCount+"，关闭旧文件，创建新文件继续写~");
                                    mStepFileName = "STEP-"+Utils.getDateTimeString()+".csv";
                                    mStepFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mStepFileName;
                                    mStepWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mStepFileName));
                                    Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
                                    broadcastIntent.putExtra(STEP_NAME, mStepFileName);
                                    LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(SensorService.this);
                                    localBroadcastManager.sendBroadcast(broadcastIntent);
                                } catch(IOException e) {
                                    e.printStackTrace();
                                }
                            }
                            stepLineCount = 0;
                        }
                        break;
                }
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        // 主线程运行
        // 设置为前台Service
        Notification.Builder builder = new Notification.Builder(this); // 获取 Notification 构造器
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, new Intent(this, MainActivity.class), 0);
        builder.setContentIntent(contentIntent);
        builder.setSmallIcon(R.mipmap.ic_launcher);
        builder.setTicker("传感器记录"); //
        builder.setContentTitle("Log Sensors"); // 设置下拉列表标题
        builder.setContentText("记录传感器数据，前台任务，不能销毁"); // 设置上下文内容
        Notification notification = builder.build();
        startForeground(1, notification); // 开启前台服务

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);

        Sensor oriention = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
        Sensor linearAcceleration = mSensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION);
        Sensor light = mSensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
        Sensor gyroscope = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        Sensor magnetic = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        Sensor pressure = mSensorManager.getDefaultSensor(Sensor.TYPE_PRESSURE);
        Sensor stepCounter = mSensorManager.getDefaultSensor(Sensor.TYPE_STEP_COUNTER);

        // 打印设备中所有的传感器设备
        Utils.debug("打印设备中所有的传感器设备");
        List<Sensor> allSensors = mSensorManager.getSensorList(Sensor.TYPE_ALL);
        for (Sensor sensor : allSensors) {
            String msg = "名称： " + sensor.getName() + ", 版本： "+sensor.getVersion()+", 供应商："+sensor.getVendor()+", 类型： "+sensor.getType();
            Utils.debug(msg);
        }

        mSensorManager.registerListener(mSensorListener, linearAcceleration, 100000);
        mSensorManager.registerListener(mSensorListener, gyroscope, 100000);
        mSensorManager.registerListener(mSensorListener, magnetic, 100000);
        mSensorManager.registerListener(mSensorListener, oriention, 100000);

        mSensorManager.registerListener(mSensorListener, stepCounter, 10000000);
        mSensorManager.registerListener(mSensorListener, pressure, 10000000);
        mSensorManager.registerListener(mSensorListener, light, 10000000);

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Utils.debug("onStartCommand: Thread:"+Thread.currentThread().getName());
        if (!mLogging) {
            Utils.debug("onStartCommand, Thread id: "+Thread.currentThread().getId());

            mAcceFileName = "ACCE-"+Utils.getDateTimeString()+".csv";
            mAcceFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
            mGyroFileName = "GYRO-"+Utils.getDateTimeString()+".csv";
            mGyroFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
            mMagnFileName = "MAGN-"+Utils.getDateTimeString()+".csv";
            mMagnFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
            mOrieFileName = "ORIE-"+Utils.getDateTimeString()+".csv";
            mOrieFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
            mPresFileName = "PRES-"+Utils.getDateTimeString()+".csv";
            mPresFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
            mLighFileName = "LIGH-"+Utils.getDateTimeString()+".csv";
            mLighFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;
            mStepFileName = "STEP-"+Utils.getDateTimeString()+".csv";
            mStepFilePath = Utils.getStorageDirectory(SensorService.this)+"/"+mAcceFileName;

            mLogging = true;
            Utils.debug("开始记录, 文件名: "+mAcceFileName);
            Utils.debug("开始记录, 文件名: "+mGyroFileName);
            Utils.debug("开始记录, 文件名: "+mMagnFileName);
            Utils.debug("开始记录, 文件名: "+mOrieFileName);
            Utils.debug("开始记录, 文件名: "+mPresFileName);
            Utils.debug("开始记录, 文件名: "+mLighFileName);
            Utils.debug("开始记录, 文件名: "+mStepFileName);

            Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
            LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(this);

            broadcastIntent.putExtra(ACCE_NAME, mAcceFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);
            broadcastIntent.putExtra(GYRO_NAME, mGyroFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);
            broadcastIntent.putExtra(MAGN_NAME, mMagnFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);
            broadcastIntent.putExtra(ORIE_NAME, mOrieFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);
            broadcastIntent.putExtra(PRES_NAME, mPresFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);
            broadcastIntent.putExtra(LIGH_NAME, mLighFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);
            broadcastIntent.putExtra(STEP_NAME, mStepFileName);
            localBroadcastManager.sendBroadcast(broadcastIntent);

            try {
                mAcceWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mAcceFileName));
                mGyroWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mGyroFileName));
                mMagnWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mMagnFileName));
                mOrieWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mOrieFileName));
                mPresWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mPresFileName));
                mLighWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mLighFileName));
                mStepWriter = new FileWriter(new File(Utils.getStorageDirectory(SensorService.this), mStepFileName));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        return START_STICKY;
        //return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mLogging) {
            Intent broadcastIntent = new Intent(LOCAL_BROADCAST);
            LocalBroadcastManager localBroadcastManager = LocalBroadcastManager.getInstance(this);
            mLogging = false;
            if (mAcceWriter != null) {
                try {
                    mAcceWriter.close();
                    Utils.debug("关闭文件: "+mAcceFileName);
                    broadcastIntent.putExtra(ACCE_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mOrieWriter != null) {
                try {
                    mOrieWriter.close();
                    Utils.debug("关闭文件: "+mOrieFileName);
                    broadcastIntent.putExtra(ORIE_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mGyroWriter != null) {
                try {
                    mGyroWriter.close();
                    Utils.debug("关闭文件: "+mGyroFileName);
                    broadcastIntent.putExtra(GYRO_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mMagnWriter != null) {
                try {
                    mMagnWriter.close();
                    Utils.debug("关闭文件: "+mMagnFileName);
                    broadcastIntent.putExtra(MAGN_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mLighWriter != null) {
                try {
                    mLighWriter.close();
                    Utils.debug("关闭文件: "+mLighFileName);
                    broadcastIntent.putExtra(LIGH_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mPresWriter != null) {
                try {
                    mPresWriter.close();
                    Utils.debug("关闭文件: "+mPresFileName);
                    broadcastIntent.putExtra(PRES_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mStepWriter != null) {
                try {
                    mStepWriter.close();
                    Utils.debug("关闭文件: "+mStepFileName);
                    broadcastIntent.putExtra(STEP_NAME, "");
                    localBroadcastManager.sendBroadcast(broadcastIntent);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            if (mSensorManager != null) {
                mSensorManager.unregisterListener(mSensorListener);
            }
        }
    }

    public SensorService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }
}
