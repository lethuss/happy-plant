
#include <iostream>
#include "sys_clock.hpp"

using namespace std;

int main()
{
    Sys_clock cookoo;
    cout << cookoo.get_year() << cookoo.get_second() << endl;
    return 0;
}
