#include <stdio.h>
#include <math.h>
#include "Term.h"

#define FALSE_READ_ONLY 0
#define TRUE_READ_ONLY 1
#define FALSE 2
#define TRUE 3
#define BYTE_MODE "0100"
#define TERMINATOR "0000" // end of message
#define XOR_MASK 0b101010000010010
#define FORMAT_GX 0b10100110111
#define VERSION_GX 0b1111100100101
#define FORMAT_OFFSET 10
#define PD_WIDTH 7
#define AP_WIDTH 5
#define TIMING_PATTERN 6
#define ERROR_CODES                                                                                              \
    (int[4][40])                                                                                                 \
    {                                                                                                            \
        {7, 10, 15, 20, 26, 36, 40, 48, 60, 72, 80, 96, 104, 120, 132,                                           \
         144, 168, 180, 196, 224, 224, 252, 270, 300, 312, 336, 360, 390, 420, 450, 480, 510, 540, 570,          \
         570, 600, 630, 660, 720, 750},                                                                          \
            {10, 16, 26, 36, 48, 64, 72, 88, 110, 130, 150, 176, 198, 216,                                       \
             240, 280, 308, 338, 364, 416, 442, 476, 504, 560, 588, 644, 700, 728, 784, 812, 868, 924, 980,      \
             1036, 1064, 1120, 1204, 1260, 1316, 1372},                                                          \
            {13, 22, 36, 52, 72, 96, 108, 132, 160, 192, 224, 260, 288,                                          \
             320, 360, 408, 448, 504, 546, 600, 644, 690, 750, 810, 870, 952, 1020, 1050, 1140, 1200, 1290,      \
             1350, 1440, 1530, 1590, 1680, 1770, 1860, 1950, 2040},                                              \
        {                                                                                                        \
            17, 28, 44, 64, 88, 112, 130, 156, 192, 224, 264, 308, 352,                                          \
                384, 432, 480, 532, 588, 650, 700, 750, 816, 900, 960, 1050, 1110, 1200, 1260, 1350, 1440, 1530, \
                1620, 1710, 1800, 1890, 1980, 2100, 2220, 2310, 2430                                             \
        }                                                                                                        \
    }

#define BLOCK_COUNT                                                                                \
    (int[4][40])                                                                                   \
    {                                                                                              \
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 6, 6, 6, 6, 7, 8, 8, 9, 9,                      \
         10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},                      \
            {1, 1, 1, 2, 2, 4, 4, 4, 5, 5, 5, 8, 9, 9, 10, 10, 11, 13, 14, 16,                     \
             17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},      \
            {1, 1, 2, 2, 4, 4, 6, 6, 8, 8, 8, 10, 12, 16, 12, 17, 16, 18, 21, 20,                  \
             23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},      \
        {                                                                                          \
            1, 1, 2, 4, 4, 4, 8, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25,                      \
                25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81 \
        }                                                                                          \
    }
#define P_VALUE                                    \
    (int[3][4])                                    \
    {                                              \
        {3, 2, 1, 1}, {2, 0, 0, 0}, { 1, 0, 0, 0 } \
    }
#define CODEWORD_CAPACITY                                                                                                                                                                                                        \
    (int[4][40])                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                            \
        {19, 34, 55, 80, 108, 136, 156, 194, 232, 274, 324, 370, 428, 461, 523, 589, 647, 721, 795, 861, 932, 1006, 1094, 1174, 1276, 1370, 1468, 1531, 1631, 1735, 1843, 1955, 2071, 2191, 2306, 2434, 2566, 2702, 2812, 2956}, \
            {16, 28, 44, 64, 86, 108, 124, 154, 182, 216, 254, 290, 334, 365, 415, 453, 507, 563, 627, 669, 714, 782, 860, 914, 1000, 1062, 1128, 1193, 1267, 1373, 1455, 1541, 1631, 1725, 1812, 1914, 1992, 2102, 2216, 2334}, \
            {13, 22, 34, 48, 62, 76, 88, 110, 132, 154, 180, 206, 244, 261, 295, 325, 367, 397, 445, 485, 512, 568, 614, 664, 718, 754, 808, 871, 911, 985, 1033, 1115, 1171, 1231, 1286, 1354, 1426, 1502, 1582, 1666},         \
        {                                                                                                                                                                                                                        \
            9, 16, 26, 36, 46, 60, 66, 86, 100, 122, 140, 158, 180, 197, 223, 253, 283, 313, 341, 385, 406, 442, 464, 514, 538, 596, 628, 661, 701, 745, 793, 845, 901, 961, 986, 1054, 1096, 1142, 1222, 1276                   \
        }                                                                                                                                                                                                                        \
    }
#define ECL_L 0
#define ECL_M 1
#define ECL_Q 2
#define ECL_H 3
#define TWO_THREE_SIX "11101100"
#define SEVENTEEN "00010001"
#define REMAINDER_BITS_LENGTH 40
#define REMAINDER_BITS \
    (int[40]) { 0, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0 }
#define I_PLUS_J_MOD_2 0b000
#define I_MOD_2 0b001
#define J_MOD_3 0b010
#define I_PLUS_J_MOD_3 0b011
#define I_DIV_2_PLUS_J_DIV_3_MOD_2 0b100
#define I_J_MOD_2_PLUS_I_J_MOD_3 0b101
#define I_J_MOD_2_PLUS_I_J_MOD_3_MOD_2 0b110
#define I_PLUS_J_MOD_2_PLUS_I_J_MOD_3_MOD_2 0b111
#define MODULE_PATTERN_LENGTH 11
#define MODULE_PATTERN_1                \
    (int[MODULE_PATTERN_LENGTH])        \
    {                                   \
        3, 2, 3, 3, 3, 2, 3, 2, 2, 2, 2 \
    }
#define MODULE_PATTERN_2                \
    (int[MODULE_PATTERN_LENGTH])        \
    {                                   \
        2, 2, 2, 2, 3, 2, 3, 3, 3, 2, 3 \
    }
#define BUFFER_SIZE 32768
#define ERROR -1
#define SUCCESS 0
#ifndef BOOL_ENUM
#define BOOL_ENUM
typedef int bool;
enum
{
    false,
    true
};
#endif
#define POSITION_DETECTOR          \
    (int[7][7])                    \
    {                              \
        {1, 1, 1, 1, 1, 1, 1},     \
            {1, 0, 0, 0, 0, 0, 1}, \
            {1, 0, 1, 1, 1, 0, 1}, \
            {1, 0, 1, 1, 1, 0, 1}, \
            {1, 0, 1, 1, 1, 0, 1}, \
            {1, 0, 0, 0, 0, 0, 1}, \
            {1, 1, 1, 1, 1, 1, 1}, \
    }
#define ALIGNMENT_PATTERN    \
    (int[5][5])              \
    {                        \
        {1, 1, 1, 1, 1},     \
            {1, 0, 0, 0, 1}, \
            {1, 0, 1, 0, 1}, \
            {1, 0, 0, 0, 1}, \
            {1, 1, 1, 1, 1}, \
    }
typedef struct QR
{
    int QRVersion;
    size_t QRWidth;
    int ECL;
    int formatData[15];
    int versionData[18];
    int **QRData;
} QRCode;

QRCode init(int ECL, char *message, bool setToDark);
void gen_format_data(int ECL, int mpr, int formatData[], size_t len);
int highest_one_bit_position_of(int number);
int to_int(int array[], size_t len);
void gen_position_detectors(void);
void add_position_detectors(size_t QRWidth, int **QRData);
void add_remainder_bits_array(void);
void add_format_data(int **QRData, size_t QRWidth, int QRVersion, int formatData[15]);
void gen_version_data(int QRVersion, int versionData[18]);
void add_version_data(int versionData[18], size_t QRWidth, int **QRData);
void copy_PD_to_qr_code(int offsetI, int offsetJ, int **QRData);
void copy_AP_to_qr_code(int offsetI, int offsetJ, int **QRCata);
void add_timing_patterns();
int *gen_alignment_coords(int version, int *coords, int QRVersion);
void add_alignment_pattern(int QRVersion, int **QRData);
int *byte_mode(char *inputData, int QRVersion, size_t *outputLen, int ECL);
char *int_to_binary_string(int character, size_t len);
void add_code_words(int *final_message, size_t final_message_len, int **QRData, size_t QRWidth);
int num_of_code_words(int QRVersion);
int concat_bits_to_bytes(int *message, int *output, size_t message_len);
int get_qr_version(char *message, int ECL);
char *add_pad_bytes(char *messgage, int ECL, int QRVersion);
char *repeat_string(const char *string, int numberOfTimes);
int *gen_block_stucture(size_t len, int QRVersion, int ECL, size_t *blockStructureLen);
int *gen_final_message(int *message, size_t len, int QRVersion, int ECL, size_t *finalMessageLen);
int *gen_error_block(int **messageBlock, size_t messageBlockHeight, size_t *messageBlockRowLens, int numOFErrorCodewords);
Term *gen_error_polynomial(int *message, size_t messageLen, int numOfErrorCodewords, size_t *errorPolyLen);
int *to_bit_array(int *message, size_t messageLen);
int reverse(int *array, size_t arrayLen);
int *add_remaining_bits(int *finalMessage, size_t finalMessageLen, int QRVersion, size_t *len);
void add_seperators(int **QRData, size_t QRWidth);
void add_mask(int **QRData, size_t QRWidth, int ECL, int formatData[15], int QRVersion);
int **copy_matrix(int **src, int **dst, size_t QRWidth);
void set_to_dark(int **QRData, size_t QRWidth);
int get_matrix_penalty(int **QRData, size_t QRWidth);
int **init_matrix(size_t row, size_t column);
void free_matrix(int **matrix, size_t row);
int evaluate_consecutive_modules_penalty(int **QRData, size_t QRWidth);
int evaluate_2_by_2_module_penalty(int **QRData, size_t QRWidth);
int evaluate_pattern_penalty(int **QRData, size_t QRWidth);
int evaluate_ratio_penalty(int **QRData, size_t QRWidth);
int **add_white_border(QRCode qr);
void print_array(int *list, size_t len);
void print_qr(int **QRData, size_t QRWidth);