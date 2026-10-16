import java.util.Locale;

public class MatMul {

    public static double[][] generateMatrix(int n, long seed) {
        double[][] matrix = new double[n][n];
        long state = seed;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                state = state * 6364136223846793005L + 1442695040888963407L;
                long bits = state >>> 11;
                matrix[i][j] = bits * (1.0 / 9007199254740992.0);
            }
        }
        return matrix;
    }

    public static double[][] identityMatrix(int n) {
        double[][] matrix = new double[n][n];
        for (int i = 0; i < n; i++) {
            matrix[i][i] = 1.0;
        }
        return matrix;
    }

    public static void clearMatrix(double[][] C, int n) {
        for (int i = 0; i < n; i++) {
            double[] row = C[i];
            for (int j = 0; j < n; j++) {
                row[j] = 0.0;
            }
        }
    }

    public static void multiplyKernel(double[][] A, double[][] B, double[][] C, int n) {
        for (int i = 0; i < n; i++) {
            double[] aRow = A[i];
            double[] cRow = C[i];
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int k = 0; k < n; k++) {
                    sum += aRow[k] * B[k][j];
                }
                cRow[j] = sum;
            }
        }
    }

    public static double computeChecksum(double[][] C, int n) {
        double sum = 0.0;
        for (int i = 0; i < n; i++) {
            double[] row = C[i];
            for (int j = 0; j < n; j++) {
                sum += row[j];
            }
        }
        return sum;
    }

    public static double getUsedHeapKb() {
        Runtime rt = Runtime.getRuntime();
        return (rt.totalMemory() - rt.freeMemory()) / 1024.0;
    }

    public static void main(String[] args) {
        Locale.setDefault(Locale.US);
        if (args.length < 5) {
            System.exit(1);
        }
        String mode = args[0];
        int n = Integer.parseInt(args[1]);
        int warmup = Integer.parseInt(args[2]);
        int reps = Integer.parseInt(args[3]);
        long seed = Long.parseLong(args[4]);

        if (mode.equals("identity")) {
            double[][] A = generateMatrix(n, seed);
            double[][] B = identityMatrix(n);
            double[][] C = new double[n][n];
            multiplyKernel(A, B, C, n);
            double maxDiff = 0.0;
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    double diff = Math.abs(C[i][j] - A[i][j]);
                    if (diff > maxDiff) {
                        maxDiff = diff;
                    }
                }
            }
            System.out.printf("IDENTITY,%d,%.17e,%.17e%n", n, maxDiff, computeChecksum(C, n));
            return;
        }

        if (mode.equals("zero")) {
            double[][] A = generateMatrix(n, seed);
            double[][] B = new double[n][n];
            double[][] C = new double[n][n];
            multiplyKernel(A, B, C, n);
            double maxDiff = 0.0;
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    double diff = Math.abs(C[i][j]);
                    if (diff > maxDiff) {
                        maxDiff = diff;
                    }
                }
            }
            System.out.printf("ZERO,%d,%.17e,%.17e%n", n, maxDiff, computeChecksum(C, n));
            return;
        }

        if (mode.equals("dump")) {
            double[][] A = generateMatrix(n, seed);
            double[][] B = generateMatrix(n, seed + 1);
            double[][] C = new double[n][n];
            multiplyKernel(A, B, C, n);
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (i > 0 || j > 0) {
                        sb.append(",");
                    }
                    sb.append(String.format("%.17e", C[i][j]));
                }
            }
            System.out.println(sb.toString());
            return;
        }

        double[][] A = generateMatrix(n, seed);
        double[][] B = generateMatrix(n, seed + 1);
        double[][] C = new double[n][n];

        for (int w = 0; w < warmup; w++) {
            multiplyKernel(A, B, C, n);
        }

        for (int r = 1; r <= reps; r++) {
            clearMatrix(C, n);
            long t0 = System.nanoTime();
            multiplyKernel(A, B, C, n);
            long t1 = System.nanoTime();
            double elapsedS = (t1 - t0) / 1e9;
            double chk = computeChecksum(C, n);
            double memKb = getUsedHeapKb();
            System.out.printf("Java,%d,float64,%d,%.9f,%.2f,%.12e,1%n", n, r, elapsedS, memKb, chk);
        }
    }
}
