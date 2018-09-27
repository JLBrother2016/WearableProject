class DSPTest {
    public static void main(String [] args) {
        // Complex [] x = new Complex[8];
        // for (int i = 0; i < 8; ++i) {
        //     x[i] = new Complex(i+1, 0);
        // }
        // Complex [] y = FFT.fft(x);
        // FFT.show(x, "x");
        // Complex [] y_ = FFT.ifft(y);
        // FFT.show(y, "y");
        // FFT.show(y_, "y_");

        // TestGetHanningWindow();
        // TestGetHammingWindow();
        TestEnframe();
    }

    public static void TestGetHanningWindow() {
        double [] win = DSP.getHanningWindow(8);
        for (int i = 0; i < win.length; ++i) {
            System.out.println(win[i]);
        }
    }

    public static void TestGetHammingWindow() {
        double [] win = DSP.getHammingWindow(8);
        for (int i = 0; i < win.length; ++i) {
            System.out.println(win[i]);
        }
    }

    public static void TestEnframe() {
        double[] data = new double[256];
        for (int i = 0; i < 256; ++i) {
            data[i] = i+1;
        }
        double [][] dataFrame = DSP.enframe("hanning", data, 8, 0.5);
        for (int i = 0; i < dataFrame.length; ++i) {
            for (int j = 0; j < dataFrame[0].length; ++j) {
                System.out.print(dataFrame[i][j]+" ");
            }
            System.out.println(" ");
        }
    }
}