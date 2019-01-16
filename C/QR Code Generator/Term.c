#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "Term.h"

void populate_alpha_arrays()
{
    ALPHA_VALUE_EXPONENT = (int *)calloc(257, sizeof(int));
    ALPHA_VALUE = (int *)calloc(257, sizeof(int));
    for (int i = 0; i < 255; i++)
    {
        int alpha = get_alpha(i);
        ALPHA_VALUE_EXPONENT[alpha] = i;
        ALPHA_VALUE[i] = alpha;
    }
    ALPHA_VALUE[255] = 1;
    ALPHA_VALUE_EXPONENT[0] = 256;
}

Term init_term(int xExponent, int alphaExponent)
{
    Term t;
    t.xExponent = xExponent;
    t.alphaExponent = alphaExponent;
    return t;
}

int get_alpha(int exponent)
{
    int result = 1;
    for (int i = 0; i < exponent; i++)
    {
        result *= 2;
        if (result > 255)
            result ^= 285;
    }
    return result;
}

Term multiply_terms(Term a, Term b)
{
    Term result;
    result.xExponent = a.xExponent + b.xExponent;
    result.alphaExponent = (a.alphaExponent + b.alphaExponent) % 255;
    return result;
}

Term *multiply_polynomials(Term *poly1, Term *poly2, size_t poly1Length, size_t poly2Length)
{
    Term *result = (Term *)malloc(poly1Length * poly2Length * sizeof(Term));
    for (int i = 0; i < poly1Length; i++)
        for (int j = 0; j < poly2Length; j++)
            result[i * poly2Length + j] = multiply_terms(poly1[i], poly2[j]);
    return result;
}

Term *multiply_x(Term *polynomial, size_t len, int x)
{
    Term *result = (Term *)malloc(len * sizeof(Term));
    for (int i = 0; i < len; i++)
        result[i] = init_term(polynomial[i].xExponent + x, polynomial[i].alphaExponent);
    return result;
}

Term *multiply_alpha(Term *polynomial, size_t len, int alpha)
{
    Term *result = (Term *)malloc(len * sizeof(Term));
    for (int i = 0; i < len; i++)
        result[i] = init_term(polynomial[i].xExponent, (polynomial[i].alphaExponent + alpha) % 255);
    return result;
}

Term * xor (Term * poly1, Term *poly2, size_t poly1Length, size_t poly2Length, size_t *resultLen) {
    int limit = min(poly1Length, poly2Length);
    int max = max(poly1Length, poly2Length);
    Term *result = (Term *)malloc((max - 1) * sizeof(Term));
    *resultLen = max - 1;
    int i = 0;
    for (; i < limit - 1; i++)
        result[i] = init_term(poly1[i + 1].xExponent, ALPHA_VALUE_EXPONENT[ALPHA_VALUE[poly1[i + 1].alphaExponent] ^ ALPHA_VALUE[poly2[i + 1].alphaExponent]]);
    if (poly1Length > poly2Length)
        for (; i < max - 1; i++)
            result[i] = init_term(poly1[i + 1].xExponent, poly1[i + 1].alphaExponent);
    else
        for (; i < max - 1; i++)
            result[i] = init_term(poly2[i + 1].xExponent, poly2[i + 1].alphaExponent);
    return result;
}

    Term add(Term t1, Term t2)
{
    int alpha1 = ALPHA_VALUE[t1.alphaExponent];
    int alpha2 = ALPHA_VALUE[t2.alphaExponent];
    Term result;
    result.xExponent = t1.xExponent;
    result.alphaExponent = ALPHA_VALUE_EXPONENT[alpha1 ^ alpha2];
    return result;
}

Term *simplify(Term *polynomial, int difference, size_t len)
{
    Term *list = (Term *)malloc((len - difference) * sizeof(Term));
    int xExponent = polynomial[0].xExponent;
    int index = 0;
    Term tally = polynomial[0];
    for (int i = 1; i < len; i++)
        if (polynomial[i].xExponent != xExponent)
        {
            list[index++] = tally;
            tally = polynomial[i];
            xExponent = polynomial[i].xExponent;
        }
        else
            tally = add(tally, polynomial[i]);
    list[index] = tally;
    return list;
}

Term *to_polynomial(int *message, size_t len)
{
    Term *polynomial = (Term *)malloc(len * sizeof(Term) / 8);
    for (int i = 0; i < len; i += 8)
    {
        int codeword = 0;
        for (int j = 0; j < 8; j++)
        {
            int bit = message[i + j] % 2;
            codeword |= (bit << j);
        }
        polynomial[(len - i) / 8 - 1] = init_term(i / 8, ALPHA_VALUE_EXPONENT[codeword]);
    }
    return polynomial;
}

Term *to_polynomial_temp(int *message, size_t len)
{
    Term *polynomial = (Term *)malloc(len * sizeof(Term));
    for (int i = 0; i < len; i++)
    {
        Term t;
        t.xExponent = len - i - 1;
        t.alphaExponent = ALPHA_VALUE_EXPONENT[message[i]];
        polynomial[i] = t;
    }
    return polynomial;
}

int *to_error_codes(Term *polynomial, size_t len)
{
    int *message = (int *)malloc(len * sizeof(int) * 8);
    for (int i = 0; i < len; i++)
    {
        int codeword = ALPHA_VALUE[polynomial[i].alphaExponent];
        for (int j = 7; j > -1; j--)
        {
            message[(len - i) * 8 - 1 - j] = 2 + (codeword % 2);
            codeword >>= 1;
        }
    }
    return message;
}

int *to_array(Term *polynomial, size_t len)
{
    int *result = (int *)malloc(len * sizeof(int));
    for (int i = 0; i < len; i++)
        result[i] = ALPHA_VALUE[polynomial[i].alphaExponent];
    return result;
}

Term *gen_polynomial(int errorCodeCount, size_t *result_len)
{
    // index 0 == exponent of x
    // index 1 == α
    Term **terms = init_term_marix(errorCodeCount);
    for (int i = 0; i < errorCodeCount; i++)
    {
        Term t1;
        t1.xExponent = 1;
        t1.alphaExponent = 0;
        terms[i][0] = t1;
        Term t2;
        t2.xExponent = 0;
        t2.alphaExponent = i;
        terms[i][1] = t2;
    }
    Term *result = terms[0];
    *result_len = 2;
    for (int i = 1; i < errorCodeCount; i++)
    {
        result = multiply_polynomials(result, terms[i], *result_len, 2);
        *result_len *= 2;
        result = simplify(result, i, *result_len);
        *result_len -= i;
    }
    return result;
}

Term **init_term_marix(int errorCodeCount)
{
    Term **t;
    t = (Term **)malloc(errorCodeCount * sizeof(Term *));
    for (int i = 0; i < errorCodeCount; i++)
        t[i] = (Term *)malloc(2 * sizeof(Term));
    return t;
}

void free_term_matrix(Term **term, size_t len)
{
    for (int i = 0; i < len; i++)
        free(term[i]);
    free(term);
}

char *to_super_script(int n)
{
    char *result = (char *)calloc(100, sizeof(char));
    int number = 0;
    do
    {
        number *= 10;
        number += (n % 10);
        n /= 10;
    } while (n != 0);
    int i = 0;
    do
    {
        result[i++] = SUPERSCRIPT[number % 10];
        number /= 10;
    } while (number != 0);
    return result;
}

char *pretty_print(Term *polynomial, size_t len)
{
    char *result = (char *)calloc(len * 100, sizeof(result));
    for (int i = 0; i < len; i++)
    {
        strcat(result, "x");
        char *buffer0 = (char *)calloc(100, sizeof(char));
        sprintf(buffer0, "%d", polynomial[i].xExponent);
        strcat(result, buffer0);
        strcat(result, "α");
        char *buffer1 = (char *)calloc(100, sizeof(char));
        sprintf(buffer1, "%d", polynomial[i].alphaExponent);
        strcat(result, buffer1);
        strcat(result, i < len - 1 ? " + " : "");
        free(buffer0);
        free(buffer1);
    }
    return result;
}