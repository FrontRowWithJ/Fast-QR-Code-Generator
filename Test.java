import java.util.Arrays;

public class Test implements QRConstants {
    public static void main(String[] args) {
        String s = "hello";
        String t = repeat(s, 5);
        System.out.println(t);
    }

    private static String repeat(String str, int count){
        return new String(new char[count]).replace("" + '\0', str);
    }

    public static int[] getBlockStructure(int QRVersion, int errorCodeLevel) {
        int numOfCodeWords = numOfCodeWords(QRVersion);
        int n = -1;
        int[] result = null;
        switch (errorCodeLevel) {
        case ECL_L:
            n = numOfCodeWords / BLOCK_COUNT_L[QRVersion - 1];
            result = new int[numOfCodeWords % BLOCK_COUNT_L[QRVersion - 1] == 0 ? 2 : 4];
            result[1] = (numOfCodeWords - NUM_OF_ERROR_CODES_L[QRVersion - 1]) / BLOCK_COUNT_L[QRVersion - 1];
            break;
        case ECL_M:
            n = numOfCodeWords / BLOCK_COUNT_M[QRVersion - 1];
            result = new int[numOfCodeWords % BLOCK_COUNT_M[QRVersion - 1] == 0 ? 2 : 4];
            result[1] = (numOfCodeWords - NUM_OF_ERROR_CODES_M[QRVersion - 1]) / BLOCK_COUNT_M[QRVersion - 1];
            break;
        case ECL_Q:
            n = numOfCodeWords / BLOCK_COUNT_Q[QRVersion - 1];
            result = new int[numOfCodeWords % BLOCK_COUNT_Q[QRVersion - 1] == 0 ? 2 : 4];
            result[1] = (numOfCodeWords - NUM_OF_ERROR_CODES_Q[QRVersion - 1]) / BLOCK_COUNT_Q[QRVersion - 1];
            break;
        case ECL_H:
            n = numOfCodeWords / BLOCK_COUNT_H[QRVersion - 1];
            result = new int[numOfCodeWords % BLOCK_COUNT_H[QRVersion - 1] == 0 ? 2 : 4];
            result[1] = (numOfCodeWords - NUM_OF_ERROR_CODES_H[QRVersion - 1]) / BLOCK_COUNT_H[QRVersion - 1];
        }
        if (n == -1)
            throw new IllegalArgumentException("Invalid error code level. Must be a value of 0, 1 or 2");
        result[0] = n;
        if (result.length == 4) {
            result[2] = result[0] + 1;
            result[3] = result[1] + 1;
        }
        return result;
    }

    private static Term[] genPolynomial(int errorCodeCount) {
        // index 0 == exponent of x
        // index 1 == α
        Term[][] terms = new Term[errorCodeCount][2];
        for (int i = 0; i < errorCodeCount; i++) {
            terms[i][0] = new Term(1, 0);
            terms[i][1] = new Term(0, i);
        }
        Term[] result = terms[0];
        for (int i = 1; i < errorCodeCount; i++) {
            result = Term.multiplyPolynomials(result, terms[i]);
            result = Term.simplify(result, i);
        }
        return result;
    }

    public static Term[] genErrorPolyNomial(int[] message) {
        Term[] messagePolynomial = Term.toPolynomialTemp(message);
        int numOfErrorCodewords = 10; // actual value is dependant on the number of QR version
        Term[] generatorPolynomial = genPolynomial(numOfErrorCodewords);
        messagePolynomial = Term.multiplyX(messagePolynomial, numOfErrorCodewords);
        int difference = messagePolynomial[0].getXExponent() - generatorPolynomial[0].getXExponent();
        Term[] shiftedGenPolynomial = Term.multiplyX(generatorPolynomial, difference);
        shiftedGenPolynomial = Term.multiplyAlpha(shiftedGenPolynomial, messagePolynomial[0].getAlphaExponent());
        Term[] errorPolynomial = Term.xor(messagePolynomial, shiftedGenPolynomial);
        for (int i = 0; i < messagePolynomial.length - 1; i++) {
            difference = errorPolynomial[0].getXExponent() - generatorPolynomial[0].getXExponent();
            shiftedGenPolynomial = Term.multiplyX(generatorPolynomial, difference);
            shiftedGenPolynomial = Term.multiplyAlpha(shiftedGenPolynomial, errorPolynomial[0].getAlphaExponent());
            errorPolynomial = Term.xor(errorPolynomial, shiftedGenPolynomial);
        }
        return errorPolynomial;
    }

    private static int numOfCodeWords(int QRVersion) {
        int qrWidth = 4 * QRVersion + 17;
        int codewords = qrWidth * qrWidth;
        codewords -= 3 * 64;
        codewords -= QRVersion > 6 ? 67 : 31;
        int alignmentPatterns = QRVersion == 1 ? 0 : ((2 + QRVersion / 7) * (2 + QRVersion / 7) - 3) * 25;
        codewords -= alignmentPatterns;
        int timingPattern = 2 * (qrWidth - 16) - (QRVersion / 7) * 2 * 5;
        codewords -= timingPattern;
        return codewords / 8;
    }
}