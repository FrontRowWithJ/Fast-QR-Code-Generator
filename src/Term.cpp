#include "Term.hpp"

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

void multiply_x(Polynomial &result, Polynomial &p, int x)
{
    for (int i = 0; i < p.length; i++){
        result[i].xExponent = p[i].xExponent + x;
        result[i].alphaExponent = p[i].alphaExponent;
    }
}

void multiply_alpha(Polynomial &result, Polynomial &p, int alpha)
{
    for (int i = 0; i < p.length; i++){
        result[i].xExponent = p[i].xExponent;
        result[i].alphaExponent = (p[i].alphaExponent + alpha) % 255;
    }
}

Polynomial simplify(Polynomial &p, int difference)
{
    Polynomial list(p.length - difference);
    int xExponent = p[0].xExponent;
    int index = 0;
    Term tally = Term();
    tally.xExponent = p[0].xExponent;
    tally.alphaExponent = p[0].alphaExponent;
    for (int i = 1; i < p.length; i++)
        if (p[i].xExponent != xExponent)
        {
            list[index].xExponent = tally.xExponent;
            list[index++].alphaExponent = tally.alphaExponent;
            tally.xExponent = p[i].xExponent;
            tally.alphaExponent = p[i].alphaExponent;
            xExponent = p[i].xExponent;
        }
        else{
            Term tmp = tally + p[i];
            tally.xExponent =  tmp.xExponent;
            tally.alphaExponent = tmp.alphaExponent;
        }
    list[index].xExponent = tally.xExponent;
    list[index].alphaExponent = tally.alphaExponent;
    return list;
}

Polynomial to_polynomial(int *message, int len)
{
    Polynomial polynomial(len / 8);
    for (int i = 0; i < len; i += 8)
    {
        int codeword = 0;
        for (int j = 0; j < 8; j++)
        {
            int bit = message[i + j] % 2;
            codeword |= (bit << j);
        }
        polynomial[(len - i) / 8 - 1] = Term(i / 8, ALPHA_VALUE_EXPONENT[codeword]);
    }
    return polynomial;
}

Polynomial to_polynomial_temp(int *message, int len)
{
    int length = (int)len;
    Polynomial polynomial(len);
    for (int i = 0; i < length; i++)
    {
        polynomial[i].xExponent  = length - i - 1;
        polynomial[i].alphaExponent = ALPHA_VALUE_EXPONENT[message[i]];
    }
    return polynomial;
}

int *to_error_codes(Polynomial &polynomial)
{
    int *message = new int[polynomial.length * 8];
    for (int i = 0; i < polynomial.length; i++)
    {
        int codeword = ALPHA_VALUE[polynomial[i].alphaExponent];
        for (int j = 7; j > -1; j--)
        {
            message[(polynomial.length - i) * 8 - 1 - j] = 2 + (codeword % 2);
            codeword >>= 1;
        }
    }
    return message;
}

void to_array(Polynomial &polynomial, int *&result)
{
    result = new int[polynomial.length];
    for (int i = 0; i < polynomial.length; i++)
        result[i] = ALPHA_VALUE[polynomial[i].alphaExponent];
}

Polynomial gen_polynomial(int errorCodeCount)
{
    Term **matrix = new Term *[errorCodeCount];
    for (int i = 0; i < errorCodeCount; i++)
        matrix[i] = new Term[2];
    for (int i = 0; i < errorCodeCount; i++)
    {
        Term t(1, 0);
        matrix[i][0] = t;
        Term t2(0, i);
        matrix[i][1] = t2;
    }
    Polynomial result = Polynomial(2);
    for (int i = 0; i < 2; i++)
    {
        result[i].xExponent = matrix[0][i].xExponent;
        result[i].alphaExponent = matrix[0][i].alphaExponent;
    }
    for (int i = 1; i < errorCodeCount; i++)
    {
        Polynomial tmp = Polynomial(2);
        for (int j = 0; j < 2; j++)
        {
            tmp[j].xExponent = matrix[i][j].xExponent;
            tmp[j].alphaExponent = matrix[i][j].alphaExponent;
        }
        result = (result * tmp);
        result = simplify(result, i);
    }

    for (int i = 0; i < errorCodeCount; i++)
        delete[] matrix[i];
    delete[] matrix;
    return result;
}

void pretty_print(Polynomial &p)
{
    char *result = new char[p.length * 100]();
    for (int i = 0; i < p.length; i++)
    {
        std::strcat(result, "x");
        std::sprintf(result + std::strlen(result), "%d", p[i].xExponent);
        std::strcat(result, "α");
        std::sprintf(result + std::strlen(result), "%d", p[i].alphaExponent);
        std::strcat(result, i < p.length - 1 ? " + " : "");
    }
    printf("%s\n", result);
    delete[] result;
}

void pretty_print(Term *t, int len){
    Polynomial p = Polynomial(len);
    p.terms = t;
    pretty_print(p);
}
