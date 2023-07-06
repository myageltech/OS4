#include <iostream>

#include "my_stdlib.h"
#include <unistd.h>
#include <cmath>
#include <sys/wait.h>
#include <unistd.h>

void smalloc_test()
{
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    if (p == NULL)
    {
        std::cout << "smalloc fail" << std::endl;
        exit(2);
    }
    if (*p != 10)
    {
        std::cout << "smalloc fail" << std::endl;
        exit(2);
    }
    std::cout << "so! fucking! long ma bitcheees!!!!" << std::endl;
}

int main(int argc, char const *argv[])
{
    smalloc_test();
    return 0;
}
