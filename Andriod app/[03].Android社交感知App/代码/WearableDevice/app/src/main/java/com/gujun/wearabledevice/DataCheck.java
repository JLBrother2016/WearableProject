package com.gujun.wearabledevice;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import org.achartengine.ChartFactory;
import org.achartengine.chart.BarChart;
import org.achartengine.chart.PointStyle;
import org.achartengine.model.XYMultipleSeriesDataset;
import org.achartengine.model.XYSeries;
import org.achartengine.renderer.XYMultipleSeriesRenderer;
import org.achartengine.renderer.XYSeriesRenderer;

public class DataCheck extends AppCompatActivity {

    private View mChart;
    private String [] mHour = new String [] { "00:00-06:00", "06:00-12:00", "12:00-18:00", "18:00-24:00"};

    private String [] mWeek = new String [] { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    private String [] mMonth = new String [] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_data_check);

        Button dayDataCheckButton = findViewById(R.id.day);
        dayDataCheckButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent dailyCheckIntent = new Intent(DataCheck.this, DailyDataCheck.class);
                startActivity(dailyCheckIntent);
            }
        });

        Button weekDataCheckButton = findViewById(R.id.week);
        weekDataCheckButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

        Button monthDataCheckButton = findViewById(R.id.month);
        monthDataCheckButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

        Button yearDataCheckButton = findViewById(R.id.year);
        yearDataCheckButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                checkOneYearData();
            }
        });
    }


    // 年柱状图
    void checkOneYearData() {
        int [] x = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        int [] activities = new int [12];
        for (int i = 0; i < 12; ++i) {
            activities[i] = (int)(100 * Math.random());
        }

        // 创建 XYSeries
        XYSeries incomeSeries = new XYSeries("运动量");

        // 加入数据到 XYSeries
        for (int i = 0; i < x.length; ++i) {
            incomeSeries.add(i, activities[i]);
        }

        // 创建dataSet，并且把XYSeries加入到dataSet
        XYMultipleSeriesDataset dataSet = new XYMultipleSeriesDataset();
        dataSet.addSeries(incomeSeries);

        // 创建XYSeriesRenderer 来自定义incomeRenderer信息
        XYSeriesRenderer incomeRenderer = new XYSeriesRenderer();
        incomeRenderer.setColor(Color.CYAN);
        incomeRenderer.setFillPoints(true);
        incomeRenderer.setLineWidth(5);
        incomeRenderer.setDisplayChartValues(true);
        incomeRenderer.setDisplayChartValuesDistance(10);

        // 创建XYMultipleSeriesRenderer 来自定义整个柱状图的信息
        XYMultipleSeriesRenderer multiRenderer = new XYMultipleSeriesRenderer();
        multiRenderer.setOrientation(XYMultipleSeriesRenderer.Orientation.HORIZONTAL); // 柱状图竖着
        multiRenderer.setXLabels(0);
        multiRenderer.setChartTitle("年运动量统计");             // 标题
        multiRenderer.setXTitle("Year 2017");                   // X坐标轴标题
        //multiRenderer.setYTitle("Amount in Dollars");         // Y坐标轴标题
        multiRenderer.setChartTitleTextSize(50);                // 标题字体大小
        multiRenderer.setAxisTitleTextSize(40);                 // 坐标轴标题字体大小
        //setting text size of the graph lable
        multiRenderer.setLabelsTextSize(30);                    // 坐标轴刻度字体大小
        //setting zoom buttons visiblity
        multiRenderer.setZoomButtonsVisible(false);
        //setting pan enablity which uses graph to move on both axis
        multiRenderer.setPanEnabled(false, false);
        //setting click false on graph
        multiRenderer.setClickEnabled(false);
        //setting zoom to false on both axis
        multiRenderer.setZoomEnabled(false, false);
        //setting lines to display on y axis
        multiRenderer.setShowGridY(false);
        //setting lines to display on x axis
        multiRenderer.setShowGridX(false);
        //setting legend to fit the screen size
        multiRenderer.setFitLegend(true);
        //setting displaying line on grid
        multiRenderer.setShowGrid(false);
        //setting zoom to false
        multiRenderer.setZoomEnabled(false);
        //setting external zoom functions to false
        multiRenderer.setExternalZoomEnabled(false);
        //setting displaying lines on graph to be formatted(like using graphics)
        multiRenderer.setAntialiasing(true);
        //setting to in scroll to false
        multiRenderer.setInScroll(false);
        //setting to set legend height of the graph
        multiRenderer.setLegendHeight(40);
        multiRenderer.setLegendTextSize(40); // 设置图例的字体大小
        //setting x axis label align
        multiRenderer.setXLabelsAlign(Paint.Align.CENTER);
        //setting y axis label to align
        multiRenderer.setYLabelsAlign(Paint.Align.LEFT);
        //setting text style
        multiRenderer.setTextTypeface("sans_serif", Typeface.NORMAL);
        //setting no of values to display in y axis
        multiRenderer.setYLabels(20);
        // setting y axis max value, Since i'm using static values inside the graph so i'm setting y max value to 4000.
        // if you use dynamic values then get the max y value and set here
        multiRenderer.setYAxisMax(100);
        //setting used to move the graph on xaxiz to .5 to the right
        multiRenderer.setXAxisMin(-0.5);
        //setting max values to be display in x axis
        multiRenderer.setXAxisMax(11);
        //setting bar size or space between two bars
        multiRenderer.setBarSpacing(0.5);
        //Setting background color of the graph to transparent
        multiRenderer.setBackgroundColor(Color.TRANSPARENT);
        //Setting margin color of the graph to transparent
        multiRenderer.setMarginsColor(getResources().getColor(R.color.transparent_background));
        multiRenderer.setApplyBackgroundColor(true);
        multiRenderer.setMargins(new int[] {50, 50, 50, 50}); // 设置布局上下左右的 Margins
        for (int i = 0; i< x.length; i++) {
            multiRenderer.addXTextLabel(i, mMonth[i]);
        }
        // Adding incomeRenderer and expenseRenderer to multipleRenderer
        // Note: The order of adding data series to dataSet and renderers to multipleRenderer should be same
        multiRenderer.addSeriesRenderer(incomeRenderer);
        // this part is used to display graph on the xml
        LinearLayout chartContainer = findViewById(R.id.chart);
        // remove any views before u paint the chart
        chartContainer.removeAllViews();
        // drawing bar chart
        mChart = ChartFactory.getBarChartView(DataCheck.this, dataSet, multiRenderer, BarChart.Type.DEFAULT);
        // adding the view to the linearlayout
        chartContainer.addView(mChart);
    }
}
