public class DCT {
 
    public static double [] dct(final double [] x) {
        int frameSize = x.length;
        double [] X = new double[frameSize];
        for (int k = 0; k < frameSize; ++k) {
            X[k] = 0.0;
            double a = k == 0 ? Math.sqrt(1.0/frameSize) : Math.sqrt(2.0/frameSize);
            for (int n = 0; n < frameSize; ++n) {  
                X[k] += a * x[n]*Math.cos(Math.PI*(2.0*n+1.0)*(double)k/(2.0*frameSize));
            }
        }
        return X;
    }

    public static double [] idct(final double [] X) {
        int frameSize = X.length;
        double [] x = new double[frameSize];
        for (int n = 0; n < frameSize; ++n) {
            x[n] = 0.0;
            for (int k = 0; k < frameSize; ++k) {
                double a = k == 0 ? Math.sqrt(1.0/frameSize) : Math.sqrt(2.0/frameSize);
                x[n] += a * X[k] * Math.cos(Math.PI*(2.0*n+1.0)*(double)k/(2.0*frameSize));
            }
        }
        return x;
    }

    // public static void main(String[] args) {
    //     double [] data = new double[8];
    //     for (int i = 0; i < 8; ++i) {
    //         data[i] = i+1;
    //     }

    //     double [] X = DCT.dct(data);
    //     for (int i = 0; i < X.length; ++i) {
    //         System.out.print(X[i]+" ");
    //     }
    //     System.out.println(" ");
    //     double [] x = DCT.idct(X);
    //     for (int i = 0; i < x.length; ++i) {
    //         System.out.print(x[i]+" ");
    //     }
    //     System.out.println(" ");
    // }
}