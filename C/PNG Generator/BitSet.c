#include <stdarg.h>
#include "BitSet.h"

int init_bit_set(BitSet *bitSet, size_t len)
{
    if (len <= 0)
        return ERROR;
    size_t byteLen = len / 8 + (len % 8 > 0 ? 1 : 0);
    bitSet->len = byteLen;
    bitSet->bits = (byte_t *)calloc(byteLen, sizeof(byte_t));
    return SUCCESS;
}

int set_bit(BitSet *bitSet, size_t position, int val)
{
    if (bitSet == NULL)
    {
        return ERROR;
    }
    if (bitSet->len - 1 < position)
    {
        char *error = calloc(300, sizeof(char));
        sprintf(error, "The position (%ld) is higher than the max position (%ld)", position, bitSet->len - 1);
        perror(error);
        free(error);
        return position;
    }
    size_t bytePos = position / 8;
    int bitPos = position % 8;
    byte_t mask = 1 << bitPos;
    if (val == 0)
        bitSet->bits[bytePos] &= (~mask);
    else if (val == 1)
        bitSet->bits[bytePos] |= mask;
    else
    {
        return ERROR;
        perror("The value is neither 1 nor 0");
    }
    return SUCCESS;
}

int set_bits(BitSet *bitSet, size_t position, byte_t *vals, size_t len)
{
    if (bitSet == NULL)
    {
        return ERROR;
        perror("The BitSet is NULL");
    }
    char *error = calloc(100, sizeof(char));
    if (len <= 0)
    {
        sprintf(error, "The length (%ld), is less than 0", len);
        perror(error);
        free(error);
        return ERROR;
    }
    if (position + len > bitSet->len)
    {
        sprintf(error, "Some of the bits, lie outside of the bit range[0 - %ld], %ld\n", bitSet->len, position + len);
        perror(error);
        free(error);
        return len;
    }
    for (int i = 0; i < len; i++)
    {
        size_t bytePosVal =  i / 8;
        size_t bytePosBitSet = (position + i) / 8;
        int bitPosVal = i % 8;
        int bitPosBitSet = (position + i) % 8;
        int val = (vals[bytePosVal] >> bitPosVal) & 1;
        byte_t mask = 1 << bitPosBitSet;
        if (val == 0)
            bitSet->bits[bytePosBitSet] |= mask;
        else
            bitSet->bits[bytePosBitSet] &= ~mask;
    }
    return SUCCESS;
}