public class Window {

    public static float [] hanning(int N) {

        int half = 0;
        float [] win = new float[N];

        if (N % 2 == 0) {
            half = N / 2;
            for (int i = 0; i <= half; ++i) {
                win[i-1] = 0.5 - 0.5*Math.cos(Math.PI * i / (N+1.0));
            }
            int index = half+1;
            for (int i = half; i >= 1; --i) {
                win[index-1] = win[i-1];
                index++;
            }
        } else {
            half = (N+1) / 2;
            for (int i = 1; i <= half; ++i) {
                win[i-1] = 0.5 - 0.5*Math.cos(Math.PI * i / (N+1.0));
            }
            int index = half+1;
            for (int i = half-1; i >= 1; --i) {
                win[index-1] = win[i-1];
                index++;
            }
        }
    }

    public static float [] hamming(int N) {

        int half = 0;
        if (N % 2 == 0) {
            half = N / 2;
            for (int i = 1; i <= half; ++i) {
                
            }
        }

    }
}