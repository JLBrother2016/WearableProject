package com.example.gujun.phonerecorder;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 启动 TelephoneListenerService 服务
        Intent i = new Intent(this, TelephoneListenerService.class);
        i.putExtra("CallNumber", "");
        Toast.makeText(this, "启动 TelephoneListenerService 服务~", Toast.LENGTH_SHORT).show();
        finish();
    }
}
