#include <math.h>

float SinIntegralRect(float A, float B, float e)
{
    float sum = 0.0;
    for (float x = A; x < B; x += e)
    {
        sum += sin(x) * e;
    }
    return sum;
}

float SinIntegralTrap(float A, float B, float e)
{
    float sum = (sin(A) + sin(B)) / 2.0;
    for (float x = A + e; x < B; x += e)
    {
        sum += sin(x);
    }

    return sum * e;
}