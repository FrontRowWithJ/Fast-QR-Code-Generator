import java.util.ArrayList;

public class Term {
    private static int[] ALPHA_VALUE_EXPONENT = new int[257];
    private final static int[] ALPHA_VALUE = new int[257];
    private final static String[] SUPERSCRIPT = { "\u2070", "\u00B9", "\u00B2", "\u00b3", "\u2074", "\u2075", "\u2076",
            "\u2077", "\u2078", "\u2079" };
    // x refers to the exponent of x
    private int xExponent;
    // alpha refers to the exponent of α
    private int alphaExponent;
    static {
        for (int i = 0; i < 255; i++) {
            int alpha = getAlpha(i);
            ALPHA_VALUE_EXPONENT[alpha] = i;
            ALPHA_VALUE[i] = alpha;
        }
        ALPHA_VALUE[255] = 1;
        ALPHA_VALUE_EXPONENT[0] = 256;
    }

    public Term(int xExponent, int alphaExponent) {
        this.xExponent = xExponent;
        this.alphaExponent = alphaExponent;
    }

    public Term multiplyTerms(Term term) {
        return new Term(xExponent + term.xExponent, (alphaExponent + term.alphaExponent) % 255);
    }

    public static Term[] multiplyPolynomials(Term[] poly1, Term[] poly2) {
        Term[] result = new Term[poly1.length * poly2.length];
        for (int i = 0; i < poly1.length; i++)
            for (int j = 0; j < poly2.length; j++)
                result[i * poly2.length + j] = poly1[i].multiplyTerms(poly2[j]);
        return result;
    }

    public static Term[] multiplyX(Term[] polynomial, int x) {
        Term[] result = new Term[polynomial.length];
        for (int i = 0; i < polynomial.length; i++)
            result[i] = new Term(polynomial[i].xExponent + x, polynomial[i].alphaExponent);
        return result;
    }

    public static Term[] multiplyAlpha(Term[] polynomial, int alpha) {
        Term[] result = new Term[polynomial.length];
        for (int i = 0; i < polynomial.length; i++)
            result[i] = new Term(polynomial[i].xExponent, (polynomial[i].alphaExponent + alpha) % 255);
        return result;
    }

    public static Term[] xor(Term[] poly1, Term[] poly2) {
        int limit = Math.min(poly1.length, poly2.length);
        int max = Math.max(poly1.length, poly2.length);
        Term[] result = new Term[max - 1];
        int i = 0;
        for (; i < limit - 1; i++)
            result[i] = new Term(poly1[i + 1].xExponent, ALPHA_VALUE_EXPONENT[ALPHA_VALUE[poly1[i + 1].alphaExponent]
                    ^ ALPHA_VALUE[poly2[i + 1].alphaExponent]]);
        if (poly1.length > poly2.length)
            for (; i < result.length; i++)
                result[i] = poly1[i + 1].clone();
        else
            for (; i < result.length; i++)
                result[i] = poly2[i + 1].clone();
        return result;
    }

    public Term clone() {
        return new Term(xExponent, alphaExponent);
    }

    public Term add(Term term) {
        int alpha1 = ALPHA_VALUE[this.alphaExponent];
        int alpha2 = ALPHA_VALUE[term.alphaExponent];
        return new Term(xExponent, ALPHA_VALUE_EXPONENT[alpha1 ^ alpha2]);
    }

    public static Term[] simplify(Term[] polynomial, int difference) {
        Term[] list = new Term[polynomial.length - difference];
        int xExponent = polynomial[0].xExponent;
        int index = 0;
        Term tally = polynomial[0];
        for (int i = 1; i < polynomial.length; i++)
            if (polynomial[i].xExponent != xExponent) {
                list[index++] = tally;
                tally = polynomial[i];
                xExponent = polynomial[i].xExponent;
            } else
                tally = tally.add(polynomial[i]);
        list[index] = tally;
        return list;
    }

    public static int getAlpha(int exponent) {
        int result = 1;
        for (int i = 0; i < exponent; i++) {
            result *= 2;
            if (result > 255)
                result ^= 285;
        }
        return result;
    }

    public static String prettyPrint(Term... polynomial) {
        String result = "";
        for (int i = 0; i < polynomial.length; i++)
            result += "α" + toSuperscript(polynomial[i].alphaExponent) + "x" + toSuperscript(polynomial[i].xExponent)
                    + " + ";
        return result.substring(0, result.length() - 3);
    }

    public static String condensedPrettyPrint(Term... polynomial) {
        String result = "";
        for (int i = 0; i < polynomial.length; i++) {
            String alpha = polynomial[i].alphaExponent == 0 ? "" : "α" + toSuperscript(polynomial[i].alphaExponent);
            String x = polynomial[i].xExponent == 0 ? "" : "x" + toSuperscript(polynomial[i].xExponent);
            result += alpha + x + " + ";
        }
        return result.substring(0, result.length() - 3);
    }

    public static String NumericPrettyPrint(Term... polynomial) {
        String result = "";
        for (int i = 0; i < polynomial.length; i++) {
            String alpha = polynomial[i].alphaExponent == 0 ? "" : "" + ALPHA_VALUE[polynomial[i].alphaExponent];
            String x = polynomial[i].xExponent == 0 ? "" : "x" + toSuperscript(polynomial[i].xExponent);
            result += alpha + x + " + ";
        }
        return result.substring(0, result.length() - 3);
    }

    public static Term[] toPolynomial(int[] message) {
        Term[] polynomial = new Term[message.length / 8];
        for (int i = 0; i < message.length; i += 8) {
            int codeword = 0;
            for (int j = 0; j < 8; j++) {
                int bit = message[i + j] % 2;
                codeword |= (bit << j);
            }
            polynomial[polynomial.length - i / 8 - 1] = new Term(i / 8, ALPHA_VALUE_EXPONENT[codeword]);
        }
        return polynomial;
    }

    public static Term[] toPolynomialTemp(int[] message) {
        Term[] polynomial = new Term[message.length];
        for (int i = 0; i < polynomial.length; i++)
            polynomial[i] = new Term(polynomial.length - i - 1, ALPHA_VALUE_EXPONENT[message[i]]);
        return polynomial;
    }

    private static String toSuperscript(int number) {
        String result = "";
        int total = number;
        int m = 10;
        do {
            result = SUPERSCRIPT[total % m] + result;
            total /= 10;
        } while (total != 0);
        return result;
    }

    public int getAlphaExponent() {
        return alphaExponent;
    }

    public int getXExponent() {
        return xExponent;
    }

    public static int[] toErrorCodes(Term[] polynomial) {
        int[] message = new int[polynomial.length * 8];
        for (int i = 0; i < polynomial.length; i++) {
            int codeword = ALPHA_VALUE[polynomial[i].alphaExponent];
            for (int j = 7; j > -1; j--) {
                message[message.length - 1 - i * 8 - j] = 2 + (codeword % 2);
                codeword >>>= 1;
            }
        }
        return message;
    }

    public static int[] toArray(Term[] polynomial) {
        int[] result = new int[polynomial.length];
        for (int i = 0; i < polynomial.length; i++)
            result[i] = ALPHA_VALUE[polynomial[i].alphaExponent];
        return result;
    }

    public static Term[] genPolynomial(int errorCodeCount) {
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
}