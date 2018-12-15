import java.util.Arrays;

import jdk.nashorn.internal.runtime.ScriptingFunctions;

public class QRCode implements QRConstants {
    private final int QRVersion;
    private final int QRWidth;
    private final String mode;
    private final int ECL;
    // number of format and verion information modules
    private int[] formatData = new int[15];
    private int[] versionData;
    private int[][] QRData;
    static {
        long a = 0b1111111100000110111011011101101110110000011111111L;
        for (int i = 0; i < PD_WIDTH; i++)
            for (int j = 0; j < PD_WIDTH; j++) {
                POSITION_DETECTOR[i][j] = (int) (a & 0x1);
                a >>>= 1;
            }
    }

    static {
        long a = 0b1111110001101011000111111;
        for (int i = 0; i < ALIGNMENT_PATTERN.length; i++)
            for (int j = 0; j < ALIGNMENT_PATTERN.length; j++) {
                ALIGNMENT_PATTERN[i][j] = (int) (a & 0x1);
                a >>>= 1;
            }
    }

    public QRCode(int ECL, int mpr, String message) {
        mode = getMode(message);
        QRVersion = getQRVersion(message, ECL, mode);
        this.ECL = ECL;
        int[] messageBitStream = null;
        switch (mode) {
        case ALPHANUMERIC_MODE:
            messageBitStream = alphanumericMode(message);
            break;
        case NUMERIC_MODE:
            messageBitStream = numericMode(message);
            break;
        case BYTE_MODE:
            messageBitStream = byteMode(message);
        }
        int[] messageCodewords = coalesceBitsToBytes(messageBitStream);
        QRWidth = 17 + QRVersion * 4;
        QRData = new int[QRWidth][QRWidth];
        for (int[] row : QRData)
            for (int i = 0; i < row.length; i++)
                row[i] = -1;
        addPositionDetectors();
        addTimingPatterns();
        int QRDarkModuleI = 4 * QRVersion + 9;
        int QRDarkModuleJ = 8;
        QRData[QRDarkModuleI][QRDarkModuleJ] = TRUE_READ_ONLY;
        genFormatData(ECL, mpr);
        addFormatData();
        genVersionData();
        addVersionData();
        addAlignmentPatterns();
        genMessageBlock(messageCodewords);
    }

    private void genFormatData(int ECL, int mpr) {
        int test = mpr & 0xFF;
        if (test > 7)
            throw new IllegalArgumentException("mpr must not be greater than 7 (111)");
        int result = ECL;
        result <<= 3;
        result |= mpr;

        int data = result;
        if (result == 0) {
            formatData = new int[15];
            return;
        }
        result <<= FORMAT_OFFSET;
        do {
            int pos = highestOneBit(result);
            int gx = FORMAT_GX << (pos - FORMAT_OFFSET);
            result ^= gx;
        } while (highestOneBit(result) >= FORMAT_OFFSET);
        result |= (data << 10);
        result ^= XOR_MASK;
        for (int i = 0; i < 15; i++) {
            formatData[i] = (result & 1);
            result >>= 1;
        }
    }

    private void addFormatData() {
        // 0 - 5: i1 = fBit & j1 = 8
        // i2 = 8 & j2 = QRWidth - fBit - 1
        // 6 - 8: i1 = fBit + 1 & j1 = 8(for 6 and 7) then i1 = fBit & j1 = 7
        // i2 = 8 & j2 = QRWirdth - fBit - 1(for 6 and 7) then i1 = (4V + fBit + 2) & j2
        // = 8
        // 9 - 14: i1 = 8 & j2 = 14 - fBit
        // i2 = (4V + fBit + 2) & j2 = 8
        int fBit = 0;
        for (; fBit <= 5; fBit++) {
            QRData[fBit][8] = formatData[fBit];
            QRData[8][QRWidth - fBit - 1] = formatData[fBit];
        }
        for (; fBit <= 7; fBit++) {
            QRData[fBit + 1][8] = formatData[fBit];
            QRData[8][QRWidth - fBit - 1] = formatData[fBit];
        }
        QRData[fBit][7] = formatData[fBit];
        QRData[4 * QRVersion + fBit + 2][8] = formatData[fBit++];
        for (; fBit <= 14; fBit++) {
            QRData[8][14 - fBit] = formatData[fBit];
            QRData[4 * QRVersion + fBit + 2][8] = formatData[fBit];
        }
    }

    private void genVersionData() {
        int result = QRVersion << 12;
        do {
            int pos = highestOneBit(result);
            int gx = VERSION_GX << (pos - 12);
            result ^= gx;
        } while (highestOneBit(result) >= 12);
        result |= QRVersion << 12;
        versionData = new int[18];
        for (int i = 0; i < 18; i++) {
            versionData[i] = (result & 1);
            result >>= 1;
        }
    }

    private int highestOneBit(int number) {
        return (int) (Math.log(Integer.highestOneBit(number)) / NATURAL_LOG_2);
    }

    private void addVersionData() {
        if (QRVersion < 7)
            return;
        int[][] vrData = new int[6][3];
        for (int i = 0; i < versionData.length; i++)
            vrData[i / vrData[0].length][i % vrData[0].length] = versionData[i];
        paste(0, QRWidth - 11, vrData, QRData);
        vrData = new int[3][6];
        for (int i = 0; i < versionData.length; i++)
            vrData[i % vrData.length][i / vrData.length] = versionData[i];
        paste(QRWidth - 11, 0, vrData, QRData);
    }

    public static void main(String[] args) {
        QRCode qr = new QRCode(1, 0b101, "A");
        qr.export("./", 4);
    }

    private void addPositionDetectors() {
        paste(0, 0, POSITION_DETECTOR, QRData);
        paste(0, QRData[0].length - PD_WIDTH, POSITION_DETECTOR, QRData);
        paste(QRData.length - PD_WIDTH, 0, POSITION_DETECTOR, QRData);
    }

    private void addTimingPatterns() {
        int iH = PD_WIDTH - 1;
        int jH = PD_WIDTH + 1;
        int iV = PD_WIDTH + 1;
        int jV = PD_WIDTH - 1;
        int len = QRData.length - (PD_WIDTH + 1) * 2;
        int val = TRUE_READ_ONLY;
        for (int i = 0; i < len; i++) {
            QRData[iH][jH++] = val;
            QRData[iV++][jV] = val;
            val = val == TRUE_READ_ONLY ? FALSE_READ_ONLY : TRUE_READ_ONLY;
        }
    }

    private static void paste(int offsetI, int offsetJ, int[][] copyFrom, int[][] pasteTo) {
        for (int i = 0; i < copyFrom.length; i++)
            for (int j = 0; j < copyFrom[0].length; j++)
                pasteTo[offsetI + i][offsetJ + j] = copyFrom[i][j];
    }

    private int[] genAlignmentCoords(int version) {
        if (version == 1)
            return new int[0];
        if (version == 32)
            return new int[] { 6, 34, 60, 86, 112, 138 };
        int numOfCoords = 2 + version / 7;
        int[] result = new int[numOfCoords];
        result[0] = 6;
        result[result.length - 1] = 4 * version + 10;
        int diff = 4 * (version + 1);
        double gap = (double) diff / (numOfCoords - 1);
        gap = ((int) gap / 2 * 2) + (gap % 2.0 == 0.0 ? 0.0 : 2.0);
        result[1] = 6 + (diff - (int) gap * (numOfCoords - 2));
        for (int i = 2; i < result.length; i++)
            result[i] = result[i - 1] + (int) gap;
        return result;
    }

    private void addAlignmentPatterns() {
        if (QRVersion == 1)
            return;
        int[] patternCoords = genAlignmentCoords(QRVersion);
        int numOfCoords = 2 + QRVersion / 7;
        for (int i = 0; i < numOfCoords; i++) {
            for (int j = 0; j < numOfCoords; j++) {
                if (i == 0 && j == 0)
                    continue;
                if (i == 0 && j == numOfCoords - 1)
                    continue;
                if (i == numOfCoords - 1 && j == 0)
                    continue;
                paste(patternCoords[i] - 2, patternCoords[j] - 2, ALIGNMENT_PATTERN, QRData);
            }
        }
    }

    private int[] numericMode(String inputData) {
        String result = "";
        int quotient = inputData.length() / 3;
        int remainder = inputData.length() % 3;
        for (int i = 0; i < quotient * 3; i += 3)
            result += String.format("%1$10s", Integer.toBinaryString(Integer.parseInt(inputData.substring(i, i + 3))))
                    .replace(" ", "0");
        if (remainder > 0)
            result += Integer.toBinaryString(Integer.parseInt(inputData.substring(quotient * 3, inputData.length())))
                    .replace(" ", "0");
        int ccBitCount = QRVersion < 10 ? 10 : QRVersion < 27 ? 12 : 14;
        String characterCount = String.format("%1$" + ccBitCount + "s", Integer.toBinaryString(inputData.length()))
                .replace(" ", "0");
        result = NUMERIC_MODE + characterCount + result;
        result = addPadBytes(result);
        int[] bitStream = new int[result.length()];
        for (int i = 0; i < bitStream.length; i++)
            bitStream[i] = result.charAt(result.length() - i - 1) == '1' ? TRUE : FALSE;
        return bitStream;
    }

    private int[] alphanumericMode(String inputData) {
        String result = "";
        int quotient = inputData.length() / 2;
        int remainder = inputData.length() % 2;
        for (int i = 0; i < quotient * 2; i += 2) {
            String group = inputData.substring(i, i + 2);
            int decimalValue = ALPHANUMERIC_TABLE.indexOf(group.charAt(0)) * 45
                    + ALPHANUMERIC_TABLE.indexOf(group.charAt(1));
            result += String.format("%1$11s", Integer.toBinaryString(decimalValue)).replace(" ", "0");
        }

        if (remainder > 0) {
            int decimalValue = ALPHANUMERIC_TABLE.indexOf(inputData.charAt(inputData.length() - 1));
            result += String.format("%1$6s", Integer.toBinaryString(decimalValue)).replace(" ", "0");
        }
        int ccBitCount = QRVersion < 10 ? 9 : QRVersion < 27 ? 11 : 13;
        String characterCount = String.format("%1$" + ccBitCount + "s", Integer.toBinaryString(inputData.length()))
                .replace(" ", "0");
        result = ALPHANUMERIC_MODE + characterCount + result;
        result = addPadBytes(result);
        int[] bitStream = new int[result.length()];
        for (int i = 0; i < bitStream.length; i++)
            bitStream[i] = result.charAt(result.length() - i - 1) == '1' ? TRUE : FALSE;
        return bitStream;
    }

    private int[] byteMode(String inputData) {
        String result = "";
        for (int i = 0; i < inputData.length(); i++)
            result += String.format("%1$8s", Integer.toBinaryString(inputData.charAt(i))).replace(" ", "0");
        int ccBitCount = QRVersion < 10 ? 8 : 16;
        String characterCount = String.format("%1$" + ccBitCount + "s", Integer.toBinaryString(inputData.length()))
                .replace(" ", "0");
        result = BYTE_MODE + characterCount + result;
        result = addPadBytes(result);
        int[] bitStream = new int[result.length()];
        for (int i = 0; i < bitStream.length; i++)
            bitStream[i] = result.charAt(result.length() - i - 1) == '1' ? TRUE : FALSE;
        return bitStream;
    }

    private int[] kanjiMode(int[] inputData) {
        String bitStream = "";
        for (int i = 0; i < inputData.length; i++) {
            int data = 0;
            if (inputData[i] >= 0x8140 && inputData[i] <= 0x9FFC)
                data = inputData[i] - 0x8140;
            else if (inputData[i] >= 0xE040 && inputData[i] <= 0xEBBF)
                data = inputData[i] - 0xC140;
            else
                throw new IllegalArgumentException(String
                        .format("This character is not recognised by Shift JIS: inputData[%d], %#x ", i, inputData[i]));
            int msB = (data & 0xFF00) >>> 8;
            msB *= 0xC0;
            msB += data & 0xFF;
            bitStream += String.format("%1$13s", Integer.toBinaryString(msB)).replace(" ", "0");
        }
        int ccCount = QRVersion < 10 ? 8 : QRVersion < 27 ? 10 : 12;
        bitStream = KANJI_MODE + String.format("%1$" + ccCount + "s", inputData.length).replace(" ", "0") + bitStream;
        bitStream = addPadBytes(bitStream);
        int[] result = new int[bitStream.length()];
        for (int i = 0; i < result.length; i++)
            result[i] = bitStream.charAt(bitStream.length() - i - 1) == '1' ? TRUE : FALSE;
        return result;
    }

    public void export(String directory, int moduleSize) {
        QRImage.genQRImage(QRData, moduleSize, directory);
    }

    private void addCodeWords() {
        // int[] val = genVal();
        // int index = val.length - 1
        int j = QRWidth - 1;
        for (int k = 0; k < QRWidth / 2; k++) {
            int i = k % 2 == 0 ? QRWidth - 1 : 0;
            int inc = k % 2 == 0 ? -1 : 1;
            for (; k % 2 == 0 ? i > -1 : i < QRWidth; k += inc) {
                if (i == TIMING_PATTERN)
                    continue;
                if (QRData[i][j] > 1)
                    // QRData[i][j] = val[index--];
                    System.out.print("");
                if (QRData[i][j - 1] > 1)
                    // QRData[i][j - 1] = val[index--];
                    System.out.print("");
            }
            j -= 2;

            if (j == TIMING_PATTERN)
                j -= 1;
        }
    }

    private int numOfCodeWords(int QRVersion) {
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

    public int[] coalesceBitsToBytes(int[] message) {
        if (message.length % 8 != 0)
            throw new IllegalArgumentException(
                    "The meessage array must be divisible by eight (" + message.length + ")");
        int[] byteArray = new int[message.length / 8];
        for (int i = 0; i < message.length; i += 8) {
            int codeword = 0;
            for (int j = 0; j < 8; j++) {
                codeword |= (message[i + j] % 2) << j;
            }
            byteArray[i / 8] = codeword;
        }
        return byteArray;
    }

    private String getMode(String message) {
        String mode = "";
        boolean isByteMode, isAlphaNumericMode;
        isByteMode = isAlphaNumericMode = false;
        for (int i = 0; i < message.length(); i++) {
            char character = message.charAt(i);
            if (!isAlphaNumericMode && character >= 48 && character <= 57) {
                mode = NUMERIC_MODE;
            } else if (!isByteMode && ALPHANUMERIC_TABLE.contains("" + character)) {
                isAlphaNumericMode = true;
                mode = ALPHANUMERIC_MODE;
            } else {
                mode = BYTE_MODE;
                break;
            }
        }
        return mode;
    }

    public int getQRVersion(String message, int ECL, String mode) {
        // only numbers = numeric mode
        // numbers and characters = alphanumeric mode
        // more than that = byte mode
        int messageLength = -1;
        if (ECL < 0 || ECL > 3)
            throw new IllegalArgumentException("The ECL version must be a value between 0 and 3 inclusive");
        if (message.length() == 0)
            return 1;
        switch (mode) {
        case NUMERIC_MODE:
            messageLength = 4 + (message.length() < 1024 ? 10
                    : message.length() < 4096 ? 12 : message.length() < 16384 ? 14 : -1);
            if (messageLength == 3)
                throw new IllegalArgumentException(String.format(
                        "The string is too long (%d). The maximum length is 16383 characters", message.length()));
            messageLength += 10 * (message.length() % 3);
            int[] yeet = new int[] { 0, 4, 7 };
            messageLength += yeet[(message.length() % 3)];
            messageLength = messageLength % 8 == 0 ? messageLength / 8 : messageLength / 8 + 1;
            break;
        case ALPHANUMERIC_MODE:
            messageLength = message.length() < 512 ? 9
                    : message.length() < 2048 ? 11 : message.length() < 8192 ? 13 : -1;
            if (messageLength == -1)
                throw new IllegalArgumentException(String.format(
                        "The string is too long (%d). The maximum length is 8191 characters", message.length()));
            messageLength += 4 + 11 * (message.length() / 2) + 6 * (message.length() % 2);
            break;
        case BYTE_MODE:
            messageLength = message.length() < 256 ? 8
                    : message.length() < 1024 ? 10 : message.length() < 4096 ? 12 : -1;
            if (messageLength == -1)
                throw new IllegalArgumentException(String.format(
                        "The string is too long (%d). The maximum length is 4095 characters", message.length()));
            messageLength += 4 + 8 * message.length();
        }
        int byteLength = messageLength % 8 == 0 ? messageLength / 8 : messageLength / 8 + 1;
        for (int i = 1; i <= 40; i++) {
            int c = 0;
            if (i < 4)
                c += P_VALUE[i - 1][ECL];
            int messageCodeWordCount = numOfCodeWords(i) - c - ERROR_CODES[ECL][i - 1];
            if (messageCodeWordCount >= byteLength)
                return i;
        }
        throw new IllegalAccessError("Oops, something went wrong");
    }

    private String addPadBytes(String message) {
        int maxBitLength = (numOfCodeWords(QRVersion) - ERROR_CODES[ECL][QRVersion - 1]) * 8;
        if (maxBitLength < message.length())
            throw new IllegalArgumentException("The qr code version is incorrect");
        if (maxBitLength - message.length() <= 4) {
            message += new String(new char[maxBitLength - message.length()]).replace('\0', '0');
            return message;
        } else
            message += "0000";
        message += new String(new char[(8 - message.length() % 8) % 8]).replace('\0', '0');
        int l = (maxBitLength - message.length()) / 8;
        for (int i = 0; i < l; i++)
            message += i % 2 == 0 ? TWO_THREE_SIX : SEVENTEEN;
        return message;
    }

    // generate final qr message
    private void genMessageBlock(int[] message) {
        int[] blockStructure = getBlockStructure();
        int[][] messageBlock = new int[BLOCK_COUNT[ECL][QRVersion - 1]][];
        for (int i = 0; i < blockStructure[2]; i++) {
            messageBlock[i] = new int[blockStructure[0]];
            System.arraycopy(message, blockStructure[0] * i, messageBlock[i], 0, messageBlock[i].length);
        }
        if (blockStructure.length == 6)
            for (int i = blockStructure[2]; i < blockStructure[4]; i++) {
                messageBlock[i] = new int[blockStructure[3]];
                System.arraycopy(message, blockStructure[3] * i, messageBlock[i], 0, messageBlock[i].length);
            }
    }

    private int[] getBlockStructure() {
        // array Structure: result[0] = numOfDataCodewords per block, result[1] =
        // numOfErrorCodewords per block, result[2] = number of blocks
        int numOfCodeWords = numOfCodeWords(QRVersion);
        int[] result = new int[numOfCodeWords % BLOCK_COUNT[ECL][QRVersion - 1] == 0 ? 3 : 6];
        result[0] = (numOfCodeWords - ERROR_CODES[ECL][QRVersion - 1]) / BLOCK_COUNT[ECL][QRVersion - 1];
        result[1] = numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1] - result[0]
                - (QRVersion < 4 ? P_VALUE[QRVersion - 1][ECL] : 0);
        int n = numOfCodeWords - (numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1]) * BLOCK_COUNT[ECL][QRVersion - 1];
        result[2] = BLOCK_COUNT[ECL][QRVersion - 1] - n;
        if (result.length == 6) {
            result[3] = result[0] + 1;
            result[4] = result[1];
            result[5] = n;
        }
        return result;
    }
}