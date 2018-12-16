import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;
import java.util.Random;

public class QRImage {
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
            BufferedImage bi = new BufferedImage(side * moduleSize, side * moduleSize, BufferedImage.TYPE_BYTE_BINARY);
            for (int i = 0; i < QRData.length; i++)
                for (int j = 0; j < QRData.length; j++) {
                    int[] cArray = new int[moduleSize * moduleSize];
                    for (int k = 0; k < cArray.length; k++)
                        cArray[k] = QRData[i][j] % 2 == 1 ? 0x00000000 : 0xFFFFFFFF;
                    bi.setRGB(j * moduleSize, i * moduleSize, moduleSize, moduleSize, cArray, 0, 0);
                }
            ImageIO.write(bi, "PNG", new File(directory + "QRImage.png"));
        } catch (Exception e) {
        }
    }
}