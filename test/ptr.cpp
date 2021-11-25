int main()
{
    int a = 0;
    int b = a;
    printf("%d, %d\n", a, b);
    int *pa = &a;
    int *pb = &b;
    int t = *pb;
    *pb = *pa;
    *pa = t;
    printf("%d, %d\n", a, b);
    return 0;
}