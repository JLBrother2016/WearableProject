package com.gujun.sqlite;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.widget.Toast;

/**
 * Created by gyguj on 2018/1/19.
 */

public class MyDatabaseHelper extends SQLiteOpenHelper{

    // BOOK 表， 存放书的各种详细数据
    public static final String CREATE_BOOK = "create table Book ("
            + "id integer primary key autoincrement, "
            + "author text, "
            + "price integer, "
            + "pages integer, "
            + "name text)";

    // Category 表，用于记录图书的分类
    public static final String CREATE_CATEGORY = "create table Category ("
            + "id integer primary key autoincrement, "
            + "category_name text, "
            + "category_code integer)";

    private Context mContext;

    public MyDatabaseHelper(Context context, String name,
                            SQLiteDatabase.CursorFactory factory, int version) {
        super(context, name, factory, version);
        mContext = context;
    }

    @Override
    public void onCreate(SQLiteDatabase sqLiteDatabase) {
        sqLiteDatabase.execSQL(CREATE_BOOK); // 创建 Book 表
        sqLiteDatabase.execSQL(CREATE_CATEGORY); // 创建 Category 表
        Toast.makeText(mContext, "Create succeeded", Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i1) {
        sqLiteDatabase.execSQL("drop table if exists Book"); // 发现表已存在则删除
        sqLiteDatabase.execSQL("drop table if exists Category");
        onCreate(sqLiteDatabase); // 再调用onCreate重新创建表
    }
}
