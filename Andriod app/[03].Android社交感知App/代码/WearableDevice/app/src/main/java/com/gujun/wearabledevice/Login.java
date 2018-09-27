package com.gujun.wearabledevice;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

public class Login extends AppCompatActivity {

    private EditText mAccount = null; // 账号编辑框
    private EditText mPassword = null; // 密码编辑框
    private CheckBox mRememberPassword = null; // 记住密码选择框
    private Button mLogin = null; // 登入按键
    private SharedPreferences mSharedPreferences = null;
    private SharedPreferences.Editor    mSharedPreferencesEditor = null;

    // SharedPreference存储的Keys
    private final String KEY_REMEMBER_PASSWORD = "RememberPassword";
    private final String KEY_ACCOUNT = "Account";
    private final String KEY_PASSWORD = "Password";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        mAccount = findViewById(R.id.account);
        mPassword = findViewById(R.id.password);
        mRememberPassword = findViewById(R.id.remember_password);
        mLogin = findViewById(R.id.login);

        mSharedPreferences = getPreferences(MODE_PRIVATE);// 自动将当前的活动作为SharedPreferences文件名
        boolean isRememberPassword = mSharedPreferences.getBoolean(KEY_REMEMBER_PASSWORD, false);
        // 如果勾选了记住密码选项则将保存在本地的账号和密码设置到文本框中
        if (isRememberPassword) {
            String account = mSharedPreferences.getString(KEY_ACCOUNT, "");
            String password = mSharedPreferences.getString(KEY_PASSWORD, "");
            mAccount.setText(account);
            mPassword.setText(password);
            mRememberPassword.setChecked(true);
        }
        // 登入按键时间
        mLogin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String account = mAccount.getText().toString();
                String password = mPassword.getText().toString();
                // 检查账号和密码
                if (account.equals("admin") && password.equals("123456")) {
                    mSharedPreferencesEditor = mSharedPreferences.edit();
                    if (mRememberPassword.isChecked()) {
                        mSharedPreferencesEditor.putBoolean(KEY_REMEMBER_PASSWORD, true);
                        mSharedPreferencesEditor.putString(KEY_ACCOUNT, account);
                        mSharedPreferencesEditor.putString(KEY_PASSWORD, password);
                    } else {
                        mSharedPreferencesEditor.clear();
                    }
                    mSharedPreferencesEditor.apply(); // 完成数据存储操作
                    Intent intent = new Intent(Login.this, MainActivity.class);
                    startActivity(intent);
                    finish();
                } else {
                    Toast.makeText(Login.this, "账号或密码错误", Toast.LENGTH_SHORT).show();
                    AlertDialog.Builder dialog = new AlertDialog.Builder(Login.this);
                    dialog.setTitle("登入失败");
                    dialog.setMessage("账号或密码错误，请重新输入。");
                    dialog.setPositiveButton("确定", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {

                        }
                    });
                    dialog.show();
                }
            }
        });
    }
}
