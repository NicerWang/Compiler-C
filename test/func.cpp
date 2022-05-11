int* do_it(int n,int x)
{
    return n % 2;
}

int main()
{
    int c = 1;
    int d = c;
    int e = do_it(d,2);
    return 0;
}

// int main(){
//     int total = 0;
//     // for(int i = 0; i < 5; i++){
//     //     total = total + i;
//     // }
//     // return 0;
//     return 0;
// }