int do_it(int n)
{
    return n % 2;
}

int main()
{
    int d = 5;
    int c = do_it(d);
    printf("%d", do_it(5)); // should be 1
    d = d + 1;
    return 0;
}