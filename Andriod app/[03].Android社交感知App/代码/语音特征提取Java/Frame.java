public class Frame {
    public static void main(String [] args) {
        double [] data = new double[8 * 10];
        for (int i = 0; i < 8 * 10; ++i) {
            data[i] = 1.0;
        }

        double [][] ret = applyWindow("hanning", data, 8, 0.5);
        display(ret);
    }


    public static void display(double [][] ret) {
        for (int i = 0; i < ret.length; ++i) {
            for (int j = 0; j < ret[0].length; ++j) {
                System.out.print(ret[i][j]+" ");
            }
            System.out.println(" ");
        }
    }
    public static double [][] applyWindow(String window, double [] data, int frameSize, double overlap) {

        int frameStep = (int) (frameSize * (1.0-overlap));
        int length = data.length;
        int frameNum = (length - frameSize) / frameStep + 1;
        double [][] ret = new double[frameNum][frameSize];
        double [] win;

        if (window.equals("hanning")) {
            win = getHanningWindow(frameSize);
        } else if (window.equals("hamming")) {
            win = getHammingWindow(frameSize);
        } else {
            throw new IllegalArgumentException("No such window: "+window);
        }

        // 分帧
        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            for (int offset = 0; offset < frameSize; ++offset) {
                ret[frameIndex][offset] = data[offset+frameIndex*frameStep];
            }
        }
        // 加窗
        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            for (int offset = 0; offset < frameSize; ++offset) {
                ret[frameIndex][offset] *= win[offset];
            }
        }

        return ret;
    }

    // Matlab hanning窗去掉了收尾的两个0
    public static double [] getHanningWindow(int frameSize) {
        // 标准的 hanning 窗
        // double [] win = new double[frameSize];
        // for (int i = 0; i < frameSize; ++i) {
        //     win[i] = 0.5*(1.0-Math.cos(2.0*Math.PI*i / (frameSize-1.0)));
        // }
        // Matlab 的 hanning 窗
        double [] win = new double[frameSize];
        for (int i = 1; i < frameSize+1; ++i) {
            win[i-1] = 0.5*(1.0-Math.cos(2.0*Math.PI*i / (frameSize+1.0)));
        }
        return win;
    }

    // 跟 Matlab 一致
    public static double [] getHammingWindow(int frameSize) {
        double [] win = new double[frameSize];
        for (int i = 0; i < frameSize; ++i) {
            win[i] = 0.54 - 0.46*Math.cos(2.0*Math.PI*i / (frameSize-1.0));
        }
        return win;
    }
};