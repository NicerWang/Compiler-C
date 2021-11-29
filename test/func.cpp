int do_it(int n)
{
    return n % 2;
}

int main()
{
    int i = 0;
    printf("%d", do_it(5)); // should be 1
    return 0;
}