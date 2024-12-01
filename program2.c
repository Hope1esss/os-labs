#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

int main()
{
    char input[256];
    void *lib_integrals, *lib_pi;
    float (*Pi)(int);
    float (*Integral)(float, float, float);

    lib_integrals = dlopen("libintegrals.so", RTLD_LAZY);
    if (!lib_integrals)
    {
        printf("Error opening libintegrals.so: %s\n", dlerror());
        return 1;
    }

    lib_pi = dlopen("libpi.so", RTLD_LAZY);
    if (!lib_pi)
    {
        printf("Error opening libpi.so: %s\n", dlerror());
        return 1;
    }

    Pi = dlsym(lib_pi, "PiLeibniz");

    Integral = dlsym(lib_integrals, "SinIntegralRect");
    if (!Pi || !Integral)
    {
        printf("Error loading functions: %s\n", dlerror());
        dlclose(lib_integrals);
        dlclose(lib_pi);
        return 1;
    }

    printf("Используйте '1' для расчёта Pi (формат <1 K>), '2' для расчёта интеграла (формат <2 A B e>), '0' для переключения реализаций контрактов, '3' для вывода этой справки, '4' для выхода из программы\n");

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

            float result = Pi(K);
            printf("Результат (Pi) = %.6f\n", result);
        }
        else if (command == 2)
        {
            float A, B, e;
            if (sscanf(input + 2, "%f %f %f", &A, &B, &e) != 3)
            {
                printf("Ошибка. Для этой команды ожидается три аргумента A, B, e\n");
                continue;
            }

            float result = Integral(A, B, e);
            printf("Результат (Integral) = %.6f\n", result);
        }
        else if (command == 0)
        {
            if (Integral == dlsym(lib_integrals, "SinIntegralRect"))
            {
                Integral = dlsym(lib_integrals, "SinIntegralTrap");
                printf("Реализация вычисления интеграла изменена: Метод Прямоугольников -> Метод Трапеций\n");
            }
            else
            {
                Integral = dlsym(lib_integrals, "SinIntegralRect");
                printf("Реализация вычисления интеграла изменена: Метод Трапеций -> Метод Прямоугольников\n");
            }

            if (Pi == dlsym(lib_pi, "PiLeibniz"))
            {
                Pi = dlsym(lib_pi, "PiWallis");
                printf("Реализация вычисления Pi изменена: Ряд Лейбница -> Формула Валлиса\n");
            }
            else
            {
                Pi = dlsym(lib_pi, "PiLeibniz");
                printf("Реализация вычисления Pi изменена: Формула Валлиса -> Ряд Лейбница\n");
            }
        }
        else if (command == 3)
        {
            printf("Используйте '1' для расчёта Pi (формат <1 K>), '2' для расчёта интеграла (формат <2 A B e>), '0' для переключения реализаций контрактов, '3' для вывода этой справки '4' для выхода из программы\n");
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

    dlclose(lib_integrals);
    dlclose(lib_pi);
    return 0;
}