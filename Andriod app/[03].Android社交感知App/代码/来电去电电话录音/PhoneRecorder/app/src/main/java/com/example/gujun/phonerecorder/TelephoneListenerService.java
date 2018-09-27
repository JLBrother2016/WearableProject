package com.example.gujun.phonerecorder;

import android.app.Service;
import android.content.Intent;
import android.media.MediaRecorder;
import android.os.Environment;
import android.os.IBinder;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

public class TelephoneListenerService extends Service {

    private static final String TAG = "TelephoneService";
    private TelephonyManager    mTelephonyManager; // 电话管理器
    private MyListener          mMyListener; // 监听器对象
    private MediaRecorder       mMediaRecorder; // 录音机
    private String              mCallNumber;

    public TelephoneListenerService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }

    @Override // 服务创建的时候调用的方法
    public void onCreate() {
        super.onCreate();
        // 后台监听电话的呼叫状态。
        // 得到电话管理器
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mMyListener = new MyListener();
        mTelephonyManager.listen(mMyListener, PhoneStateListener.LISTEN_CALL_STATE);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mCallNumber = intent.getStringExtra("CallNumber");
        Log.d(TAG, "onStartCommand"+mCallNumber);
        return super.onStartCommand(intent, flags, startId);
    }

    private class MyListener extends PhoneStateListener {
        String inComingNumber;

        @Override // 当电话的呼叫状态发生变化的时候调用该方法
        public void onCallStateChanged(int state, String incomingNumber) {
            super.onCallStateChanged(state, incomingNumber);
            try {
                switch (state) {
                    case TelephonyManager.CALL_STATE_IDLE: // 空闲状态
                        Log.d(TAG, "========== 空闲状态 ==========");
                        if (mMediaRecorder != null) {
                            //8.停止捕获
                            mMediaRecorder.stop();
                            //9.释放资源
                            mMediaRecorder.release();
                            mMediaRecorder = null;
                            //TODO 这个地方你可以将录制完毕的音频文件上传到服务器，这样就可以监听了
                            Log.d("TelephoneService", "音频文件录制完毕，可以在后台将文件上传到服务器~");
                        }
                        break;
                    case TelephonyManager.CALL_STATE_RINGING: // 零响状态
                        inComingNumber = incomingNumber;
                        Log.d(TAG, "========== 响铃状态 ==========\n来电号码： " + incomingNumber);
                        break;

                    case TelephonyManager.CALL_STATE_OFFHOOK: //通话状态
                        Log.d(TAG, "========== 通话状态 ==========");
                        // 开始录音
                        // 1.实例化一个录音机
                        mMediaRecorder = new MediaRecorder();
                        Log.d(TAG, "实例化一个录音机");
                        // 2.指定录音机的声音源
                        // mediaRecorder.setAudioSource(MediaRecorder.AudioSource.VOICE_CALL);
                        mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
                        Log.d(TAG, "指定录音机的声音源");
                        // 3.设置录制的文件输出的格式
                        mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.DEFAULT);
                        Log.d(TAG, "设置录制的文件输出的格式");
                        long time = System.currentTimeMillis();
                        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                        Date date = new Date(time);
                        String time1=format.format(date);
                        // 4.指定录音文件的名称
                        File file;
                        if ("".equals(mCallNumber )) {
                            Log.i(TAG, "来电电话：" + inComingNumber);
                            file = new File(Environment.getExternalStorageDirectory(),inComingNumber+"-"+time1+".3gp");
                        } else {
                            Log.i(TAG, "拨出电话：" + mCallNumber);
                            file = new File(Environment.getExternalStorageDirectory(),mCallNumber+"-"+time1+".3gp");
                            mCallNumber = "";
                        }
                        if (file != null) {
                            Log.i(TAG, "指定录音文件的名称");
                        }
                        mMediaRecorder.setOutputFile(file.getAbsolutePath());
                        // 5.设置音频的编码
                        mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
                        Log.d(TAG, "设置音频的编码");
                        // 6.准备开始录音
                        mMediaRecorder.prepare();
                        Log.d(TAG, "准备开始录音");
                        // 7.开始录音
                        mMediaRecorder.start();
                        Log.d(TAG, "开始录音");
                        break;

                    default:
                        break;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 服务销毁调用的方法
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        // 取消电话的监听,采取线程守护的方法，当一个服务关闭后，开启另外一个服务，除非你很快把两个服务同时关闭才能完成
        Intent i = new Intent(this,TelephoneProtectService.class);
        startService(i);
        mMyListener = null;
    }
}
