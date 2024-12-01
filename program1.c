#include <stdio.h>
#include <string.h>
#include "include/integrals.h"
#include "include/pi.h"

int main()
{
    char input[256];
    printf("Используйте '1' для расчёта Pi (формат <1 K>), '2' для расчёта интеграла (формат <2 A B e>), '3' для вывода этой справки, '4' для выхода из программы\n");
    while (fgets(input, sizeof(input), stdin))
    {
        input[strcspn(input, "\n")] = '\0';

        int command;
        if (sscanf(input, "%d", &command) != 1)
        {
            printf("Неверная команда. Попробуйте ещё раз\n");
            continue;
        }

        if (command == 1)
        {
            int K;
            if (sscanf(input + 2, "%d", &K) != 1)
            {
                printf("Ошибка. Для этой команды ожидается один аргумент K\n");
                continue;
            }

            float result = PiLeibniz(K);
            printf("Результат (PiLeibniz) = %.6f\n", result);
        }
        else if (command == 2)
        {
            float A, B, e;
            if (sscanf(input + 2, "%f %f %f", &A, &B, &e) != 3)
            {
                printf("Ошибка. Для этой команды ожидается три аргумента A, B и e\n");
                continue;
            }

            float result = SinIntegralRect(A, B, e);
            printf("Результат (SinIntegralRect) = %.6f\n", result);
        }
        else if (command == 3)
        {
            printf("Используйте '1' для расчёта Pi (формат <1 K>), '2' для расчёта интеграла (формат <2 A B e>), '3' для вывода этой справки, '4' для выхода из программы\n");
        }
        else if (command == 4)
        {
            break;
        }

        else
        {
            printf("Неверная команда. Попробуйте ещё раз\n");
        }
        printf("Введите следующую команду\n");
    }

    return 0;
}