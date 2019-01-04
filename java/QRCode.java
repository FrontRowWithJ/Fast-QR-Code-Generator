package java;
import java.util.Arrays;

public class QRCode implements QRConstants {
    private final int QRVersion;
    private final int QRWidth;
    private final int ECL;
    // number of format and verion information modules
    private int[] formatData = new int[15];
    private int[] versionData = new int[18];
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
    static {
        Arrays.fill(REMAINDER_BITS, 1, 6, 7);
        Arrays.fill(REMAINDER_BITS, 13, 34, 3);
        Arrays.fill(REMAINDER_BITS, 20, 27, 4);
    }

    // I might scrap alphanumeric mode and byteMode
    public QRCode(int ECL, String message, boolean setToDark) {
        QRVersion = getQRVersion(message, ECL);
        this.ECL = ECL;
        int[] messageBitStream = byteMode(message);
        int[] messageCodewords = coalesceBitsToBytes(messageBitStream);
        QRWidth = 17 + QRVersion * 4;
        QRData = new int[QRWidth][QRWidth];
        for (int[] row : QRData)
            Arrays.fill(row, -1);
        addSeparators();
        addPositionDetectors();
        addTimingPatterns();
        QRData[4 * QRVersion + 9][8] = TRUE_READ_ONLY;
        addFormatData(QRData);
        if (QRVersion >= 7) {
            genVersionData();
            addVersionData();
        }
        addAlignmentPatterns();
        reverse(messageCodewords);
        int[] finalMessage = genFinalMessage(messageCodewords);
        finalMessage = toBitArray(finalMessage);
        finalMessage = addRemainingBits(finalMessage);
        addCodeWords(finalMessage);
        addMask();
        addWhiteBorder();
        if (setToDark)
            setToDark();
    }

    private void genFormatData(int ECL, int mpr, int[] formatData) {
        int result = new int[] { 1, 0, 3, 2 }[ECL];
        result <<= 3;
        result |= mpr;
        int data = result;
        if (result == 0)
            return;
        result <<= FORMAT_OFFSET;
        do {
            int gx = FORMAT_GX << (highestOneBit(result) - FORMAT_OFFSET);
            result ^= gx;
        } while (highestOneBit(result) >= FORMAT_OFFSET);
        result |= (data << (FORMAT_OFFSET));
        result ^= XOR_MASK;
        for (int i = 0; i < formatData.length; i++)
            formatData[i] = (result >> i) & 1;
    }

    private void addFormatData(int[][] QRData) {
        int fBit = 0;
        for (; fBit <= 5; fBit++)
            QRData[fBit][8] = QRData[8][QRWidth - fBit - 1] = formatData[fBit];
        for (; fBit <= 7; fBit++)
            QRData[fBit + 1][8] = QRData[8][QRWidth - fBit - 1] = formatData[fBit];
        QRData[fBit][7] = QRData[4 * QRVersion + fBit + 2][8] = formatData[fBit++];
        for (; fBit <= 14; fBit++)
            QRData[8][14 - fBit] = QRData[4 * QRVersion + fBit + 2][8] = formatData[fBit];
    }

    private void genVersionData() {
        int result = QRVersion << 12;
        do {
            int pos = highestOneBit(result);
            int gx = VERSION_GX << (pos - 12);
            result ^= gx;
        } while (highestOneBit(result) >= 12);
        result |= QRVersion << 12;
        for (int i = 0; i < 18; i++)
            versionData[i] = (result >>> i) & 1;
    }

    private int highestOneBit(int number) {
        return (int) (Math.log(Integer.highestOneBit(number)) / NATURAL_LOG_2);
    }

    private void addVersionData() {
        for (int i = 0; i < versionData.length; i++)
            QRData[i / 3][(QRWidth - 11) + i % 3] = QRData[i % 3 + QRWidth - 11][i / 3] = versionData[i];
    }

    private void addPositionDetectors() {
        copyToQRCode(0, 0, POSITION_DETECTOR);
        copyToQRCode(0, QRData[0].length - PD_WIDTH, POSITION_DETECTOR);
        copyToQRCode(QRData.length - PD_WIDTH, 0, POSITION_DETECTOR);
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

    private void copyToQRCode(int offsetI, int offsetJ, int[][] copyFrom) {
        for (int i = 0; i < copyFrom.length; i++)
            System.arraycopy(copyFrom[i], 0, QRData[offsetI + i], offsetJ, copyFrom[0].length);
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
                copyToQRCode(patternCoords[i] - 2, patternCoords[j] - 2, ALIGNMENT_PATTERN);
            }
        }
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

    public void export(String directory, int moduleSize, String fileName) {
        QRImage.genQRImage(QRData, moduleSize, directory, fileName);
    }

    private void addCodeWords(int[] finalMessage) {
        int index = finalMessage.length - 1;
        int j = QRWidth - 1;
        for (int k = 0; k < QRWidth / 2; k++) {
            int i = k % 2 == 0 ? QRWidth - 1 : 0;
            int inc = k % 2 == 0 ? -1 : 1;
            for (; k % 2 == 0 ? i > -1 : i < QRWidth; i += inc) {
                if (i == TIMING_PATTERN)
                    continue;
                if (QRData[i][j] != TRUE_READ_ONLY && QRData[i][j] != FALSE_READ_ONLY)
                    QRData[i][j] = finalMessage[index--];
                if (QRData[i][j - 1] != TRUE_READ_ONLY && QRData[i][j - 1] != FALSE_READ_ONLY)
                    QRData[i][j - 1] = finalMessage[index--];
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
            throw new IllegalArgumentException("The message array must be divisible by eight (" + message.length + ")");
        int[] byteArray = new int[message.length / 8];
        for (int i = 0; i < message.length; i += 8) {
            int codeword = 0;
            for (int j = 0; j < 8; j++)
                codeword |= (message[i + j] % 2) << j;
            byteArray[i / 8] = codeword;
        }
        return byteArray;
    }

    public int getQRVersion(String message, int ECL) {
        int messageLength = -1;
        if (message.length() == 0)
            return 1;
        messageLength = message.length() < 256 ? 8 : message.length() < 1024 ? 10 : message.length() < 4096 ? 12 : -1;
        if (messageLength == -1)
            throw new IllegalArgumentException(String
                    .format("The string is too long (%d). The maximum length is 4095 characters", message.length()));
        messageLength += 4 + 8 * message.length();
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
            message += TERMINATOR;
        message += new String(new char[(8 - message.length() % 8) % 8]).replace('\0', '0');
        int l = (maxBitLength - message.length()) / 8;
        for (int i = 0; i < l; i++)
            message += i % 2 == 0 ? TWO_THREE_SIX : SEVENTEEN;
        return message;
    }

    private int[] getBlockStructure() {
        // array Structure: result[0] = numOfDataCodewords per block, result[1] =
        // numOfErrorCodewords per block, result[2] = number of blocks
        int numOfCodeWords = numOfCodeWords(QRVersion);
        int[] result = new int[numOfCodeWords % BLOCK_COUNT[ECL][QRVersion - 1] == 0 ? 3 : 6];
        result[0] = (numOfCodeWords - ERROR_CODES[ECL][QRVersion - 1]) / BLOCK_COUNT[ECL][QRVersion - 1];
        result[1] = numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1] - result[0];
        // - (QRVersion < 4 ? P_VALUE[QRVersion - 1][ECL] : 0);
        int n = numOfCodeWords - (numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1]) * BLOCK_COUNT[ECL][QRVersion - 1];
        result[2] = BLOCK_COUNT[ECL][QRVersion - 1] - n;
        if (result.length == 6) {
            result[3] = result[0] + 1;
            result[4] = result[1];
            result[5] = n;
        }
        return result;
    }

    // generate final qr message
    private int[] genFinalMessage(int[] message) {
        int position = 0;
        int[] blockStructure = getBlockStructure();
        int[][] messageBlock = new int[BLOCK_COUNT[ECL][QRVersion - 1]][];
        for (int i = 0; i < blockStructure[2]; i++) {
            messageBlock[i] = new int[blockStructure[0]];
            System.arraycopy(message, position, messageBlock[i], 0, messageBlock[i].length);
            position += blockStructure[0];
        }
        if (blockStructure.length == 6)
            for (int i = blockStructure[2]; i < blockStructure[2] + blockStructure[5]; i++) {
                messageBlock[i] = new int[blockStructure[3]];
                System.arraycopy(message, position, messageBlock[i], 0, messageBlock[i].length);
                position += blockStructure[3];
            }
        int[] weavedMessage = new int[message.length];
        if (blockStructure.length == 6) {
            for (int i = 0; i < message.length - blockStructure[5]; i++)
                weavedMessage[i] = messageBlock[i % messageBlock.length][i / messageBlock.length];
            for (int i = 0; i < blockStructure[5]; i++)
                weavedMessage[weavedMessage.length
                        - (blockStructure[5] - i)] = messageBlock[blockStructure[2] + i][blockStructure[3] - 1];
        } else
            for (int i = 0; i < weavedMessage.length; i++)
                weavedMessage[i] = messageBlock[i % messageBlock.length][i / messageBlock.length];
        int[] weavedErrorCodewords = genErrorBlock(messageBlock, blockStructure[1]);
        int[] finalMessage = new int[weavedMessage.length + weavedErrorCodewords.length];
        System.arraycopy(weavedMessage, 0, finalMessage, 0, weavedMessage.length);
        System.arraycopy(weavedErrorCodewords, 0, finalMessage, weavedMessage.length, weavedErrorCodewords.length);
        return finalMessage;
    }

    private int[] genErrorBlock(int[][] messageBlock, int numOfErrorCodewords) {
        int[][] errorBlock = new int[messageBlock.length][];
        for (int i = 0; i < errorBlock.length; i++)
            errorBlock[i] = Term.toArray(genErrorPolyNomial(messageBlock[i], numOfErrorCodewords));
        int[] weavedCodewords = new int[numOfErrorCodewords * messageBlock.length];
        for (int i = 0; i < weavedCodewords.length; i++)
            weavedCodewords[i] = errorBlock[i % errorBlock.length][i / errorBlock.length];
        return weavedCodewords;
    }

    public Term[] genErrorPolyNomial(int[] message, int numOfErrorCodewords) {
        Term[] messagePolynomial = Term.toPolynomialTemp(message);
        Term[] generatorPolynomial = Term.genPolynomial(numOfErrorCodewords);
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

    public static int[] toBitArray(int[] message) {
        // LSB is stored at the lowest index
        reverse(message);
        int[] bitArray = new int[message.length * 8];
        for (int i = 0; i < message.length; i++) {
            int codeword = message[i];
            for (int j = 0; j < 8; j++) {
                bitArray[i * 8 + j] = (codeword & 1) + 2;
                codeword >>>= 1;
            }
        }
        return bitArray;
    }

    private int[] addRemainingBits(int[] finalMessage) {
        if (REMAINDER_BITS[QRVersion - 1] != 0) {
            int[] result = new int[finalMessage.length + REMAINDER_BITS[QRVersion - 1]];
            System.arraycopy(finalMessage, 0, result, REMAINDER_BITS[QRVersion - 1], finalMessage.length);
            Arrays.fill(result, 0, REMAINDER_BITS[QRVersion - 1], TRUE);
            return result;
        }
        return finalMessage;
    }

    private void addSeparators() {
        for (int i = 0; i < 8; i++) {
            QRData[i][7] = QRData[7][i] = FALSE_READ_ONLY;
            QRData[i][QRWidth - 8] = QRData[QRWidth - 8][i] = FALSE_READ_ONLY;
            QRData[QRWidth - i - 1][7] = QRData[7][QRWidth - i - 1] = FALSE_READ_ONLY;
        }
    }

    private void addMask() {
        int correct_mpr = 0;
        int mpr = 0;
        int[][] QRDataOriginal = copyOf(QRData);
        int[][] QRDataCopy = copyOf(QRData);
        int matrixPenalty = Integer.MAX_VALUE;
        for (int i = 0; i < 8; i++) {
            switch (i) {
            case I_PLUS_J_MOD_2:
                mpr = I_PLUS_J_MOD_2;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = row % 2; column < QRDataCopy.length; column += 2)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= 1;
                break;
            case I_MOD_2:
                mpr = I_MOD_2;
                for (int row = 0; row < QRDataCopy.length; row += 2)
                    for (int column = 0; column < QRDataCopy.length; column++)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= 1;
                break;
            case J_MOD_3:
                mpr = J_MOD_3;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = 0; column < QRDataCopy.length; column += 3)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= 1;
                break;
            case I_PLUS_J_MOD_3:
                mpr = I_PLUS_J_MOD_3;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = new int[] { 0, 2, 1 }[row % 3]; column < QRDataCopy.length; column += 3)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= 1;
                break;
            case I_DIV_2_PLUS_J_DIV_3_MOD_2:
                mpr = I_DIV_2_PLUS_J_DIV_3_MOD_2;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = 0; column < QRDataCopy.length; column++)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= (row / 2 + column / 3) % 2 ^ 1;
                break;
            case I_J_MOD_2_PLUS_I_J_MOD_3:
                mpr = I_J_MOD_2_PLUS_I_J_MOD_3;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = 0; column < QRDataCopy.length; column++)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= row * column % 2 + row * column % 3 == 0 ? 1 : 0;
                break;
            case I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2:
                mpr = I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = 0; column < QRDataCopy.length; column++)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= (row * column % 2 + row * column % 3) % 2 ^ 1;
                break;
            case I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2:
                mpr = I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2;
                for (int row = 0; row < QRDataCopy.length; row++)
                    for (int column = 0; column < QRDataCopy.length; column++)
                        if (QRDataCopy[row][column] > 1)
                            QRDataCopy[row][column] ^= ((row + column % 2) + row * column % 3) % 2 ^ 1;
            }
            genFormatData(ECL, mpr, formatData);
            addFormatData(QRDataCopy);
            int penalty = getMatrixPenalty(QRDataCopy);
            if (penalty < matrixPenalty) {
                matrixPenalty = penalty;
                QRData = copyOf(QRDataCopy);
                correct_mpr = mpr;
            }
            QRDataCopy = copyOf(QRDataOriginal);
        }
        genFormatData(ECL, correct_mpr, formatData);
        addFormatData(QRData);
    }

    private static int[][] copyOf(int[][] matrix) {
        int[][] copy = new int[matrix.length][matrix.length];
        for (int i = 0; i < matrix.length; i++)
            System.arraycopy(matrix[i], 0, copy[i], 0, matrix.length);
        return copy;
    }

    private static int getMatrixPenalty(int[][] QRData) {
        int penalty = 0;
        penalty += evaluateConsecutiveModulesPenalty(QRData);
        penalty += evaluate2by2ModulePenalty(QRData);
        penalty += evaluatePatternPenalty(QRData);
        penalty += evaluateRatioPenalty(QRData);
        return penalty;
    }

    private static int evaluateConsecutiveModulesPenalty(int[][] QRData) {
        int penalty = 0;
        for (int i = 0; i < QRData.length; i++) {
            int numOfSimilarModulesVertical = 1;
            int numOfSimilarModulesHorizontal = 1;
            for (int j = 1; j < QRData.length; j++) {
                if (QRData[j - 1][i] % 2 == QRData[j][i] % 2)
                    numOfSimilarModulesVertical++;
                else
                    numOfSimilarModulesVertical = 1;
                if (numOfSimilarModulesVertical == 5)
                    penalty += 3;
                else if (numOfSimilarModulesVertical > 5)
                    penalty += 1;
                if (QRData[i][j] % 2 == QRData[i][j - 1] % 2)
                    numOfSimilarModulesHorizontal++;
                else
                    numOfSimilarModulesHorizontal = 1;
                if (numOfSimilarModulesHorizontal == 5)
                    penalty += 3;
                if (numOfSimilarModulesHorizontal > 5)
                    penalty += 1;
            }
        }
        return penalty;
    }

    private static int evaluate2by2ModulePenalty(int[][] QRData) {
        int penalty = 0;
        for (int i = 0; i < QRData.length - 1; i++)
            for (int j = 0; j < QRData.length - 1; j++)
                if (QRData[i][j] % 2 == QRData[i][j + 1] % 2 && QRData[i][j + 1] % 2 == QRData[i + 1][j] % 2
                        && QRData[i + 1][j] % 2 == QRData[i + 1][j + 1] % 2)
                    penalty += 3;
        return penalty;
    }

    private static int evaluatePatternPenalty(int[][] QRData) {
        int penalty = 0;
        for (int i = 0; i < QRData.length - MODULE_PATTERN_1.length; i++)
            for (int j = 0; j < QRData.length - MODULE_PATTERN_1.length; j++) {
                int tally = 0;
                while (tally < MODULE_PATTERN_1.length) {
                    if (QRData[i][j + tally] % 2 == MODULE_PATTERN_1[tally] % 2)
                        tally++;
                    else
                        break;
                }
                if (tally == MODULE_PATTERN_1.length)
                    penalty += 40;
                tally = 0;
                while (tally < MODULE_PATTERN_2.length) {
                    if (QRData[i][j + tally] % 2 == MODULE_PATTERN_2[tally] % 2)
                        tally++;
                    else
                        break;
                }
                if (tally == MODULE_PATTERN_2.length)
                    penalty += 40;
                tally = 0;
                while (tally < MODULE_PATTERN_1.length) {
                    if (QRData[j + tally][i] % 2 == MODULE_PATTERN_1[tally] % 2)
                        tally++;
                    else
                        break;
                }
                if (tally == MODULE_PATTERN_1.length)
                    penalty += 40;
                tally = 0;
                while (tally < MODULE_PATTERN_2.length) {
                    if (QRData[j + tally][i] % 2 == MODULE_PATTERN_2[tally] % 2)
                        tally++;
                    else
                        break;
                }
                if (tally == MODULE_PATTERN_2.length)
                    penalty += 40;
            }
        return penalty;
    }

    private static int evaluateRatioPenalty(int[][] QRData) {
        int darkModules = 0;
        for (int i = 0; i < QRData.length; i++)
            for (int j = 0; j < QRData.length; j++)
                darkModules += QRData[i][j] % 2;
        int ratio = (int) Math.round((double) darkModules / (QRData.length * QRData.length) * 100);
        int previousMultiple = ratio - ratio % 5;
        int nextMultiple = previousMultiple + 5;
        return Math.min(Math.abs(previousMultiple - 50) / 5, Math.abs(nextMultiple - 50) / 5) * 10;
    }

    private void addWhiteBorder() {
        int[][] finalQRData = new int[QRWidth + 8][QRWidth + 8];
        for (int i = 0; i < QRWidth; i++)
            System.arraycopy(QRData[i], 0, finalQRData[i + 4], 4, QRWidth);
        QRData = finalQRData;
    }

    private void setToDark() {
        for (int i = 0; i < QRData.length; i++)
            for (int j = 0; j < QRData.length; j++)
                QRData[i][j] ^= 1;
    }

    public static void reverse(int[] array) {
        for (int i = 0; i < array.length / 2; i++) {
            int tmp = array[i];
            array[i] = array[array.length - 1 - i];
            array[array.length - 1 - i] = tmp;
        }
    }
}