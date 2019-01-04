package java;
import java.util.Random;

public class RandomString {

    private RandomString() {
    }

    public static String generateString(int len) {
        String result = "";
        for (int i = 0; i < len; i++)
            result += (char) (33 + new Random(System.nanoTime()).nextInt(94));
        return result;
    }

    public static String generateString(int len, long seed) {
        String result = "";
        for (int i = 0; i < len; i++)
            result += (char) (33 + new Random(seed).nextInt(94));
        return result;
    }


    public static void main(String[] args) {
        String s = generateString(60);
        System.out.println(s);
        QRCode qr = new QRCode(1, s, false);
        qr.export("./", 2, "Random String QRCode");
    }
}