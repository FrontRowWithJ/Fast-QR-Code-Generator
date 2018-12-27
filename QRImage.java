import java.awt.image.BufferedImage;
import java.io.File;
import java.util.Arrays;
import java.io.IOException;
import javax.imageio.ImageIO;
import java.util.Random;

public class QRImage {
    private final static int BLACK = 0x00000000;
    private final static int WHITE = 0xFFFFFFFF;
    private final static int RED   = 0x00FF0000;

    public static void main(String... args) {
        Random r = new Random();
        int[][] qrdata = new int[50][50];
        for (int i = 0; i < qrdata[0].length; i++)
            for (int j = 0; j < qrdata.length; j++)
                qrdata[i][j] = r.nextInt();
        genQRImage(qrdata, 2, "./");
    }

    public static void genQRImage(int[][] QRData, int moduleSize, String directory) {
        try {
            int side = QRData.length;
            BufferedImage bi = new BufferedImage(side * moduleSize, side * moduleSize, BufferedImage.TYPE_INT_RGB);
            for (int i = 0; i < QRData.length; i++)
                for (int j = 0; j < QRData.length; j++) {
                    int[] cArray = new int[moduleSize * moduleSize];
                    Arrays.fill(cArray, QRData[i][j] == -1 ? RED : QRData[i][j] % 2 == 1 ? BLACK : WHITE);
                    bi.setRGB(j * moduleSize, i * moduleSize, moduleSize, moduleSize, cArray, 0, 0);
                }
            ImageIO.write(bi, "PNG", new File(directory + "QRImage.png"));
        } catch (Exception e) {
        }
    }
}