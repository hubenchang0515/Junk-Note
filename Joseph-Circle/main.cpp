#include <iostream>
#include <vector>

using std::cin;
using std::cout;
using std::endl;

/********************************************************
 * 求约瑟夫环问题
 * N 总人数
 * x 每多少个人，丢一个人
 * t 求丢第几个人的文章
 * 返回第times个被丢的人所在的初始位置
 *******************************************************/
int Josephus(int N, int x, int t)
{
    if(t == 1)
    {
        return x - 1;
    }
    else
    {
        return (Josephus(N - 1, x, t - 1) + x) % N;
    }
    
}

int main()
{
    int n;
    while(cin >> n)
    {
        if(n > 999)
        {
            n = 999;
        }
        cout << Josephus(n, 3, n) << endl;
    }
}