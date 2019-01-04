#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QRConstants.h"
#include <math.h>

void genFormatData(int ECL, int mpr, int formatData[], size_t len);
int highestOneBit(int number);
int toInt(int array[], size_t len);

int main(void)
{
    int formatData[15] = {0};
    genFormatData(0, 0, formatData, sizeof(formatData) / sizeof(int));
    printf("%d\n", toInt(formatData, sizeof(formatData) / sizeof(int)));
}

void genFormatData(int ECL, int mpr, int formatData[], size_t len)
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
        int gx = fgx << (highestOneBit(result) - fo);
        result ^= gx;
    } while (highestOneBit(result) >= fo);
    result |= (data << (fo));
    result ^= XOR_MASK;
    printf("%d\n", result);
    for (int i = 0; i < len; i++)
        formatData[i] = (result >> i) & 1;
}

int toInt(int array[], size_t len)
{
    int result = 0;
    for (int i = 0; i < len; i++){
        result |= array[i] << i;
    }
    return result;
}

int highestOneBit(int n)
{
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    return n - (n >> 1) - 1;
}