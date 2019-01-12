#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QRCode.h"
#include <math.h>

// int QRVersion;
// size_t QRWidth;
// int ECL;
// int formatData[15];
// int versionData[18];
// int **QRData;

int main(void)
{
    gen_position_detectors();
    int formatData[15] = {0};
    gen_format_data(0, 0, formatData, sizeof(formatData) / sizeof(int));
    printf("%d\n", to_int(formatData, sizeof(formatData) / sizeof(int)));
    return 0;
}

void gen_format_data(int ECL, int mpr, int formatData[15], size_t len)
{
    int ecl_value[4] = {1, 0, 3, 2};
    int result = ecl_value[ECL];
    int fo = FORMAT_OFFSET;
    int fgx = FORMAT_GX;
    result <<= 3;
    result |= mpr;
    int data = result;
    if (result == 0)
        return;
    result <<= FORMAT_OFFSET;
    do
    {
        int gx = fgx << (highest_one_bit_position_of(result) - fo);
        result ^= gx;
    } while (highest_one_bit_position_of(result) >= fo);
    result |= (data << (fo));
    result ^= XOR_MASK;
    printf("%d\n", result);
    for (int i = 0; i < len; i++)
        formatData[i] = (result >> i) & 1;
}

int to_int(int array[], size_t len)
{
    int result = 0;
    for (int i = 0; i < len; i++)
        result |= (array[i] << i);
    return result;
}

int highest_one_bit_position_of(int n)
{
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    int r = n - (n >> 1);
    return (int)(log(r) / log(2));
}

void gen_position_detectors(void)
{
    POSITION_DETECTOR = init_matrix(PD_WIDTH, PD_WIDTH);
    long pd = 0b1111111100000110111011011101101110110000011111111L;
    for (int i = 0; i < PD_WIDTH; i++)
    {
        for (int j = 0; j < PD_WIDTH; j++)
        {
            POSITION_DETECTOR[i][j] = (int)(pd & 1);
            pd >>= 1;
        }
    }
}

void gen_alignment_pattern(void)
{
    ALIGNMENT_PATTERN = init_matrix(AP_WIDTH, AP_WIDTH);
    long a = 0b1111110001101011000111111;
    for (int i = 0; i < AP_WIDTH; i++)
        for (int j = 0; j < AP_WIDTH; j++)
        {
            ALIGNMENT_PATTERN[i][j] = (int)(a & 0x1);
            a >>= 1;
        }
}

void add_remainder_bits_aray(void)
{
    for (int i = 1; i < 6; i++)
        REMAINDER_BITS[i] = 7;
    for (int i = 13; i < 34; i++)
        REMAINDER_BITS[i] = 3;
    for (int i = 20; i < 27; i++)
        REMAINDER_BITS[i] = 4;
}

void add_format_data(int **QRData, size_t QRWidth, int QRVersion, int formatData[15])
{
    int fBit = 0;
    for (; fBit <= 5; fBit++)
        QRData[fBit][8] = QRData[8][QRWidth - fBit - 1] = formatData[fBit];
    for (; fBit <= 7; fBit++)
        QRData[fBit + 1][8] = QRData[8][QRWidth - fBit - 1] = formatData[fBit];
    QRData[fBit][7] = QRData[4 * QRVersion + fBit + 2][8] = formatData[fBit++];
    for (; fBit <= 14; fBit++)
        QRData[8][14 - fBit] = QRData[4 * QRVersion + fBit + 2][8] = formatData[fBit];
}

void gen_version_data(int QRVersion, int versionData[18])
{
    int result = QRVersion << 12;
    do
    {
        int pos = highest_one_bit_position_of(result);
        int gx = VERSION_GX << (pos - 12);
        result ^= gx;
    } while (highest_one_bit_position_of(result) >= 12);
    result |= QRVersion << 12;
    for (int i = 0; i < 18; i++)
        versionData[i] = (result >> i) & 1;
}

void add_version_data(int versionData[18], size_t QRWidth, int **QRData)
{
    for (int i = 0; i < sizeof(versionData) / sizeof(int); i++)
        QRData[i / 3][(QRWidth - 11) + i % 3] = QRData[i % 3 + QRWidth - 11][i / 3] = versionData[i];
}

void add_position_detectors(size_t QRWidth, int **QRData)
{
    copy_to_qr_code(0, 0, POSITION_DETECTOR, PD_WIDTH, QRData);
    copy_to_qr_code(0, QRWidth - PD_WIDTH, POSITION_DETECTOR, PD_WIDTH, QRData);
    copy_to_qr_code(QRWidth - PD_WIDTH, 0, POSITION_DETECTOR, PD_WIDTH, QRData);
}

void copy_to_qr_code(int offsetI, int offsetJ, int **copyFrom, size_t len, int **QRData)
{
    for (int i = 0; i < len; i++)
        for (int j = 0; j < len; j++)
            QRData[offsetI + i][offsetJ + j] = copyFrom[i][j];
}

void add_timing_patterns(size_t QRWidth, int **QRData)
{
    int iH = PD_WIDTH - 1;
    int jH = PD_WIDTH + 1;
    int iV = PD_WIDTH + 1;
    int jV = PD_WIDTH - 1;
    int len = QRWidth - (PD_WIDTH + 1) * 2;
    int val = TRUE_READ_ONLY;
    for (int i = 0; i < len; i++)
    {
        QRData[iH][jH++] = val;
        QRData[iV++][jV] = val;
        val = val == TRUE_READ_ONLY ? FALSE_READ_ONLY : TRUE_READ_ONLY;
    }
}

int *gen_alignment_coords(int version, int *coords, int QRVersion)
{
    int *tmp = (int *)malloc(sizeof(int) * 6);
    int array[] = {6, 34, 60, 86, 112, 138};
    for (int i = 0; i < sizeof(array) / sizeof(int); i++)
        tmp[i] = array[i];
    if (version == 32)
        return tmp;
    int numOfCoords = 2 + QRVersion / 7;
    coords = (int *)(malloc(sizeof(int) * numOfCoords));
    coords[0] = 6;
    coords[numOfCoords - 1] = 4 * version + 10;
    int diff = 4 * (version + 1);
    double gap = (double)diff / (numOfCoords - 1);
    gap = ((int)gap / 2 * 2) + (fmod(gap, 2.0) == 0.0 ? 0.0 : 2.0);
    coords[1] = 6 + (diff - (int)gap * (numOfCoords - 2));
    for (int i = 2; i < numOfCoords; i++)
        coords[i] = coords[i - 1] + (int)gap;
    return coords;
}

void add_alignment_coords(int QRVersion, int **QRData)
{
    if (QRVersion == 1)
        return;
    int *coords;
    coords = gen_alignment_coords(QRVersion, coords, QRVersion);
    int numOfCoords = 2 + QRVersion / 7;
    for (int i = 0; i < numOfCoords; i++)
    {
        for (int j = 0; j < numOfCoords; j++)
        {
            if (i == 0 && j == 0)
                continue;
            if (i == 0 && j == numOfCoords - 1)
                continue;
            if (i == numOfCoords - 1 && j == 0)
                continue;
            copy_to_qr_code(coords[i] - 2, coords[j] - 2, ALIGNMENT_PATTERN, AP_WIDTH, QRData);
        }
    }
}

int *byte_mode(char *inputData, int *output, int QRVersion)
{
    char *result = "";
    for (int i = 0; i < strlen(inputData); i++)
    {
        char binary[8];
        int_to_binay_string((int)inputData[i], binary, sizeof(binary));
        strcat(result, binary);
    }
    int ccBitCount = QRVersion < 10 ? 8 : 16;
    char characterCount[ccBitCount];
    int_to_binay_string(strlen(inputData), characterCount, ccBitCount);
    char str[BUFFER_SIZE];
    strcpy(str, BYTE_MODE);
    strcat(str, characterCount);
    strcat(str, result);
    output = malloc(strlen(str) * sizeof(int));
    for (int i = 0; i < strlen(str); i++)
        output[i] = str[sizeof(str) - i - 1] == '1' ? TRUE : FALSE;
    return output;
}

void int_to_binay_string(int num, char *output, size_t len)
{
    for (int i = 0; i < len; i++)
        output[len - i - 1] = (char)((num >> i & 1) + 48);
}

void add_code_words(int *final_message, size_t final_message_len, int **QRData, size_t QRWidth)
{
    int index = final_message_len - 1;
    int j = QRWidth - 1;
    for (int k = 0; k < QRWidth / 2; k++)
    {
        int i = k % 2 == 0 ? QRWidth - 1 : 0;
        int inc = k % 2 == 0 ? -1 : 1;
        for (; k % 2 == 0 ? i > -1 : i < QRWidth; i += inc)
        {
            if (i == TIMING_PATTERN)
                continue;
            if (QRData[i][j] != TRUE_READ_ONLY && QRData[i][j] != FALSE_READ_ONLY)
                QRData[i][j] = final_message[index--];
            if (QRData[i][j - 1] != TRUE_READ_ONLY && QRData[i][j - 1] != FALSE_READ_ONLY)
                QRData[i][j - 1] = final_message[index--];
        }
        j -= 2;
        if (j == TIMING_PATTERN)
            j -= 1;
    }
}

int num_of_code_words(int QRVersion)
{
    size_t QRWidth = 4 * QRVersion + 17;
    int codewords = QRWidth * QRWidth;
    codewords -= 3 * 64;
    codewords -= QRVersion > 6 ? 67 : 31;
    int alignmentPatterns = QRVersion == 1 ? 0 : ((2 + QRVersion / 7) * (2 + QRVersion / 7) - 3) * 25;
    codewords -= alignmentPatterns;
    int timingPattern = 2 * (QRWidth - 16) - (QRVersion / 7) * 2 * 5;
    codewords -= timingPattern;
    return codewords / 8;
}

int concat_bits_to_bytes(int *message, int *output, size_t message_len)
{
    if (message_len % 8 != 0)
        return message_len % 8;
    output = malloc(message_len / 8 * sizeof(int));
    for (int i = 0; i < message_len; i += 8)
    {
        int codeword = 0;
        for (int j = 0; j < 8; j++)
            codeword |= (message[i + j] % 2) << j;
        output[i / 8] = codeword;
    }
    return 0;
}

int get_qr_version(char *message, int ECL)
{
    int messageLength = -1;
    if (strlen(message) == 0)
        return 1;
    messageLength = strlen(message) < 256 ? 8 : strlen(message) < 1024 ? 10 : strlen(message) < 4096 ? 12 : -1;
    if (messageLength == -1)
        return messageLength;
    messageLength += 4 + 8 * strlen(message);
    int byteLength = (int)ceil((double)messageLength / 8.0);
    for (int qr_version = 1; qr_version <= 40; qr_version++)
        if (messageLength >= CODEWORD_CAPACITY[ECL][qr_version - 1])
            return qr_version;
    return -1;
}

int add_pad_bytes(char *message, char *paddedString, int ECL, int QRVersion)
{
    paddedString = malloc(BUFFER_SIZE);
    strcpy(paddedString, message);
    int maxBitLength = CODEWORD_CAPACITY[ECL][QRVersion - 1] * 8;
    if (maxBitLength < strlen(message))
        return -1;
    if (maxBitLength - strlen(message) <= 4)
    {
        char *terminator = "";
        strcat(paddedString, repeat_string("0", maxBitLength - strlen(message), terminator));
        return 0;
    }
    else
    {
        strcat(paddedString, TERMINATOR);
    }
    int remainingBitsToByte = (8 - strlen(paddedString) % 8) % 8;
    char *remainderBits;
    strcat(paddedString, repeat_string("0", remainingBitsToByte, remainderBits));
    int remainingBytes = (maxBitLength - strlen(paddedString)) / 8;
    for (int i = 0; i < remainingBytes; i++)
        strcat(paddedString, i % 2 == 0 ? TWO_THREE_SIX : SEVENTEEN);
    return 0;
}

char *repeat_string(const char *string, int numberOfTimes, char *result)
{
    result = malloc(strlen(string) * numberOfTimes);
    for (int tally = 0; tally < numberOfTimes; tally++)
        strcat(result, string);
    return result;
}

int gen_block_stucture(int *blockStructure, size_t len, int QRVersion, int ECL)
{
    if (blockStructure == NULL || len <= 0)
        return -1;
    //array Structure: blockStructure[0 % 3] = numOfDataCodeWords per block
    //                 blockStructure[1 % 3] = numOfErrorCodewords per block
    //                 blockStructure[2 % 3] = number of blocks
    int numOfCodeWords = num_of_code_words(QRVersion);
    blockStructure = malloc((numOfCodeWords & BLOCK_COUNT[ECL][QRVersion - 1] == 0 ? 3 : 6) * sizeof(int));
    blockStructure[0] = (numOfCodeWords - ERROR_CODES[ECL][QRVersion - 1]) / BLOCK_COUNT[ECL][QRVersion - 1];
    blockStructure[1] = numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1] - blockStructure[0];
    int n = numOfCodeWords - (numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1]) * BLOCK_COUNT[ECL][QRVersion - 1];
    blockStructure[2] = BLOCK_COUNT[ECL][QRVersion - 1] - n;
    if (len == 6)
    {
        blockStructure[3] = blockStructure[0] + 1;
        blockStructure[4] = blockStructure[1];
        blockStructure[5] = n;
    }
    return 0;
}

int gen_final_message(int *message, int *finalMessage, size_t len)
{
    //do shit, don't forget Term.c
}

int gen_error_block(int **messageBlock, size_t messageBLockWidth, size_t messageBlockHeight, int numOFErrorCodewords, int **errorBlock)
{
    // do some hit also
}

//*Term genErrorPolyNomial(int *message, size_t messageLen, int numOfErrorCodewords);

int to_bit_array(int *message, size_t messageLen, int *bitArray)
{
    if (reverse(message, messageLen) == ERROR)
        ;
    return ERROR;
    bitArray = malloc(messageLen * 8 * sizeof(int));
    for (int i = 0; i < messageLen; i++)
    {
        int codeword = message[i];
        for (int j = 0; j < 8; j++)
            bitArray[i * 8 + j] = ((codeword >> j) & 1) + 2;
    }
    return SUCCESS;
}

int reverse(int *array, size_t array_len)
{
    if (array == NULL || array_len < 0)
        return -1;
    for (int i = 0; i < array_len / 2; i++)
    {
        int tmp = array[i];
        array[i] = array[array_len - i - 1];
        array[array_len - i - 1] = tmp;
    }
    return SUCCESS;
}

void add_remaining_bits(int *finalMessage, size_t finalMessageLen, int *result, int QRVersion)
{
    if (REMAINDER_BITS[QRVersion - 1] != 0)
    {
        size_t resultLen = finalMessageLen + REMAINDER_BITS[QRVersion - 1];
        result = malloc(resultLen * sizeof(int));
        for (int i = 0; i < finalMessageLen; i++)
            result[i] = finalMessage[i];
        for (int i = finalMessageLen; i < resultLen; i++)
            result[i] = REMAINDER_BITS[QRVersion - 1];
    }
}

void add_seperators(int **QRData, size_t QRWidth)
{
    for (int i = 0; i < 8; i++)
    {
        QRData[i][7] = QRData[7][i] = FALSE_READ_ONLY;
        QRData[i][QRWidth - 8] = QRData[QRWidth - 8][i] = FALSE_READ_ONLY;
        QRData[QRWidth - i - 1][7] = QRData[7][QRWidth - i - 1] = FALSE_READ_ONLY;
    }
}

void add_mask(int **QRData, size_t QRWidth, int ECL, int formatData[15], int QRVersion)
{
    int correctMpr = 0;
    int mpr = 0;
    int **QRDataOriginal = init_matrix(QRWidth, QRWidth);
    copy_matrix(QRData, QRDataOriginal, QRWidth);
    int **QRDataCopy = init_matrix(QRWidth, QRWidth);
    copy_matrix(QRData, QRDataCopy, QRWidth);
    int matrixPenalty = __INT32_MAX__;
    for (int i = 0; i < 8; i++)
    {
        switch (i)
        {
        case I_PLUS_J_MOD_2:
            mpr = I_PLUS_J_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = row % 2; column < QRWidth; column += 2)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= 1;
            break;
        case I_MOD_2:
            mpr = I_MOD_2;
            for (int row = 0; row < QRWidth; row += 2)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= 1;
            break;
        case J_MOD_3:
            mpr = J_MOD_3;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column += 3)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= 1;
            break;
        case I_PLUS_J_MOD_3:
            mpr = I_PLUS_J_MOD_3;
            for (int row = 0; row < QRWidth; row++)
                for (int column = (int[3]){0, 2, 1}[row % 3]; column < QRWidth; column += 3)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= 1;
            break;
        case I_DIV_2_PLUS_J_DIV_3_MOD_2:
            mpr = I_DIV_2_PLUS_J_DIV_3_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= (row / 2 + column / 3) % 2 ^ 1;
            break;
        case I_J_MOD_2_PLUS_I_J_MOD_3:
            mpr = I_J_MOD_2_PLUS_I_J_MOD_3;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= row * column % 2 + row * column % 3 == 0 ? 1 : 0;
            break;
        case I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2:
            mpr = I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= (row * column % 2 + row * column % 3) % 2 ^ 1;
            break;
        case I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2:
            mpr = I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row][column] > 1)
                        QRDataCopy[row][column] ^= ((row + column % 2) + row * column % 3) % 2 ^ 1;
        }
        gen_format_data(ECL, mpr, formatData, 15);
        add_format_data(QRDataCopy, QRWidth, QRVersion, formatData);
        int penalty = get_matrix_penalty(QRDataCopy, QRWidth);
        if (penalty < matrixPenalty)
        {
            matrixPenalty = penalty;
            copy_matrix(QRDataCopy, QRData, QRWidth);
            correctMpr = mpr;
        }
        copy_matrix(QRDataOriginal, QRDataCopy, QRWidth);
    }
    gen_format_data(ECL, correctMpr, formatData, 15);
    add_format_data(QRData, QRWidth, QRVersion, formatData);
}

int get_matrix_penalty(int **QRData, size_t QRWidth)
{
    int penalty = 0;
    penalty += evaluate_consecutive_modules_penalty(QRData, QRWidth);
    penalty += evaluate_2_by_2_module_penalty(QRData, QRWidth);
    penalty += evaluate_pattern_penalty(QRData, QRWidth);
    penalty += evaluate_ratio_penalty(QRData, QRWidth);
    return penalty;
}

int evaluate_consecutive_modules_penalty(int **QRData, size_t QRWidth)
{
    int penalty = 0;
    for (int i = 0; i < QRWidth; i++)
    {
        int numOfSimilarModulesVertical = 1;
        int numOfSimilarModulesHorizontal = 1;
        for (int j = 1; j < QRWidth; j++)
        {
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

int evaluate_2_by_2_module_penalty(int **QRData, size_t QRWidth)
{
    int penalty = 0;
    for (int i = 0; i < QRWidth - 1; i++)
        for (int j = 0; j < QRWidth - 1; j++)
            if (QRData[i][j] % 2 == QRData[i][j + 1] % 2 && QRData[i][j + 1] % 2 == QRData[i + 1][j] % 2 && QRData[i + 1][j] % 2 == QRData[i + 1][j + 1] % 2)
                penalty += 3;
    return penalty;
}

int evaluate_pattern_penalty(int **QRData, size_t QRWidth)
{
    int penalty = 0;
    for (int i = 0; i < QRWidth - MODULE_PATTERN_LENGTH; i++)
        for (int j = 0; j < QRWidth - MODULE_PATTERN_LENGTH; j++)
        {
            int tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[i][j + tally] % 2 == MODULE_PATTERN_1[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
            tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[i][j + tally] % 2 == MODULE_PATTERN_2[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
            tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[j + tally][i] % 2 == MODULE_PATTERN_1[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
            tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[j + tally][i] % 2 == MODULE_PATTERN_2[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
        }
    return penalty;
}

int evaluate_ratio_penalty(int **QRData, size_t QRWidth)
{
    int darkModules = 0;
    for (int i = 0; i < QRWidth; i++)
        for (int j = 0; j < QRWidth; j++)
            darkModules += QRData[i][j] % 2;
    int ratio = (int)round((double)darkModules / (QRWidth * QRWidth) * 100);
    int previousMultiple = ratio - ratio % 5;
    int nextMultiple = previousMultiple + 5;
    return (int)fmin(abs(previousMultiple - 50) / 5, abs(nextMultiple - 50) / 5) * 10;
}

void add_white_border(int **QRData, size_t QRWidth)
{
    int **finalQRData = init_matrix(QRWidth + 8, QRWidth + 8);
    for (int i = 0; i < QRWidth; i++)
        for (int j = 0; j < QRWidth; j++)
            finalQRData[i + 4][j + 4] = QRData[i][j];
    QRData = finalQRData;
}

void copy_matrix(int **src, int **dst, size_t QRWidth)
{
    src = init_matrix(QRWidth, QRWidth);
    for (int i = 0; i < QRWidth; i++)
        for (int j = 0; j < QRWidth; j++)
            dst[i][j] = src[i][j];
}

void set_to_dark(int **QRData, size_t QRWidth)
{
    for (int i = 0; i < QRWidth; i++)
        for (int j = 0; j < QRWidth; j++)
            QRData[i][j] ^= 1;
}
int **init_matrix(size_t row, size_t column)
{
    int **matrix = (int **)malloc(row * sizeof(int *));
    for (int i = 0; i < row; i++)
        matrix[i] = (int *)malloc(column * sizeof(int));
    return matrix;
}

void free_matrix(int **matrix, size_t row)
{
    for (int i = 0; i < row; i++)
        free(matrix[i]);
    free(matrix);
}