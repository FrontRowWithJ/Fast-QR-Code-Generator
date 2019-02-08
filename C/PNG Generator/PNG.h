#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BitSet.h"
#include "../QR Code Generator/QRCode.h"
#define BUFFER_LENGTH 81920
#define CHUNK_TYPE_LENGTH 4
#define CRC_LENGTH 4
#define SIGNATURE_LENGTH 8
#define SUCCESS 0
#define ERROR -1
#define BASE 65521 // largest prime smaller than 65536
#define PNG_FILE_SIGNATUE \
    (int[SIGNATURE_LENGTH]) { 137, 80, 78, 71, 13, 10, 26, 10 }
#define IHDR_TYPE \
    (int[CHUNK_TYPE_LENGTH]) { 73, 72, 68, 82 }
#define IHDR_DATA_LENGTH 13
#define IEND_TYPE \
    (int[CHUNK_TYPE_LENGTH]) { 73, 69, 78, 68 }
#define IEND_CRC \
    (int[CRC_LENGTH]) { 174, 66, 96, 130 }
#define PLTE_TYPE \
    (int[CHUNK_TYPE_LENGTH]) { 80, 76, 84, 69 }
#define IDAT_TYPE \
    (int[CHUNK_TYPE_LENGTH]) { 73, 68, 65, 84 }
#ifndef BOOL_ENUM
#define BOOL_ENUM
typedef int bool;
enum
{
    false,
    true
};
#endif
bool crc_table_computed = false;
unsigned long crc_table[256] = {};
#ifndef IMAGE_HEADER
#define IMAGE_HEADER
typedef unsigned char byte;
typedef struct ImageHeader
{
    int width;
    int height;
    byte bitDepth;
    byte colorType;
    byte compressionMethod;
    byte filterMethod;
    byte interlaceMethod;
} IHDR;
#endif

#ifndef PALETTE
#define PALETTE
typedef struct PaletteEntry
{
    byte red;
    byte green;
    byte blue;
} Entry;
typedef struct Palette
{
    Entry *entries;
    int numOfEntries;
} PLTE;
#endif
typedef Entry pixel_t;
typedef struct PNGImage
{
    IHDR ihdr;
    PLTE Plte;
    pixel_t **bitmap;
} PNG;

int add_png_signature(FILE *png_file);
IHDR gen_IHDR(int width, int height, byte bitDepth, byte colorType, byte compressionMethod, byte filterMethod, byte interlaceMethod);
FILE *open_file(const char *filename);
int add_ihdr_chunk(FILE *png_file, IHDR ihdr);
void make_crc_table();
unsigned long update_crc(unsigned char *buf, size_t len);
int add_iend_chunk(FILE *png_file);
int add_plte_chunk(FILE *png_file, PLTE *plte);
int add_idat_chunk(FILE *png_file, size_t magLen,byte magQR[magLen][magLen]);
byte *qr_data_to_idat_bit_stream(size_t QRWidth, byte QRData[QRWidth][QRWidth], size_t *bufLen);
void magnify_data(QRCode qr, size_t magWidth, int factor, byte magQRData[magWidth][magWidth]);
byte *qr_data_to_bit_stream(size_t QRWidth, byte QRData[QRWidth][QRWidth], size_t *bufLen);
unsigned long update_adler32(byte_t *buf, int len);
int export_to_png(QRCode qr, int factor, const char *filename);
void free_byte_matrix(byte_t **matrix, int column);
PLTE gen_grayscale_plte();