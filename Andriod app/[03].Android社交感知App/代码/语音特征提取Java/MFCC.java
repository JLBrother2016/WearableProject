class MFCC {
    private final static int    nunMelFilters       = 30;
    private final static double preEmphasisAlpha    = 0.95;
    private final static double lowerFilterFreq     = 80.0

    private final double    sampleRate;
    private final double    upperFilterFreq;
    private final int       samplesPerFrame;

    private final boolean usePreEmphasis;

    public MFCC(int samplesPerFrame, double sampleRate, int numCoefficients, boolean preEmphasis) {
        
        this.samplesPerFrame = samplesPerFrame;
        this.sampleRate = sampleRate;
        this.usePreEmphasis = preEmphasis;
        upperFilterFreq = sampleRate / 2.0;
    }

    // 预加重
    public double [] preEmphasis(double [] data) {
        int length = data.length;
        double [] data_ = new double[length];
        data_[0] = data[0];

        for (int i = 1; i < length; ++i) {
            data_[i] = data[i] - preEmphasisAlpha*data[i-1];
        }

        return data_;
    }

    public int [] fftBinIndices() {
        int cBin[] = new[nunMelFilters+2];
        cBin[0] = (int)Math.rou
    }
}