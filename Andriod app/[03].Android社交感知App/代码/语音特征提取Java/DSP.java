public class DSP {

    public static double [] getHanningWindow(int frameSize) {
        // 标准的 hanning 窗
        // double [] win = new double[frameSize];
        // for (int i = 0; i < frameSize; ++i) {
        //     win[i] = 0.5*(1.0-Math.cos(2.0*Math.PI*i / (frameSize-1.0)));
        // }

        // Matlab 的 hanning窗去掉了首尾的两个0
        double [] win = new double[frameSize];

        for (int i = 1; i < frameSize+1; ++i) {
            win[i-1] = 0.5*(1.0-Math.cos(2.0*Math.PI*i / (frameSize+1.0)));
        }

        return win;
    }

    public static double [] getHammingWindow(int frameSize) {
        double [] win = new double[frameSize];

        for (int i = 0; i < frameSize; ++i) {
            win[i] = 0.54 - 0.46*Math.cos(2.0*Math.PI*i / (frameSize-1.0));
        }

        return win;
    }

    public static double [][] enframe(String window, double [] data, int frameSize, double overlap) {
        int frameStep = (int) (frameSize * (1.0-overlap));
        int length = data.length;
        int frameNum = (length - frameSize) / frameStep + 1;
        double [][] ret = new double[frameNum][frameSize];
        double [] win;

        // 选择窗函数
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

    public static double [] getShortTimeEnergy(double [][] dataFrames) {
        int frameNum = dataFrames.length;         // 信号总帧数
        int frameSize = dataFrames[0].length;     // 一帧数据大小
        double [] ste = new double[frameNum];

        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            double sum = 0.0;
            for (int offset = 0; offset < frameSize; ++offset) {
                sum += dataFrames[frameIndex][offset] * dataFrames[frameIndex][offset];
            }
            ste[frameIndex] = sum;
        }

        return ste;
    }

    public static int [] getShortTimeZeroCrossingRate(double [][] dataFrames) {
        int frameNum = dataFrames.length;         // 信号总帧数
        int frameSize = dataFrames[0].length;     // 一帧数据大小
        int [] zcr = new int[frameNum];

        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            int sum = 0;
            for (int offset = 1; offset < frameSize; ++offset) {
                if (dataFrames[frameIndex][offset-1] * dataFrames[frameIndex][offset] < 0) {
                    ++sum;
                }
            }
            zcr[frameIndex] = sum;
        }

        return zcr;
    }

    public static double [] getSpectralEntropy(double [][] dataFrames) {
        int frameNum = dataFrames.length;         // 信号总帧数
        int frameSize = dataFrames[0].length;     // 一帧数据大小
        double [] entropy = new double[frameNum];

        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            Complex [] x = new Complex[frameSize];
            // 构造FFT输入
            for (int i = 0; i < frameSize; ++i) {
                x[i] = new Complex(dataFrames[frameIndex][i], 0);
            }
            // FFT的输出
            Complex [] y = FFT.fft(x);
            // 每个频率的幅值能量，只取正频率部分
            int halfFrameSize = frameSize / 2;
            double [] energy = new double[halfFrameSize];
            double energySum = 0.0; // 总能量
            for (int i = 0; i < halfFrameSize; ++i) {
                energy[i] = y[i].abs() * y[i].abs();
                energySum += energy[i];
            }
            double [] p = new double[halfFrameSize];
            for (int i = 0; i < halfFrameSize; ++i) {
                p[i] = energy[i] / energySum;
            }
            entropy[frameIndex] = 0.0;
            for (int i = 0; i < halfFrameSize; ++i) {
                entropy[frameIndex] -= p[i] * log2(p[i]);
            }
        }

        return entropy;
    }

    public static double [] getMaxPeak(double [][] dataFrames) {
        int frameNum = dataFrames.length;         // 信号总帧数
        int frameSize = dataFrames[0].length;     // 一帧数据大小
        double [] maxPeak = new double[frameNum];

        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            double peak = 0.0;
            for (int offset = 0; offset < frameSize; ++offset) {
                if (dataFrames[frameIndex][offset] > peak) {
                    peak = dataFrames[frameIndex][offset];
                }
            }
            maxPeak[frameIndex] = peak;
        }

        return maxPeak;
    }

    public static double [] getBrightness(final double [][] dataFrames, int sampleRate) {
        int frameNum = dataFrames.length;         // 信号总帧数
        int frameSize = dataFrames[0].length;     // 一帧数据大小
        int Fn = sampleRate / frameSize;          // 分辨率频率
        double [] brightness = new double[frameNum];

        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            Complex [] x = new Complex[frameSize];
            // 构造FFT输入
            for (int i = 0; i < frameSize; ++i) {
                x[i] = new Complex(dataFrames[frameIndex][i], 0);
            }
            // FFT的输出
            Complex [] y = FFT.fft(x);
            // 每个频率的幅值能量
            int halfFrameSize = frameSize/2;
            double [] energy = new double[halfFrameSize];
            double energySum = 0.0; // 总能量
            double sum = 0.0;
            for (int i = 0; i < halfFrameSize; ++i) {
                energy[i] = y[i].abs() * y[i].abs();
                sum += energy[i] * i * Fn;
                energySum += energy[i];
            }
            brightness[frameIndex] = sum / energySum;
        }

        return brightness;
    }
 
    public static double [][] getFormant(String window, double [] data, int frameSize, double overlap, int sampleRate) {
        int length = data.length;
        double [] data_ = new double[length];
        int Fn = sampleRate / frameSize;

        // 1. 预加重，高通滤波
        data_[0] = data[0];
        for (int i = 1; i < length; ++i) {
            data_[i] = data[i] - 0.99 * data[i-1];
        }

        // 2. 加窗、分帧
        double [][] dataFrames = enframe("hamming", data_, frameSize, overlap);
        int frameNum = dataFrames.length;         // 信号总帧数
        double [][] formant = new double [frameNum][8];

        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            Complex [] x = new Complex[frameSize];

            for (int i = 0; i < frameSize; ++i) {
                x[i] = new Complex(dataFrames[frameIndex][i], 0);
            }
            // 3. FFT
            Complex [] y = FFT.fft(x);

            int halfFrameSize = frameSize / 2;
            Complex [] a = new Complex[halfFrameSize];
            for (int i = 0; i < halfFrameSize; ++i) {
                a[i] = new Complex(Math.log(y[i].abs()), 0);
            }
            // 4. IFFT
            Complex [] b = FFT.ifft(a);
            Complex zero = new Complex(0, 0);
            // 5. 加矩形窗
            for (int i = 6; i < 123; ++i) {
                b[i] = b[i].times(zero);
            }
            // 6. FFT
            Complex [] h = FFT.fft(b);
            double [] r = new double[halfFrameSize];
            for (int i = 0; i < halfFrameSize; ++i) {
                r[i] = h[i].re();
            }
            // 7. 寻找极大值
            int j = 0;
            for (int i = 1; i < halfFrameSize-1; ++i) {
                if (r[i] > r[i]-1 && r[i] > r[i+1]) {
                    formant[frameIndex][j++] = r[i] * i * Fn;
                }
            }
        }
        return formant;
    }

    public static double [] getMFCC(double [] data, int frameSize, double overlap, int sampleRate, int p) {
        int length = data.length;
        double [] data_ = new double[length];
        int Fn = sampleRate / frameSize;

        // 1. 预加重，高通滤波
        data_[0] = data[0];
        for (int i = 1; i < length; ++i) {
            data_[i] = data[i] - 0.9375 * data[i-1];
        }

        // 2. 加窗、分帧
        double [][] dataFrames = enframe("hamming", data_, frameSize, overlap);
        int frameNum = dataFrames.length;         // 信号总帧数
        double mfcc = new double [16];


        for (int frameIndex = 0; frameIndex < frameNum; ++frameIndex) {
            Complex [] x = new Complex[frameSize];
            for (int i = 0; i < frameSize; ++i) {
                x[i] = new Complex(dataFrames[frameIndex][i], 0);
            }
            // 3. FFT
            Complex [] y = FFT.fft(x);
            // 4. 计算谱线能量
            int halfFrameSize = frameSize / 2;
            Complex [] energy = new Complex[halfFrameSize];
            for (int i = 0; i < halfFrameSize; ++i) {
                energy[i] = y[i].abs() * y[i].abs()
            }
        }

        return mfcc;

    }

    static double log2(double x) {
        return Math.log(x) / Math.log(2.0);
    }
}