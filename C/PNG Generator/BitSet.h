#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define SUCCESS 0
#define ERROR -1
typedef unsigned char byte_t;
typedef struct BIT_SET {
    byte_t *bits;
    size_t len;
} BitSet;

int init_bit_set(BitSet *bitSet, size_t len);
int set_bit(BitSet *bitSet, size_t position, int val);
int set_bits(BitSet *bitSet, size_t position, byte_t *vals, size_t len);
