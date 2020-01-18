#include "QRCode.hpp"
#define ANSI_COLOR_BLACK "\x1b[30m"
#define ANSI_COLOR_WHITE "\x1b[97m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"
// int main(const int arg, const char **argv)
// {
//     int ECL = 0;
//     std::int8_t *data = new std::int8_t[BUFFER_SIZE];
//     long int begin = std::time(NULL);
//     for(int i = 0; i < 10000; i++)
//     qrcode::gen_qr(ECL, "https://github.com/justahero/qrcode-wasm-bindgen", data);
//     long int end = std::time(NULL);
//     printf("time elapsed: %ld\ns", end - begin);
//     delete[] data;
// }
extern "C"
{
void gen_qr(int ECL, const char *message, std::int8_t *QRData)
{
    int QRVersion = qrcode::get_qr_version(message, ECL);
    int QRWidth = 17 + QRVersion * 4;
    int formatData[15] = {};
    int versionData[18] = {};
    int outputLen;
    int *messageBitStream = qrcode::byte_mode(message, QRVersion, outputLen, ECL);
    int *messageCodewords = new int[outputLen / 8];
    qrcode::concat_bits_to_bytes(messageBitStream, messageCodewords, outputLen);
    for(int i = 0; i < QRWidth * QRWidth; i++)
        QRData[i] = ERROR;
    std::memset(QRData, ERROR, QRWidth * QRWidth);
    qrcode::add_seperators(QRData, QRWidth);
    qrcode::add_position_detectors(QRWidth, QRData);
    qrcode::add_timing_patterns(QRWidth, QRData);
    QRData[(4 * QRVersion + 9) * QRWidth + 8] = TRUE_READ_ONLY;
    qrcode::add_format_data(QRData, QRWidth, QRVersion, formatData);
    if (QRVersion >= 7)
    {
        qrcode::gen_version_data(QRVersion, versionData);
        qrcode::add_version_data(versionData, QRWidth, QRData);
    }
    qrcode::add_alignment_pattern(QRVersion, QRData, QRWidth);
    qrcode::reverse(messageCodewords, outputLen / 8);
    
    int finalMessageLen;
    int *finalMessage = qrcode::gen_final_message(messageCodewords, outputLen / 8, QRVersion, ECL, finalMessageLen);
    int *finalMessageBitArray = qrcode::to_bit_array(finalMessage, finalMessageLen);
    int finalFinalMessageLen;
    qrcode::add_remaining_bits(finalMessageBitArray, finalMessageLen * 8, QRVersion, finalFinalMessageLen);
    qrcode::add_code_words(finalMessageBitArray, finalFinalMessageLen, QRData, QRWidth);
    qrcode::add_mask(QRData, QRWidth, ECL, formatData, QRVersion);
    qrcode::add_white_border(QRData, QRWidth);
    QRWidth += 8;

    //SHift all the values down one byte so the QRWidth could be stored in the first byte.
    int8_t tmp = QRData[0];
    for (int i = 0; i < QRWidth * QRWidth; i++)
    {
        std::int8_t tmp1 = QRData[i + 1];
        QRData[i + 1] = tmp;
        tmp = tmp1;
    }
    QRData[0] = QRWidth;
    delete[] messageBitStream;
    delete[] messageCodewords;
    delete[] finalMessage;
    delete[] finalMessageBitArray;
}
}
void qrcode::gen_format_data(int ECL, int mpr, int formatData[15], int len)
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
    for (int i = 0; i < len; i++)
        formatData[i] = (result >> i) & 1;
}

int qrcode::to_int(int array[], int len)
{
    int result = 0;
    for (int i = 0; i < len; i++)
        result |= (array[i] << i);
    return result;
}

int qrcode::highest_one_bit_position_of(int n)
{
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    int r = n - (n >> 1);
    return (int)(std::log(r) / std::log(2));
}

void qrcode::add_format_data(std::int8_t *QRData, int QRWidth, int QRVersion, int formatData[15])
{
    int fBit = 0;
    int index = 14;
    for (; fBit <= 5; fBit++)
    {
        QRData[8 * QRWidth + fBit] = formatData[index];
        QRData[(QRWidth - fBit - 1) * QRWidth + 8] = formatData[index--];
    }
    QRData[(QRWidth - fBit - 1) * QRWidth + 8] = formatData[index];
    QRData[(8) * QRWidth + (7)] = formatData[index--];
    fBit++;
    QRData[(8) * QRWidth + (8)] = formatData[index];
    QRData[(8) * QRWidth + (QRWidth - 8)] = formatData[index--];
    fBit++;
    QRData[(7) * QRWidth + (8)] = formatData[index];
    QRData[(8) * QRWidth + (QRWidth - 7)] = formatData[index--];
    fBit++;
    for (; fBit <= 14; fBit++)
    {
        QRData[(14 - fBit) * QRWidth + (8)] = formatData[index];
        QRData[(8) * QRWidth + (QRWidth - (15 - fBit))] = formatData[index--];
    }
}

void qrcode::gen_version_data(int QRVersion, int versionData[18])
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

void qrcode::add_version_data(int versionData[18], int QRWidth, std::int8_t *QRData)
{
    for (int i = 0; i < 18; i++)
    {
        QRData[(i / 3) * QRWidth + ((QRWidth - 11) + i % 3)] = versionData[i];
        QRData[(i % 3 + QRWidth - 11) * QRWidth + i / 3] = versionData[i];
    }
}

void qrcode::add_position_detectors(int QRWidth, std::int8_t *QRData)
{
    copy_PD_to_qr_code(0, 0, QRData, QRWidth);
    copy_PD_to_qr_code(0, QRWidth - PD_WIDTH, QRData, QRWidth);
    copy_PD_to_qr_code(QRWidth - PD_WIDTH, 0, QRData, QRWidth);
}

void qrcode::copy_PD_to_qr_code(int offsetI, int offsetJ, std::int8_t *QRData, int QRWidth)
{
    for (int i = 0; i < PD_WIDTH; i++)
        std::copy(POSITION_DETECTOR[i], POSITION_DETECTOR[i] + PD_WIDTH, QRData + (offsetI + i) * QRWidth + offsetJ);
}

void qrcode::copy_AP_to_qr_code(int offsetI, int offsetJ, std::int8_t *QRData, int QRWidth)
{
    for (int i = 0; i < AP_WIDTH; i++)
        std::copy(ALIGNMENT_PATTERN[i], ALIGNMENT_PATTERN[i] + AP_WIDTH, QRData + (offsetI + i) * QRWidth + offsetJ);
}

void qrcode::add_timing_patterns(int QRWidth, std::int8_t *QRData)
{
    int iH = PD_WIDTH - 1;
    int jH = PD_WIDTH + 1;
    int iV = PD_WIDTH + 1;
    int jV = PD_WIDTH - 1;
    int len = QRWidth - (PD_WIDTH + 1) * 2;
    int val = TRUE_READ_ONLY;
    for (int i = 0; i < len; i++)
    {
        QRData[(iH)*QRWidth + (jH++)] = val;
        QRData[(iV++) * QRWidth + (jV)] = val;
        val = val == TRUE_READ_ONLY ? FALSE_READ_ONLY : TRUE_READ_ONLY;
    }
}

void qrcode::gen_alignment_coords(int version, int *&coords, int QRVersion)
{
    // if (version == 32)
    // {
    //     coords = new int[6]{6, 34, 60, 86, 112, 138};
    //     return;
    // }
    int numOfCoords = 2 + QRVersion / 7;
    coords = new int[numOfCoords];
    coords[0] = 6;
    coords[numOfCoords - 1] = 4 * version + 10;
    int diff = 4 * (version + 1);
    double gap = (double)diff / (numOfCoords - 1);
    gap = ((int)gap / 2 * 2) + (fmod(gap, 2.0) == 0.0 ? 0.0 : 2.0);
    coords[1] = 6 + (diff - (int)gap * (numOfCoords - 2));
    for (int i = 2; i < numOfCoords; i++)
        coords[i] = coords[i - 1] + (int)gap;
}

void qrcode::add_alignment_pattern(int QRVersion, std::int8_t *QRData, int QRWidth)
{
    if (QRVersion == 1)
        return;
    int *coords = new int[6];
    int tmp[6] = {6, 34, 60, 86, 112, 138};
    std::copy(tmp, tmp + 6, coords);

    if (QRVersion != 32)
    {
        delete[] coords;
        gen_alignment_coords(QRVersion, coords, QRVersion);
    }
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
            copy_AP_to_qr_code(coords[i] - 2, coords[j] - 2, QRData, QRWidth);
        }
    }
    delete[] coords;
}

int *qrcode::byte_mode(const char *inputData, int QRVersion, int &outputLen, int ECL)
{
    char *result = new char[BUFFER_SIZE]();
    for (std::size_t i = 0; i < std::strlen(inputData); i++)
    {
        char *binary = int_to_binary_string((int)inputData[i], 8);
        if (i == 0)
            std::strcpy(result, binary);
        else
            std::strcat(result, binary);
        delete[] binary;
    }
    int ccBitCount = QRVersion < 10 ? 8 : 16;
    char *characterCount = int_to_binary_string(std::strlen(inputData), ccBitCount);
    char *str = new char[BUFFER_SIZE]();
    std::strcpy(str, BYTE_MODE);
    std::strcat(str, characterCount);
    std::strcat(str, result);
    add_pad_bytes(str, ECL, QRVersion);
    int *output = new int[std::strlen(str)];
    outputLen = std::strlen(str);
    for (std::size_t i = 0; i < std::strlen(str); i++)
        output[i] = str[outputLen - i - 1] == '1' ? TRUE : FALSE;
    delete[] result;
    delete[] str;
    delete[] characterCount;
    return output;
}

char *qrcode::int_to_binary_string(int num, int len)
{
    char *result = new char[len + 1];
    for (int i = 0; i < len; i++)
        result[len - i - 1] = (char)((num >> i & 1) + 48);
    result[len] = '\0';
    return result;
}

void qrcode::add_code_words(int *&final_message, int final_message_len, std::int8_t *QRData, int QRWidth)
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
            if (QRData[i * QRWidth + j] != TRUE_READ_ONLY && QRData[i * QRWidth + j] != FALSE_READ_ONLY && index > -1)
                QRData[(i)*QRWidth + (j)] = final_message[index--];
            if (QRData[i * QRWidth + j - 1] != TRUE_READ_ONLY && QRData[i * QRWidth + j - 1] != FALSE_READ_ONLY && index > -1)
                QRData[(i)*QRWidth + (j - 1)] = final_message[index--];
        }
        j -= 2;
        if (j == TIMING_PATTERN)
            j -= 1;
    }
}

int qrcode::num_of_code_words(int QRVersion)
{
    int QRWidth = 4 * QRVersion + 17;
    int codewords = QRWidth * QRWidth;
    codewords -= 3 * 64;
    codewords -= QRVersion > 6 ? 67 : 31;
    int alignmentPatterns = QRVersion == 1 ? 0 : ((2 + QRVersion / 7) * (2 + QRVersion / 7) - 3) * 25;
    codewords -= alignmentPatterns;
    int timingPattern = 2 * (QRWidth - 16) - (QRVersion / 7) * 2 * 5;
    codewords -= timingPattern;
    return codewords / 8;
}

void qrcode::concat_bits_to_bytes(int *&message, int *&output, int message_len)
{
    if (message_len % 8 == 0){
    for (int i = 0; i < message_len; i += 8)
    {
        int codeword = 0;
        for (int j = 0; j < 8; j++)
            codeword |= (message[i + j] % 2) << j;
        output[i / 8] = codeword;
    }
    }
}

int qrcode::get_qr_version(const char *message, int ECL)
{
    int messageLength = -1;
    int len = std::strlen(message);
    if (len == 0)
        return 1;
    messageLength = len < 256 ? 8 : len < 1024 ? 10 : len < 4096 ? 12 : -1;
    if (messageLength == -1)
        return messageLength;
    messageLength += 4 + 8 * len;
    int byteLength = (int)std::ceil((double)messageLength / 8.0);
    for (int qr_version = 1; qr_version <= 40; qr_version++)
        if (byteLength <= CODEWORD_CAPACITY[ECL][qr_version - 1])
            return qr_version;
    return ERROR;
}

void qrcode::add_pad_bytes(char *&message, int ECL, int QRVersion)
{
    int maxBitLength = CODEWORD_CAPACITY[ECL][QRVersion - 1] * 8;
    int len = (int)std::strlen(message);
    if (maxBitLength >= len){
    int n = maxBitLength - len;
    if (n <= 4 && n > 0){
        for(int i = 0; i < n; i++)
            std::strcat(message, "0");
    }
    else
    {
        std::strcat(message, TERMINATOR);
        int remainingBitsToByte = (8 - std::strlen(message) % 8) % 8;
        if (remainingBitsToByte > 0)
            for(int i = 0; i < remainingBitsToByte; i++)
                std::strcat(message, "0");
        int remainingBytes = (maxBitLength - std::strlen(message)) / 8;
        for (int i = 0; i < remainingBytes; i++)
            std::strcat(message, i % 2 == 0 ? TWO_THREE_SIX : SEVENTEEN);
    }
    }
}

void qrcode::gen_block_stucture(int len, int QRVersion, int ECL, int &blockStructureLen, int *&blockStructure)
{
    //array Structure: blockStructure[0 % 3] = numOfDataCodeWords per block
    //                 blockStructure[1 % 3] = numOfErrorCodewords per block
    //                 blockStructure[2 % 3] = number of blocks
    int numOfCodeWords = num_of_code_words(QRVersion);
    blockStructureLen = numOfCodeWords % BLOCK_COUNT[ECL][QRVersion - 1] == 0 ? 3 : 6;
    blockStructure = new int[blockStructureLen];
    blockStructure[0] = (numOfCodeWords - ERROR_CODES[ECL][QRVersion - 1]) / BLOCK_COUNT[ECL][QRVersion - 1];
    blockStructure[1] = numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1] - blockStructure[0];
    int n = numOfCodeWords - (numOfCodeWords / BLOCK_COUNT[ECL][QRVersion - 1]) * BLOCK_COUNT[ECL][QRVersion - 1];
    blockStructure[2] = BLOCK_COUNT[ECL][QRVersion - 1] - n;
    if (blockStructureLen == 6)
    {
        blockStructure[3] = blockStructure[0] + 1;
        blockStructure[4] = blockStructure[1];
        blockStructure[5] = n;
    }
}

int *qrcode::gen_final_message(int *message, int len, int QRVersion, int ECL, int &finalMessageLen)
{
    int position = 0;
    int *blockStructure;
    int blockStructureLen;
    int *messageBlockLens;
    gen_block_stucture(len, QRVersion, ECL, blockStructureLen, blockStructure);
    int blockCount = BLOCK_COUNT[ECL][QRVersion - 1];
    int **messageBlock = new int *[blockCount];
    messageBlockLens = new int[blockCount];
    for (int i = 0; i < blockStructure[2]; i++)
    {
        messageBlock[i] = new int[blockStructure[0]];
        messageBlockLens[i] = blockStructure[0];
        for (int j = 0; j < blockStructure[0]; j++)
            messageBlock[i][j] = message[position + j];
        position += blockStructure[0];
    }
    if (blockStructureLen == 6)
        for (int i = blockStructure[2]; i < blockStructure[2] + blockStructure[5]; i++)
        {
            messageBlock[i] = new int[blockStructure[3]];
            messageBlockLens[i] = blockStructure[3];
            for (int j = 0; j < blockStructure[3]; j++)
                messageBlock[i][j] = message[position + j];
            position += blockStructure[3];
        }
    int *weavedMessage = new int[len];
    if (blockStructureLen == 6)
    {
        for (int i = 0; i < len - blockStructure[5]; i++)
            weavedMessage[i] = messageBlock[i % blockCount][i / blockCount];
        for (int i = 0; i < blockStructure[5]; i++)
            weavedMessage[len - (blockStructure[5] - i)] = messageBlock[blockStructure[2] + i][blockStructure[3] - 1];
    }
    else
        for (int i = 0; i < len; i++)
            weavedMessage[i] = messageBlock[i % blockCount][i / blockCount];
    int *weavedErrorCodewords = gen_error_block(messageBlock, blockCount, messageBlockLens, blockStructure[1]);
    finalMessageLen = blockCount * (blockStructure[0] + blockStructure[1]);
    if (blockStructureLen > 3)
        finalMessageLen += blockStructure[5];
    int *finalMessage = new int[finalMessageLen];
    for (int i = 0; i < len; i++)
        finalMessage[i] = weavedMessage[i];
    for (int i = 0; i < blockCount * blockStructure[1]; i++)
        finalMessage[len + i] = weavedErrorCodewords[i];
    delete[] blockStructure;
    delete[] messageBlockLens;
    delete[] weavedMessage;
    delete[] weavedErrorCodewords;
    for (int i = 0; i < blockCount; i++)
        delete[] messageBlock[i];
    delete[] messageBlock;
    return finalMessage;
}

int *qrcode::gen_error_block(int **&messageBlock, int messageBlockHeight, int *messageBlockRowLens, int numOFErrorCodewords)
{
    int **errorBlock = new int *[messageBlockHeight];
    for (int i = 0; i < messageBlockHeight; i++)
    {
        int errorPolyLen;
        Polynomial errorPolynomial = gen_error_polynomial(messageBlock[i], messageBlockRowLens[i], numOFErrorCodewords, &errorPolyLen);
        to_array(errorPolynomial, errorBlock[i]);
    }
    int *weavedCodewords = new int[numOFErrorCodewords * messageBlockHeight];
    for (int i = 0; i < numOFErrorCodewords * messageBlockHeight; i++)
        weavedCodewords[i] = errorBlock[i % messageBlockHeight][i / messageBlockHeight];
    for (int i = 0; i < messageBlockHeight; i++)
        delete[] errorBlock[i];
    delete[] errorBlock;
    return weavedCodewords;
}

Polynomial qrcode::gen_error_polynomial(int *message, int messageLen, int numOfErrorCodewords, int *errorPolyLen)
{
    Polynomial messagePolynomial = to_polynomial_temp(message, messageLen);
    Polynomial generatorPolynomial = gen_polynomial(numOfErrorCodewords);
    multiply_x(messagePolynomial, messagePolynomial, numOfErrorCodewords);
    int difference = messagePolynomial[0].xExponent - generatorPolynomial[0].xExponent;
    Polynomial shiftedGenPolynomial = Polynomial(generatorPolynomial.length);
    multiply_x(shiftedGenPolynomial, generatorPolynomial, difference);
    multiply_alpha(shiftedGenPolynomial, shiftedGenPolynomial, messagePolynomial[0].alphaExponent);
    Polynomial errorPolynomial = messagePolynomial ^ shiftedGenPolynomial;
    for (int i = 0; i < messageLen - 1; i++)
    {
        difference = errorPolynomial[0].xExponent - generatorPolynomial[0].xExponent;
        multiply_x(shiftedGenPolynomial, generatorPolynomial, difference);
        multiply_alpha(shiftedGenPolynomial, shiftedGenPolynomial, errorPolynomial[0].alphaExponent);
        errorPolynomial = errorPolynomial ^ shiftedGenPolynomial;
    }
    return errorPolynomial;
}

int *qrcode::to_bit_array(int *message, int messageLen)
{
    reverse(message, messageLen);
    int *bitArray = new int[messageLen * 8];
    for (int i = 0; i < messageLen; i++)
    {
        int codeword = message[i];
        for (int j = 0; j < 8; j++)
            bitArray[i * 8 + j] = ((codeword >> j) & 1) + 2;
    }
    return bitArray;
}

void qrcode::reverse(int *array, int array_len)
{
    if (array != NULL && array_len >= 0)
    for (int i = 0; i < array_len / 2; i++)
    {
        int tmp = array[i];
        array[i] = array[array_len - i - 1];
        array[array_len - i - 1] = tmp;
    }
}

void qrcode::add_remaining_bits(int *&finalMessage, int finalMessageLen, int QRVersion, int &len)
{
    len = finalMessageLen + REMAINDER_BITS[QRVersion - 1];
    if (REMAINDER_BITS[QRVersion - 1] != 0)
    {
        int *tmp = finalMessage;
        finalMessage = new int[len];
        for (int i = 0; i < finalMessageLen; i++)
            finalMessage[len - i - 1] = tmp[finalMessageLen - i - 1];
        delete[] tmp;
        for (int i = 0; i < REMAINDER_BITS[QRVersion - 1]; i++)
            finalMessage[i] = TRUE;
    }
}

void qrcode::add_seperators(std::int8_t *QRData, int QRWidth)
{
    for (int i = 0; i < 8; i++)
    {
        QRData[i * QRWidth + 7] = QRData[7 * QRWidth + i] = FALSE_READ_ONLY;
        QRData[i * QRWidth + QRWidth - 8] = QRData[(QRWidth - 8) * QRWidth + i] = FALSE_READ_ONLY;
        QRData[(QRWidth - i - 1) * QRWidth + 7] = QRData[7 * QRWidth + (QRWidth - i - 1)] = FALSE_READ_ONLY;
    }
}

void qrcode::add_mask(std::int8_t *QRData, int QRWidth, int ECL, int formatData[15], int QRVersion)
{
    int correctMpr = 0;
    int mpr = 0;
    std::int8_t *QRDataOriginal = new std::int8_t[QRWidth * QRWidth];
    std::copy(QRData, QRData + QRWidth * QRWidth, QRDataOriginal);
    std::int8_t *QRDataCopy = new std::int8_t[QRWidth * QRWidth];
    std::copy(QRData, QRData + QRWidth * QRWidth, QRDataCopy);
    int matrixPenalty = __INT32_MAX__;
    for (int i = 0; i < 8; i++)
    {
        switch (i)
        {
        case I_PLUS_J_MOD_2:
            mpr = I_PLUS_J_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = row % 2; column < QRWidth; column += 2)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= 1;
            break;
        case I_MOD_2:
            mpr = I_MOD_2;
            for (int row = 0; row < QRWidth; row += 2)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= 1;
            break;
        case J_MOD_3:
            mpr = J_MOD_3;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column += 3)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= 1;
            break;
        case I_PLUS_J_MOD_3:
            mpr = I_PLUS_J_MOD_3;
            for (int row = 0; row < QRWidth; row++)
                for (int column = (int[3]){0, 2, 1}[row % 3]; column < QRWidth; column += 3)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= 1;
            break;
        case I_DIV_2_PLUS_J_DIV_3_MOD_2:
            mpr = I_DIV_2_PLUS_J_DIV_3_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= (row / 2 + column / 3) % 2 ^ 1;
            break;
        case I_J_MOD_2_PLUS_I_J_MOD_3:
            mpr = I_J_MOD_2_PLUS_I_J_MOD_3;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= row * column % 2 + row * column % 3 == 0 ? 1 : 0;
            break;
        case I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2:
            mpr = I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= (row * column % 2 + row * column % 3) % 2 ^ 1;
            break;
        case I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2:
            mpr = I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2;
            for (int row = 0; row < QRWidth; row++)
                for (int column = 0; column < QRWidth; column++)
                    if (QRDataCopy[row * QRWidth + column] > 1)
                        QRDataCopy[row * QRWidth + column] ^= ((row + column % 2) + row * column % 3) % 2 ^ 1;
        }
        gen_format_data(ECL, mpr, formatData, 15);
        add_format_data(QRDataCopy, QRWidth, QRVersion, formatData);
        int penalty = get_matrix_penalty(QRDataCopy, QRWidth);
        if (penalty < matrixPenalty)
        {
            matrixPenalty = penalty;
            std::copy(QRDataCopy, QRDataCopy + QRWidth * QRWidth, QRData);
            correctMpr = mpr;
        }
        std::copy(QRDataOriginal, QRDataOriginal + QRWidth * QRWidth, QRDataCopy);
    }
    gen_format_data(ECL, correctMpr, formatData, 15);
    add_format_data(QRData, QRWidth, QRVersion, formatData);
    delete[] QRDataCopy;
    delete[] QRDataOriginal;
}

int qrcode::get_matrix_penalty(std::int8_t *QRData, int QRWidth)
{
    int penalty = 0;
    penalty += evaluate_consecutive_modules_penalty(QRData, QRWidth);
    penalty += evaluate_2_by_2_module_penalty(QRData, QRWidth);
    penalty += evaluate_pattern_penalty(QRData, QRWidth);
    penalty += evaluate_ratio_penalty(QRData, QRWidth);
    return penalty;
}

int qrcode::evaluate_consecutive_modules_penalty(std::int8_t *QRData, int QRWidth)
{
    int penalty = 0;
    for (int i = 0; i < QRWidth; i++)
    {
        int numOfSimilarModulesVertical = 1;
        int numOfSimilarModulesHorizontal = 1;
        for (int j = 1; j < QRWidth; j++)
        {
            if (QRData[(j - 1) * QRWidth + i] % 2 == QRData[j * QRWidth + i] % 2)
                numOfSimilarModulesVertical++;
            else
                numOfSimilarModulesVertical = 1;
            if (numOfSimilarModulesVertical == 5)
                penalty += 3;
            else if (numOfSimilarModulesVertical > 5)
                penalty += 1;
            if (QRData[i * QRWidth + j] % 2 == QRData[i * QRWidth + j - 1] % 2)
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

int qrcode::evaluate_2_by_2_module_penalty(std::int8_t *&QRData, int QRWidth)
{
    int penalty = 0;
    for (int i = 0; i < QRWidth - 1; i++)
        for (int j = 0; j < QRWidth - 1; j++)
            if (QRData[i * QRWidth + j] % 2 == QRData[i * QRWidth + j + 1] % 2 && QRData[i * QRWidth + j + 1] % 2 == QRData[(i + 1) * QRWidth + j] % 2 && QRData[(i + 1) * QRWidth + j] % 2 == QRData[(i + 1) * QRWidth + j + 1] % 2)
                penalty += 3;
    return penalty;
}

int qrcode::evaluate_pattern_penalty(std::int8_t *QRData, int QRWidth)
{
    int penalty = 0;
    for (int i = 0; i < QRWidth - MODULE_PATTERN_LENGTH; i++)
        for (int j = 0; j < QRWidth - MODULE_PATTERN_LENGTH; j++)
        {
            int tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[(i)*QRWidth + (j + tally)] % 2 == MODULE_PATTERN_1[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
            tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[(i)*QRWidth + (j + tally)] % 2 == MODULE_PATTERN_2[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
            tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[(j + tally) * QRWidth + (i)] % 2 == MODULE_PATTERN_1[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
            tally = 0;
            while (tally < MODULE_PATTERN_LENGTH)
            {
                if (QRData[(j + tally) * QRWidth + (i)] % 2 == MODULE_PATTERN_2[tally] % 2)
                    tally++;
                else
                    break;
            }
            if (tally == MODULE_PATTERN_LENGTH)
                penalty += 40;
        }
    return penalty;
}

int qrcode::evaluate_ratio_penalty(std::int8_t *QRData, int QRWidth)
{
    int darkModules = 0;
    for(int i = 0; i < QRWidth * QRWidth; i++)
        darkModules += QRData[i] % 2;
    int ratio = (int)round((double)darkModules / (QRWidth * QRWidth) * 100);
    int previousMultiple = ratio - ratio % 5;
    int nextMultiple = previousMultiple + 5;
    return std::min(std::abs(previousMultiple - 50) / 5, std::abs(nextMultiple - 50) / 5) * 10;
}

void qrcode::add_white_border(std::int8_t *QRData, int QRWidth)
{
    int *tmp = new int[(QRWidth + 8) * (QRWidth + 8)]();
    for (int i = 0; i < QRWidth; i++)
        for (int j = 0; j < QRWidth; j++)
            tmp[(i + 4) * (QRWidth + 8) + j + 4] = QRData[i * QRWidth + j];
    std::copy(tmp, tmp + (QRWidth + 8) * (QRWidth + 8), QRData);
    delete[] tmp;
}

void qrcode::print_qr(std::int8_t *QRData, int QRWidth)
{
    for(int i = 0; i < QRWidth * QRWidth; i++){
        printf("%s██" ANSI_COLOR_RESET, QRData[i] % 2 == 0 ? ANSI_COLOR_WHITE : QRData[i] % 2 == 1 ? ANSI_COLOR_BLACK : ANSI_COLOR_RED);
        if(i != 0 && i % QRWidth == 0)
            printf("\n");
    }
}

void qrcode::print_array(int *array, int len)
{
    printf("len: %d\n[", len);
    for (int i = 0; i < len; i++){
        printf("%d%s", array[i], i < len - 1 ? ", " : "]\n");
        if((i+1) % 15 == 0)
            printf("\n");
    }
}