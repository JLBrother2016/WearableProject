package com.example.gujun.phonerecorder;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class TelephoneProtectService extends Service {
    private static final String TAG = "TelephoneProtectService";
    public TelephoneProtectService() {
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Intent i = new Intent(this, TelephoneListenerService.class);
        startService(i);
        Log.d(TAG, "TelephoneProtectService.守护进程");
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }
}
