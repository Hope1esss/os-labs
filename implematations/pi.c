float PiLeibniz(int K)
{
    float pi = 0.0;
    for (int n = 0; n < K; n++)
    {
        pi += (n % 2 == 0 ? 1.0 : -1.0) / (2 * n + 1);
    }

    return 4 * pi;
}

float PiWallis(int K)
{
    float pi = 1.0;
    for (int n = 1; n <= K; n++)
    {
        pi *= (4.0 * n * n) / (4.0 * n * n - 1.0);
    }
    return 2 * pi;
}