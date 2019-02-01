#include "PNG.h"

int main(int argc, char **argv)
{
    QRCode qr = init(0, "HELLO WORLD", false);
    export_to_png(qr, 1, "qrImage.png");
}

int add_png_signature(FILE *png_file)
{
    if (png_file == NULL)
        return ERROR;
    for (int i = 0; i < SIGNATURE_LENGTH; i++)
        fputc(PNG_FILE_SIGNATUE[i], png_file);
    return SUCCESS;
}

IHDR gen_IHDR(int width, int height, byte bitDepth, byte colorType, byte compressionMethod, byte filterMethod, byte interlaceMethod)
{ // error handling for incorrect inputs?
    return (IHDR){width, height, bitDepth, colorType, compressionMethod, filterMethod, interlaceMethod};
}

FILE *open_file(const char *filename)
{
    bool foundName = false;
    int index = 0;
    char *nameOfFile = (char *)calloc(BUFFER_LENGTH, sizeof(char));
    do
    {
        if (index != 0)
            sprintf(nameOfFile, "(%d)", index);
        strcat(nameOfFile, filename);
        if (fopen(nameOfFile, "r") != NULL)
        {
            memset(nameOfFile, 0, BUFFER_LENGTH);
            index++;
        }
        else
            foundName = true;
    } while (!foundName);
    return fopen(nameOfFile, "wb");
}

int add_ihdr_chunk(FILE *png_file, IHDR ihdr)
{
    if (png_file == NULL)
        return ERROR;
    int index = 0;
    byte *buf = (byte *)calloc(CHUNK_TYPE_LENGTH + IHDR_DATA_LENGTH, sizeof(byte));
    for (int i = sizeof(int) - 1; i > -1; i--)
        fputc((IHDR_DATA_LENGTH >> (i * 8)) & 0xFF, png_file);
    for (int i = 0; i < CHUNK_TYPE_LENGTH; i++)
        buf[index++] = IHDR_TYPE[i];
    for (int i = sizeof(int) - 1; i > -1; i--)
        buf[index++] = (ihdr.width >> (i * 8)) & 0xFF;
    for (int i = sizeof(int) - 1; i > -1; i--)
        buf[index++] = (ihdr.height >> (i * 8)) & 0xFF;
    buf[index++] = ihdr.bitDepth;
    buf[index++] = ihdr.colorType;
    buf[index++] = ihdr.compressionMethod;
    buf[index++] = ihdr.filterMethod;
    buf[index++] = ihdr.interlaceMethod;
    unsigned long crc = update_crc(buf, index);
    for (int i = sizeof(int) - 1; i > -1; i--)
        buf[index++] = (crc >> (i * 8)) & 0xFF;
    for (int i = 0; i < index; i++)
        fputc(buf[i], png_file);
    free(buf);
    return SUCCESS;
}

void make_crc_table()
{
    unsigned long c;
    int n, k;
    for (n = 0; n < 256; n++)
    {
        c = (unsigned long)n;
        for (k = 0; k < 8; k++)
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c >>= 1;
        crc_table[n] = c;
    }
    crc_table_computed = true;
}

unsigned long update_crc(unsigned char *buf, size_t len)
{
    unsigned long c = 0xFFFFFFFFL;
    int n;
    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++)
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    return c ^ 0xFFFFFFFFL;
}

int add_iend_chunk(FILE *png_file)
{
    if (png_file == NULL)
        return ERROR;
    for (int i = 0; i < 4; i++) //The IEND data length is 0 so 4 0 bytes
        fputc('\0', png_file);  //are written
    for (int i = 0; i < CHUNK_TYPE_LENGTH; i++)
        fputc(IEND_TYPE[i], png_file);
    for (int i = 0; i < CRC_LENGTH; i++)
        fputc(IEND_CRC[i], png_file);
    return SUCCESS;
}

PLTE gen_grayscale_plte()
{
    PLTE plte;
    plte.entries = (Entry *)malloc(sizeof(Entry) * 2);
    plte.entries[1] = (Entry){255, 255, 255};
    plte.entries[0] = (Entry){0, 0, 0};
    plte.numOfEntries = 2;
    return plte;
}

int add_plte_chunk(FILE *png_file, PLTE *plte)
{
    if (png_file == NULL)
        return ERROR;
    size_t bufLen = 0;
    byte *buf = (byte *)calloc(BUFFER_LENGTH, sizeof(byte));
    int plteDataLen = plte->numOfEntries * sizeof(Entry);
    for (int i = sizeof(int) - 1; i > -1; i--)
        fputc((plteDataLen >> (i * 8)) & 0xFF, png_file);
    for (int i = 0; i < CHUNK_TYPE_LENGTH; i++)
        buf[bufLen++] = PLTE_TYPE[i];
    for (int i = 0; i < plte->numOfEntries; i++)
    {
        Entry entry = plte->entries[i];
        buf[bufLen++] = entry.red;
        buf[bufLen++] = entry.green;
        buf[bufLen++] = entry.blue;
    }
    unsigned long crc = update_crc(buf, bufLen);
    for (int i = sizeof(int) - 1; i > -1; i--)
        buf[bufLen++] = (crc >> (i * 8)) & 0xFF;
    for (int i = 0; i < bufLen; i++)
        fputc(buf[i], png_file);
    free(buf);
    free(plte->entries);
    return SUCCESS;
}

int qr_data_to_idat_bit_stream(byte_t **QRData, size_t QRWidth, BitSet *bs, size_t *byteLen)
{
    int index = 0;
    bs->len = QRWidth * QRWidth + 88 - ((QRWidth * QRWidth) % 8); // default compression bytes  16
                                                                  // nlen                       16
                                                                  // len                        16
                                                                  // adler-32 checksum          32
                                                                  // header bits               + 8
                                                                  //                            __
                                                                  //                            88 bits

    *byteLen = (bs->len / 8 + (bs->len % 8 == 0 ? 0 : 1));
    bs->bits = (byte_t *)calloc(*byteLen, sizeof(byte_t));
    bs->bits[0] = (byte)0x78;
    bs->bits[1] = (byte)0x9C;
    bs->bits[2] = (byte)0x01;
    int len = QRWidth * QRWidth / 8;
    int nlen = ~len;
    bs->bits[3] = (byte)(len >> 8);
    bs->bits[4] = (byte)(len & 0xFF);
    bs->bits[5] = (byte)(nlen >> 8);
    bs->bits[6] = (byte)(nlen & 0xFF);
    index = 56;
    // for (int i = 0; i < sizeof(short) * 8; i++)
    //     if (set_bit(bs, index++, (len >> i) & 1))
    //         return ERROR;
    // for (int i = 0; i < sizeof(short) * 8; i++)
    //     if (set_bit(bs, index++, (nlen >> i) & 1))
    //         return ERROR;
    for (int i = 0; i < len * 8; i++)
        if (set_bit(bs, index++, QRData[i / QRWidth][i % QRWidth]))
            return ERROR;
    BitSet qrBs;
    int result = qr_data_to_bit_stream(QRData, QRWidth, &qrBs);
    if (result != SUCCESS)
        return result;
    unsigned long adler = update_adler32(qrBs.bits, qrBs.len / 8 + (qrBs.len % 8 == 0 ? 0 : 1));
    for (int i = 0; i < sizeof(int) * 8; i++)
        if (set_bit(bs, index++, (adler >> i) & 1))
            return ERROR;
    return SUCCESS;
}

int add_idat_chunk(FILE *png_file, byte **magqrData, size_t magLen)
{
    if (png_file == NULL)
        return ERROR;
    int byteCount = 0;
    BitSet bs;
    size_t byteLen;
    if (qr_data_to_idat_bit_stream(magqrData, magLen, &bs, &byteLen))
        return ERROR;
    for (int i = sizeof(int) - 1; i > -1; i--)
        fputc((byteLen >> 8 * i) & 0xFF, png_file);
    for (int i = 0; i < CHUNK_TYPE_LENGTH; i++)
        fputc(IDAT_TYPE[i], png_file);
    for (int i = 0; i < byteLen; i++)
        fputc(bs.bits[i], png_file);
    byte crcBuf[CHUNK_TYPE_LENGTH + byteLen];
    for (int i = 0; i < CHUNK_TYPE_LENGTH; i++)
        crcBuf[i] = IDAT_TYPE[i];
    for (int i = 0; i < byteLen; i++)
        crcBuf[i + CHUNK_TYPE_LENGTH] = bs.bits[i];
    unsigned long crc = update_crc(crcBuf, CHUNK_TYPE_LENGTH + byteLen);
    for (int i = sizeof(int) - 1; i > -1; i--)
        fputc((crc >> i * 8) & 0xFF, png_file);
    return SUCCESS;
}

byte_t **magnify_data(QRCode qr, int factor, size_t *newLen)
{
    if (factor <= 0)
        return NULL;
    if (factor > 255)
        return NULL;
    *newLen = qr.QRWidth * factor;
    byte_t **magnifiedData = (byte **)malloc(*newLen * sizeof(byte));
    for (int i = 0; i < *newLen; i++)
        magnifiedData[i] = (byte *)malloc(*newLen * sizeof(byte));
    for (int i = 0; i < qr.QRWidth; i++)
        for (int j = 0; j < qr.QRWidth; j++)
            for (int fi = 0; fi < factor; fi++)
                for (int fj = 0; fj < factor; fj++)
                    magnifiedData[i * factor + fi][j * factor + fj] = qr.QRData[i][j] % 2;
    return magnifiedData;
}

int qr_data_to_bit_stream(byte_t **QRData, size_t QRWidth, BitSet *bs)
{
    bs->len = QRWidth * QRWidth - ((QRWidth * QRWidth) % 8);
    bs->bits = (byte_t *)calloc(bs->len / 8 + (bs->len % 8 == 0 ? 0 : 1), sizeof(byte_t));
    int i = 0;
    byte_t codeword = 0;
    for (; i < bs->len; i += 8)
    {
        for (int j = 0; j < 8 && (i + j) < bs->len; j++)
            codeword += (QRData[(i + j) / QRWidth][(i + j) % QRWidth] & 1) << j;
        bs->bits[i / 8] = codeword;
        codeword = 0;
    }
    return SUCCESS;
}

unsigned long update_adler32(byte_t *buf, int len)
{
    unsigned long adler = 1L;
    unsigned long s1 = adler & 0xFFFF;
    unsigned long s2 = (adler >> 16) & 0xFFFF;
    for (int n = 0; n < len; n++)
    {
        s1 = (s1 + buf[n]) % BASE;
        s2 = (s2 + s1) % BASE;
    }
    return (s2 << 16) + s1;
}

int export_to_png(QRCode qr, int factor, const char *filename)
{
    if (factor <= 0)
        return ERROR;
    if (strlen(filename) == 0)
        return ERROR;
    char *png_name = calloc(BUFFER_LENGTH, sizeof(byte_t));
    if (strlen(filename) < sizeof(".png"))
    {
        strcpy(png_name, filename);
        strcat(png_name, ".png");
    }
    else
    {
        char *str = calloc(BUFFER_LENGTH, sizeof(char));
        for (int i = strlen(filename) - 4; i < strlen(filename); i++)
        {
            char c[2] = {(char)filename[i], '\0'};
            strcat(str, c);
        }
        if (strcmp(str, ".png") == 0)
            strcpy(png_name, filename);
        else
        {
            strcpy(png_name, filename);
            strcat(png_name, ".png");
        }
        free(str);
    }
    size_t magLen;
    byte_t **magQR = magnify_data(qr, factor, &magLen);
    FILE *qr_img = fopen(png_name, "wb");
    add_png_signature(qr_img);
    IHDR ihdr = gen_IHDR(magLen, magLen, 1, 3, 0, 0, 0);
    add_ihdr_chunk(qr_img, ihdr);
    PLTE grayscale = gen_grayscale_plte();
    add_plte_chunk(qr_img, &grayscale);
    add_idat_chunk(qr_img, magQR, magLen);
    add_iend_chunk(qr_img);
    // free_byte_matrix(magQR);
    fclose(qr_img);
    qr_img = NULL;
    free(png_name);
    return SUCCESS;
}

void free_byte_matrix(byte_t **matrix, int column)
{
    for (int i = 0; i < column; i++)
        free(matrix[i]);
    free(matrix);
}