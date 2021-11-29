int printf()
{
    int n = 5;
    return n;
}
int scanf()
{
    return 0;
}

int main()
{
    int m;
    int kdwada = 1;

    int n;
    m = 2;
    // handle IO
    // 1 \n
    scanf("%d", &m);
    n = m;
    printf("%d\n", (n + 100) % 2);
    m++;
    int i = 0;
    int factorial = 3;
    int result = 1;
    while (factorial > 0)
    { // factorial
        result = result * factorial;
        factorial = factorial - 1;
    }
    printf("%d\n", result % 100 - 1);
    i++;

    for (int j = 0; result % 100 - 1; j++)
    {
        for (int k = j + 1; k < result % 100 - 1; ++k)
        {
            /*
            printf("Hello ~: %d", j);
        */
            if (k - 5 == j || !j + 2 == k && k - 3 == j)
            {
                int i = 0;
                i = k % (j + 1);
                printf("1: %d\n", i);
            }
            else if (k == j + 3 && k % 2 == 0)
            {
                i = k / (j + 1);
                printf("2: %d\n", i);
            }
            else
            {
                printf("3: %d\n", i);
                /*
            printf("Aha~ %d", j);
            */
            }
        }
    }
    return 0;
}

// /*

// <your input>
// <correspnding output>
// 6
// 3: 0
// 3: 0
// 1: 0
// 3: 0
// 3: 0
// 3: 0
// 2: 2
// 3: 2
// 3: 2
// 3: 2

// */
