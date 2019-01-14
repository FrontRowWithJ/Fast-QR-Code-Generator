#include <stdlib.h>
#include <stdio.h>
#define ALPHA_VALUE_EXPONENT \
    (int[257]) { 0 }
#define ALPHA_VALUE \
    (int[257]) { 0 }

#ifndef TERM_H
#define TERM_H
typedef struct term
{
    int xExponent;
    int alphaExponent;
} Term;
#endif

void populate_alpha_arrays();
int get_alpha(int number);
Term init_term(int xExponent, int alphaExponent);
Term multiply_terms(Term a, Term b);
Term *multiply_polynomials(Term *poly1, Term *poly2, size_t poly1Length, size_t poly2Length);
Term *multiply_x(Term *polynomial, size_t len, int x);
Term clone(Term term);
Term *multiply_alpha(Term *polynomial, size_t len, int alpha);
Term * xor (Term * poly1, Term *poly2, size_t poly1Length, size_t poly2Length, size_t *resultLen);
Term add(Term t1, Term t2);
Term *simplify(Term *polynomial, int difference, size_t len);
Term *to_polynomial(int *message, size_t len);
Term *to_polynomial_temp(int *message, size_t len);
int *to_error_codes(Term *polynomial, size_t len);
int *to_array(Term *polynomial, size_t len);
Term *gen_polynomial(int errorCodeCount, size_t *result_len);
Term **init_term_marix(int errorCodeCount);
void free_term_matrix(Term **term, size_t len);
