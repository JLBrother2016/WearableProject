package com.gujun.wearabledevice;

        import android.graphics.Color;
        import android.graphics.Paint;
        import android.support.v7.app.AppCompatActivity;
        import android.os.Bundle;
        import android.view.View;
        import android.widget.Button;
        import android.widget.LinearLayout;
        import android.widget.TextView;

        import org.achartengine.ChartFactory;
        import org.achartengine.chart.BarChart;
        import org.achartengine.chart.PointStyle;
        import org.achartengine.model.XYMultipleSeriesDataset;
        import org.achartengine.model.XYSeries;
        import org.achartengine.renderer.XYMultipleSeriesRenderer;
        import org.achartengine.renderer.XYSeriesRenderer;

public class DailyDataCheck extends AppCompatActivity {

    private View mChart;
    private View mChart2;

    private Button behaviorButton = null;
    private Button pressureButton = null;
    private Button speechButton = null;
    private TextView tv = null;

    private String [] mHours = new String[] {"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23"};
    private String [] mWeek = new String [] { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    private String [] mMonth = new String [] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_daily_data_check);
        Utils.debug("DailyDataCheck onCreate.");

        behaviorButton = findViewById(R.id.behavior);
        pressureButton = findViewById(R.id.pressure);
        speechButton = findViewById(R.id.speech);

//        tv = findViewById(R.id.text);
//        if (tv != null) {
//            tv.setText("4月20日社交特征图");
//        }

        behaviorButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

        pressureButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                pressureButton.setTextColor(Color.parseColor("#0000ff"));
                behaviorButton.setTextColor(Color.parseColor("#000000"));
                behaviorButton.setTextColor(Color.parseColor("#000000"));
                speechButton.setTextColor(Color.parseColor("#000000"));
                checkPressureData();
                checkLightData();
            }
        });

        behaviorButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                pressureButton.setTextColor(Color.parseColor("#000000"));
                behaviorButton.setTextColor(Color.parseColor("#000000"));
                behaviorButton.setTextColor(Color.parseColor("#000000"));
                speechButton.setTextColor(Color.parseColor("#000000"));
                checkStepData();
            }
        });
    }

    void checkPressureData() {
        Utils.debug("CheckPressureDate.");
        XYMultipleSeriesDataset seriesDataset = new XYMultipleSeriesDataset();
        //                        0:00     2:00    4:00    6:00   8:00    10:00    12:00  14:00   16:00   18:00  20:00  22:00
        //double [] pressures = {944.88, 943.80, 942.19, 942.38, 942.9, 942.77, 941.77, 937.33, 938.00, 938.65, 940.15, 939.41};
        //                         0       1       2        3      4        5       6        7      8       9      10      11      12      13       14      15      16     17       18     19       20      21     22    23
        double [] pressures =   {944.88, 944.34, 943.80, 942.99, 942.19, 942.25, 942.38, 942.64, 943.9, 942.83, 942.77, 941.27, 941.77, 939.55, 937.33, 937.66, 938.00, 938.33, 938.65, 939.40, 940.15, 939.78, 941.41, 941.22};
        XYSeries xySeries1 = new XYSeries("气压");
        for (int i = 0; i < 24; ++i) {
            xySeries1.add(i+1, pressures[i]);
        }

        seriesDataset.addSeries(xySeries1);

        // 描绘器，设置图表整体效果，比如 x, y 轴效果，缩放比例，颜色设置
        XYMultipleSeriesRenderer seriesRenderer = new XYMultipleSeriesRenderer();
        seriesRenderer.setChartTitleTextSize(100);                  // 设置图表标题的字体大小 (图的最上面文字)
        seriesRenderer.setMargins(new int[] { 100, 60, 200, 60 });  // 设置外边距，顺序为：上左下右
        seriesRenderer.setMarginsColor(Color.WHITE);//设置外边距空间的颜色
        seriesRenderer.setClickEnabled(false);
        seriesRenderer.setChartTitle("气压日变化趋势图");
        seriesRenderer.setChartTitleTextSize(50); // 标题字体大小

        //坐标轴设置
        seriesRenderer.setAxisTitleTextSize(50);// 设置坐标轴标题字体的大小
        seriesRenderer.setYAxisMin(930);        // 设置y轴的起始值
        seriesRenderer.setYAxisMax(950);        // 设置y轴的最大值
        seriesRenderer.setXAxisMin(0.5);        // 设置x轴起始值
        seriesRenderer.setXAxisMax(24.5);       // 设置x轴最大值
        seriesRenderer.setXTitle("时间/小时");   // 设置x轴标题
        seriesRenderer.setYTitle("压强/百帕斯卡");// 设置y轴标题
        //颜色设置
        seriesRenderer.setApplyBackgroundColor(true);   // 是应用设置的背景颜色
        seriesRenderer.setLabelsColor(0xFF85848D);      // 设置标签颜色
        seriesRenderer.setBackgroundColor(Color.argb(100, 255, 231, 224));// 设置图表的背景颜色
        //缩放设置
        seriesRenderer.setZoomButtonsVisible(false);// 设置缩放按钮是否可见
        seriesRenderer.setZoomEnabled(false);       // 图表是否可以缩放设置
        seriesRenderer.setZoomInLimitX(7);
//      seriesRenderer.setZoomRate(1);// 缩放比例设置
        //图表移动设置
        seriesRenderer.setPanEnabled(false);    // 图表是否可以移动

        //Legend(最下面的文字说明)设置
        seriesRenderer.setShowLegend(true);     // 控制legend（说明文字 ）是否显示
        //seriesRenderer.setLegendHeight(20);   // 设置说明的高度，单位px
        seriesRenderer.setLegendTextSize(40);   // 设置说明字体大小
        //seriesRenderer.setFitLegend(true);    // 设置这个后字体变得特变小
        //坐标轴标签设置
        seriesRenderer.setLabelsTextSize(40);   // 设置标签字体大小
        seriesRenderer.setXLabelsAlign(Paint.Align.CENTER);
        seriesRenderer.setYLabelsAlign(Paint.Align.LEFT);
        seriesRenderer.setXLabels(24);          // 显示的X轴标签的个数
        seriesRenderer.setYLabels(10);          // 显示的Y轴标签的个数

        seriesRenderer.clearXTextLabels();
        for (int i = 0; i < 24; ++i) {
            seriesRenderer.addXTextLabel(i+1, mHours[i]); //针对特定的x轴值增加文本标签
        }
        seriesRenderer.setXLabels(0);
        seriesRenderer.setPointSize(3); // 设置坐标点大小

        //某一组数据的描绘器，描绘该组数据的个性化显示效果，主要是字体跟颜色的效果
        XYSeriesRenderer xySeriesRenderer1 = new XYSeriesRenderer();
        xySeriesRenderer1.setAnnotationsColor(0xFFFF0000);  // 设置注释（注释可以着重标注某一坐标）的颜色
        xySeriesRenderer1.setPointStyle(PointStyle.POINT);  // 坐标点的显示风格
        xySeriesRenderer1.setPointStrokeWidth(1);           // 坐标点的大小
        xySeriesRenderer1.setColor(Color.BLUE);             // 表示该组数据的图或线的颜色
        xySeriesRenderer1.setDisplayChartValues(false);     // 设置是否显示坐标点的y轴坐标值
        xySeriesRenderer1.setChartValuesTextSize(20);       // 设置显示的坐标点值的字体大小
        xySeriesRenderer1.setLineWidth(3);                  // 设置线宽

        seriesRenderer.addSeriesRenderer(xySeriesRenderer1);
        LinearLayout chartContainer = findViewById(R.id.chart);
        chartContainer.removeAllViews();
        mChart = ChartFactory.getLineChartView(this, seriesDataset, seriesRenderer);
        chartContainer.addView(mChart);
    }

    void checkStepData() {
        XYMultipleSeriesDataset seriesDataset = new XYMultipleSeriesDataset();
        //                 0:00    2:00  4:00  6:00   8:00    10:00   12:00  14:00   16:00  18:00  20:00  22:00
        //                  0    1   2   3   4   5   6   7    8     9     10      11    12     13     14     15      16     17     18    19     20    21   22    23
        double [] steps = {200, 20, 10,  9,  3,  2,  14,210, 2283, 300,   100,   2000,  573,  110,  1932,   124,  200,    2100,   200,  100,   120,  120, 1900, 220};
        XYSeries xySeries1 = new XYSeries("步数");
        for (int i = 0; i < 24; ++i) {
            xySeries1.add(i+1, steps[i]);
        }

        seriesDataset.addSeries(xySeries1);

        // 描绘器，设置图表整体效果，比如 x, y 轴效果，缩放比例，颜色设置
        XYMultipleSeriesRenderer seriesRenderer = new XYMultipleSeriesRenderer();
        seriesRenderer.setChartTitleTextSize(100);                  // 设置图表标题的字体大小 (图的最上面文字)
        seriesRenderer.setMargins(new int[] { 100, 60, 200, 60 });  // 设置外边距，顺序为：上左下右
        seriesRenderer.setMarginsColor(Color.WHITE);//设置外边距空间的颜色
        seriesRenderer.setClickEnabled(false);
        seriesRenderer.setChartTitle("步数日变化趋势图");
        seriesRenderer.setChartTitleTextSize(50); // 标题字体大小

        //坐标轴设置
        seriesRenderer.setAxisTitleTextSize(50);// 设置坐标轴标题字体的大小
        seriesRenderer.setYAxisMin(0);        // 设置y轴的起始值
        seriesRenderer.setYAxisMax(3500);        // 设置y轴的最大值
        seriesRenderer.setXAxisMin(0.5);        // 设置x轴起始值
        seriesRenderer.setXAxisMax(24.5);       // 设置x轴最大值
        seriesRenderer.setXTitle("时间/小时");   // 设置x轴标题
        seriesRenderer.setYTitle("步数");       // 设置y轴标题
        //颜色设置
        seriesRenderer.setApplyBackgroundColor(true);   // 是应用设置的背景颜色
        seriesRenderer.setLabelsColor(0xFF85848D);      // 设置标签颜色
        seriesRenderer.setBackgroundColor(Color.argb(100, 255, 231, 224));// 设置图表的背景颜色
        //缩放设置
        seriesRenderer.setZoomButtonsVisible(false);// 设置缩放按钮是否可见
        seriesRenderer.setZoomEnabled(false);       // 图表是否可以缩放设置
        seriesRenderer.setZoomInLimitX(7);
//      seriesRenderer.setZoomRate(1);// 缩放比例设置
        //图表移动设置
        seriesRenderer.setPanEnabled(false);    // 图表是否可以移动

        //Legend(最下面的文字说明)设置
        seriesRenderer.setShowLegend(true);     // 控制legend（说明文字 ）是否显示
        //seriesRenderer.setLegendHeight(20);   // 设置说明的高度，单位px
        seriesRenderer.setLegendTextSize(40);   // 设置说明字体大小
        //seriesRenderer.setFitLegend(true);    // 设置这个后字体变得特变小
        //坐标轴标签设置
        seriesRenderer.setLabelsTextSize(40);   // 设置标签字体大小
        seriesRenderer.setXLabelsAlign(Paint.Align.CENTER);
        seriesRenderer.setYLabelsAlign(Paint.Align.LEFT);
        seriesRenderer.setXLabels(24);          // 显示的X轴标签的个数
        seriesRenderer.setYLabels(10);          // 显示的Y轴标签的个数

        seriesRenderer.clearXTextLabels();
        for (int i = 0; i < 24; ++i) {
            seriesRenderer.addXTextLabel(i+1, mHours[i]); //针对特定的x轴值增加文本标签
        }
        seriesRenderer.setXLabels(0);
        seriesRenderer.setPointSize(3); // 设置坐标点大小

        //某一组数据的描绘器，描绘该组数据的个性化显示效果，主要是字体跟颜色的效果
        XYSeriesRenderer xySeriesRenderer1 = new XYSeriesRenderer();
        xySeriesRenderer1.setAnnotationsColor(0xFFFF0000);  // 设置注释（注释可以着重标注某一坐标）的颜色
        xySeriesRenderer1.setPointStyle(PointStyle.POINT);  // 坐标点的显示风格
        xySeriesRenderer1.setPointStrokeWidth(1);           // 坐标点的大小
        xySeriesRenderer1.setColor(Color.CYAN);             // 表示该组数据的图或线的颜色
        xySeriesRenderer1.setDisplayChartValues(false);     // 设置是否显示坐标点的y轴坐标值
        xySeriesRenderer1.setChartValuesTextSize(20);       // 设置显示的坐标点值的字体大小
        xySeriesRenderer1.setLineWidth(3);                  // 设置线宽

        seriesRenderer.addSeriesRenderer(xySeriesRenderer1);
        LinearLayout chartContainer = findViewById(R.id.chart);
        chartContainer.removeAllViews();
        mChart = ChartFactory.getBarChartView(this, seriesDataset, seriesRenderer, BarChart.Type.DEFAULT);
        chartContainer.addView(mChart);
    }

    void checkLightData() {
        Utils.debug("checkLightData.");
        XYMultipleSeriesDataset seriesDataset = new XYMultipleSeriesDataset();
        //                        0:00     2:00    4:00    6:00   8:00    10:00    12:00  14:00   16:00   18:00  20:00  22:00
        //double [] pressures = {944.88, 943.80, 942.19, 942.38, 942.9, 942.77, 941.77, 937.33, 938.00, 938.65, 940.15, 939.41};
        //                         0       1       2        3      4        5       6        7      8       9      10      11      12      13       14      15      16     17       18     19       20      21     22    23
        double [] pressures =   {944.88, 944.34, 943.80, 942.99, 942.19, 942.25, 942.38, 942.64, 943.9, 942.83, 942.77, 941.27, 941.77, 939.55, 937.33, 937.66, 938.00, 938.33, 938.65, 939.40, 940.15, 939.78, 941.41, 941.22};
        XYSeries xySeries1 = new XYSeries("气压");
        for (int i = 0; i < 24; ++i) {
            xySeries1.add(i+1, pressures[i]);
        }

        seriesDataset.addSeries(xySeries1);

        // 描绘器，设置图表整体效果，比如 x, y 轴效果，缩放比例，颜色设置
        XYMultipleSeriesRenderer seriesRenderer = new XYMultipleSeriesRenderer();
        seriesRenderer.setChartTitleTextSize(100);                  // 设置图表标题的字体大小 (图的最上面文字)
        seriesRenderer.setMargins(new int[] { 100, 60, 200, 60 });  // 设置外边距，顺序为：上左下右
        seriesRenderer.setMarginsColor(Color.WHITE);//设置外边距空间的颜色
        seriesRenderer.setClickEnabled(false);
        seriesRenderer.setChartTitle("气压日变化趋势图");
        seriesRenderer.setChartTitleTextSize(50); // 标题字体大小

        //坐标轴设置
        seriesRenderer.setAxisTitleTextSize(50);// 设置坐标轴标题字体的大小
        seriesRenderer.setYAxisMin(930);        // 设置y轴的起始值
        seriesRenderer.setYAxisMax(950);        // 设置y轴的最大值
        seriesRenderer.setXAxisMin(0.5);        // 设置x轴起始值
        seriesRenderer.setXAxisMax(24.5);       // 设置x轴最大值
        seriesRenderer.setXTitle("时间/小时");   // 设置x轴标题
        seriesRenderer.setYTitle("光强/勒克斯");// 设置y轴标题
        //颜色设置
        seriesRenderer.setApplyBackgroundColor(true);   // 是应用设置的背景颜色
        seriesRenderer.setLabelsColor(0xFF85848D);      // 设置标签颜色
        seriesRenderer.setBackgroundColor(Color.argb(100, 255, 231, 224));// 设置图表的背景颜色
        //缩放设置
        seriesRenderer.setZoomButtonsVisible(false);// 设置缩放按钮是否可见
        seriesRenderer.setZoomEnabled(false);       // 图表是否可以缩放设置
        seriesRenderer.setZoomInLimitX(7);
//      seriesRenderer.setZoomRate(1);// 缩放比例设置
        //图表移动设置
        seriesRenderer.setPanEnabled(false);    // 图表是否可以移动

        //Legend(最下面的文字说明)设置
        seriesRenderer.setShowLegend(true);     // 控制legend（说明文字 ）是否显示
        //seriesRenderer.setLegendHeight(20);   // 设置说明的高度，单位px
        seriesRenderer.setLegendTextSize(40);   // 设置说明字体大小
        //seriesRenderer.setFitLegend(true);    // 设置这个后字体变得特变小
        //坐标轴标签设置
        seriesRenderer.setLabelsTextSize(40);   // 设置标签字体大小
        seriesRenderer.setXLabelsAlign(Paint.Align.CENTER);
        seriesRenderer.setYLabelsAlign(Paint.Align.LEFT);
        seriesRenderer.setXLabels(24);          // 显示的X轴标签的个数
        seriesRenderer.setYLabels(10);          // 显示的Y轴标签的个数

        seriesRenderer.clearXTextLabels();
        for (int i = 0; i < 24; ++i) {
            seriesRenderer.addXTextLabel(i+1, mHours[i]); //针对特定的x轴值增加文本标签
        }
        seriesRenderer.setXLabels(0);
        seriesRenderer.setPointSize(3); // 设置坐标点大小

        //某一组数据的描绘器，描绘该组数据的个性化显示效果，主要是字体跟颜色的效果
        XYSeriesRenderer xySeriesRenderer1 = new XYSeriesRenderer();
        xySeriesRenderer1.setAnnotationsColor(0xFFFF0000);  // 设置注释（注释可以着重标注某一坐标）的颜色
        xySeriesRenderer1.setPointStyle(PointStyle.POINT);  // 坐标点的显示风格
        xySeriesRenderer1.setPointStrokeWidth(1);           // 坐标点的大小
        xySeriesRenderer1.setColor(Color.BLUE);             // 表示该组数据的图或线的颜色
        xySeriesRenderer1.setDisplayChartValues(false);     // 设置是否显示坐标点的y轴坐标值
        xySeriesRenderer1.setChartValuesTextSize(20);       // 设置显示的坐标点值的字体大小
        xySeriesRenderer1.setLineWidth(3);                  // 设置线宽

        seriesRenderer.addSeriesRenderer(xySeriesRenderer1);
        LinearLayout chartContainer = findViewById(R.id.chart2);
        chartContainer.removeAllViews();
        mChart2 = ChartFactory.getLineChartView(this, seriesDataset, seriesRenderer);
        chartContainer.addView(mChart2);
    }

    void checkBehaviorData() {
        XYMultipleSeriesDataset seriesDataset = new XYMultipleSeriesDataset();
        XYSeries xySeries1 = new XYSeries("Short Time Energy");
        xySeries1.add(1, 944.88); // 0:00
        xySeries1.add(2, 944.22); // 2:00
        xySeries1.add(3, 943.80); // 4:00
        xySeries1.add(4, 943.06); // 8:00
        xySeries1.add(5, 942.19); // 10:00
        xySeries1.add(6, 942.26); // 12:00
        xySeries1.add(7, 938.65); // 14:00
        xySeries1.add(8, 944.15); // 16:00
        xySeries1.add(9, 30);  // 18:00
        xySeries1.add(10, 27); // 20:00
        xySeries1.add(11, 29); // 22:00
        xySeries1.add(12, 34); // 24:00
        seriesDataset.addSeries(xySeries1);
        XYSeries xySeries2 = new XYSeries("Zero Crossing Rate");
        xySeries2.add(1, 94);
        xySeries2.add(2, 22);
        xySeries2.add(3, 20);
        xySeries2.add(4, 21);
        xySeries2.add(5, 25);
        xySeries2.add(6, 22);
        xySeries2.add(7, 23);
        xySeries1.add(8, 36);
        xySeries1.add(9, 30);
        xySeries1.add(10, 27);
        xySeries1.add(11, 29);
        xySeries1.add(12, 34);
        seriesDataset.addSeries(xySeries2);

        // 描绘器，设置图表整体效果，比如 x, y 轴效果，缩放比例，颜色设置
        XYMultipleSeriesRenderer seriesRenderer = new XYMultipleSeriesRenderer();
        seriesRenderer.setChartTitleTextSize(100);                  // 设置图表标题的字体大小 (图的最上面文字)
        seriesRenderer.setMargins(new int[] { 100, 60, 200, 60 });  // 设置外边距，顺序为：上左下右
        seriesRenderer.setMarginsColor(Color.WHITE);//设置外边距空间的颜色
        seriesRenderer.setClickEnabled(false);
        seriesRenderer.setChartTitle("行为特征趋势图");
        seriesRenderer.setChartTitleTextSize(80); // 标题字体大小

        //坐标轴设置
        seriesRenderer.setAxisTitleTextSize(50);// 设置坐标轴标题字体的大小
        seriesRenderer.setYAxisMin(-100);        // 设置y轴的起始值
        seriesRenderer.setYAxisMax(100);         // 设置y轴的最大值
        seriesRenderer.setXAxisMin(0.5);        // 设置x轴起始值
        seriesRenderer.setXAxisMax(12.5);        // 设置x轴最大值
        seriesRenderer.setXTitle("Time");       // 设置x轴标题
        seriesRenderer.setYTitle("Amplitude");  // 设置y轴标题
        //颜色设置
        seriesRenderer.setApplyBackgroundColor(true);   // 是应用设置的背景颜色
        seriesRenderer.setLabelsColor(0xFF85848D);      // 设置标签颜色
        seriesRenderer.setBackgroundColor(Color.argb(100, 255, 231, 224));// 设置图表的背景颜色
        //缩放设置
        seriesRenderer.setZoomButtonsVisible(false);// 设置缩放按钮是否可见
        seriesRenderer.setZoomEnabled(false);       // 图表是否可以缩放设置
        seriesRenderer.setZoomInLimitX(7);
//      seriesRenderer.setZoomRate(1);// 缩放比例设置
        //图表移动设置
        seriesRenderer.setPanEnabled(false);// 图表是否可以移动

        //Legend(最下面的文字说明)设置
        seriesRenderer.setShowLegend(true);     // 控制legend（说明文字 ）是否显示
        seriesRenderer.setLegendHeight(30);     // 设置说明的高度，单位px
        seriesRenderer.setLegendTextSize(30);   // 设置说明字体大小
        //坐标轴标签设置
        seriesRenderer.setLabelsTextSize(40);   // 设置标签字体大小
        seriesRenderer.setXLabelsAlign(Paint.Align.CENTER);
        seriesRenderer.setYLabelsAlign(Paint.Align.LEFT);
        seriesRenderer.setXLabels(100);           // 显示的X轴标签的个数
        seriesRenderer.setYLabels(10);            // 显示的Y轴标签的个数
        seriesRenderer.addXTextLabel(1, "0:00");// 针对特定的x轴值增加文本标签
        seriesRenderer.addXTextLabel(2, "2:00");
        seriesRenderer.addXTextLabel(3, "4:00");
        seriesRenderer.addXTextLabel(4, "8:00");
        seriesRenderer.addXTextLabel(5, "10:00");
        seriesRenderer.addXTextLabel(6, "12:00");
        seriesRenderer.addXTextLabel(7, "14:00");
        seriesRenderer.addXTextLabel(8, "16:00");
        seriesRenderer.addXTextLabel(9, "18:00");
        seriesRenderer.addXTextLabel(10, "20:00");
        seriesRenderer.addXTextLabel(11, "12:00");
        seriesRenderer.addXTextLabel(12, "24:00");
        seriesRenderer.setPointSize(3); // 设置坐标点大小

        /*某一组数据的描绘器，描绘该组数据的个性化显示效果，主要是字体跟颜色的效果*/
        XYSeriesRenderer xySeriesRenderer1 = new XYSeriesRenderer();
        xySeriesRenderer1.setAnnotationsColor(0xFFFF0000);  // 设置注释（注释可以着重标注某一坐标）的颜色
        xySeriesRenderer1.setPointStyle(PointStyle.POINT);  // 坐标点的显示风格
        xySeriesRenderer1.setPointStrokeWidth(1);           // 坐标点的大小
        xySeriesRenderer1.setColor(0xFFF46C48);             // 表示该组数据的图或线的颜色
        xySeriesRenderer1.setDisplayChartValues(false);     // 设置是否显示坐标点的y轴坐标值
        xySeriesRenderer1.setChartValuesTextSize(20);       // 设置显示的坐标点值的字体大小

        /*某一组数据的描绘器，描绘该组数据的个性化显示效果，主要是字体跟颜色的效果*/
        XYSeriesRenderer xySeriesRenderer2 = new XYSeriesRenderer();
        xySeriesRenderer2.setPointStyle(PointStyle.POINT); // 坐标点的显示风格
        xySeriesRenderer2.setPointStrokeWidth(1);           // 坐标点的大小
        xySeriesRenderer2.setColor(0xFF00C8FF);             // 表示该组数据的图或线的颜色
        xySeriesRenderer2.setDisplayChartValues(false);     // 设置是否显示坐标点的y轴坐标值
        xySeriesRenderer2.setChartValuesTextSize(20);       // 设置显示的坐标点值的字体大小

        seriesRenderer.addSeriesRenderer(xySeriesRenderer1);
        seriesRenderer.addSeriesRenderer(xySeriesRenderer2);

        LinearLayout chartContainer = findViewById(R.id.chart);
        chartContainer.removeAllViews();
        mChart = ChartFactory.getLineChartView(this, seriesDataset, seriesRenderer);
        chartContainer.addView(mChart);
    }
}

