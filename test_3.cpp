#include <iostream>

#include "my_stdlib.h"
#include <unistd.h>
#include <cmath>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    // test smalloc
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Smalloc Test" << std::endl;
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    if (p == NULL)
    {
        std::cout << "Smalloc Test Failed!" << std::endl;
        std::cout << "got NULL" << std::endl;
    }
    else if (*p != 10)
    {
        std::cout << "Smalloc Test Failed!" << std::endl;
        std::cout << "Expected: 10" << std::endl;
        std::cout << "Got: " << *p << std::endl;
    }
    else{
        
        std::cout << "Smalloc Test Passed!" << std::endl;
    }
    std::cout << "|-----------------------------------|" << std::endl;

    // test sfree
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Sfree Test" << std::endl;
    sfree(p);
    if (p != NULL)
    {
        std::cout << "Sfree Test Failed!" << std::endl;
        std::cout << "Expected: NULL" << std::endl;
        std::cout << "Got: " << p << std::endl;
    }
    else{

    std::cout << "Sfree Test Passed!" << std::endl;
    }
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "the size should be" << sizeof(int) << std::endl;
    std::cout << "the order should be" << 0 << std::endl;
    std::cout << "is free shoud be" << "false" << std::endl;
    
    // // test cookies for smalloc if they are not overwritten
    // std::cout << "|-----------------------------------|" << std::endl;
    // std::cout << "Smalloc Test for cookies" << std::endl;
    // int *a = (int *)smalloc(sizeof(int));
    // if (a == NULL)
    // {
    //     std::cout << "Smalloc Test Failed!" << std::endl;
    //     std::cout << "Got NULL" << std::endl;
    // }
    // else if (*a != 0xAAAAAAAA)
    // {
    //     std::cout << "Smalloc Test Failed!" << std::endl;
    //     std::cout << "Expected: 0xAAAAAAAA" << std::endl;
    //     std::cout << "Got: " << *a << std::endl;
    // }
    // else{
    // std::cout << "Smalloc Test Passed!" << std::endl;
    // }
    std::cout << "|-----------------------------------|" << std::endl;
    // test scalloc
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Scalloc Test" << std::endl;
    int *q = (int *)scalloc(5, sizeof(int));
    if (q == NULL)
    {
        std::cout << "Scalloc Test Failed!" << std::endl;
        std::cout << "Got NULL" << std::endl;
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (q[i] != 0)
            {
                std::cout << "Scalloc Test Failed!" << std::endl;
                std::cout << "Expected: 0" << std::endl;
                std::cout << "Got: " << q[i] << std::endl;
                break;
            }
        }
    }
    std::cout << "Scalloc Test Passed!" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    // test srealloc
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Srealloc Test" << std::endl;
    int *r = (int *)srealloc(q, 10 * sizeof(int));
    if (r == NULL)
    {
        std::cout << "Srealloc Test Failed!" << std::endl;
        std::cout << "Got NULL" << std::endl;
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (r[i] != 0)
            {
                std::cout << "Srealloc Test Failed!" << std::endl;
                std::cout << "Expected: 0" << std::endl;
                std::cout << "Got: " << r[i] << std::endl;
                break;
            }
        }
    }
    std::cout << "Srealloc Test Passed!" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    // test smalloc with size 0
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Smalloc Test with size 0" << std::endl;
    int *s = (int *)smalloc(0);
    if (s != NULL)
    {
        std::cout << "Smalloc Test Failed!" << std::endl;
        std::cout << "Expected: NULL" << std::endl;
        std::cout << "Got: " << s << std::endl;
    }
    else
    {
        std::cout << "Smalloc Test Passed!" << std::endl;
    }
    std::cout << "|-----------------------------------|" << std::endl;
    // test scalloc with size 0
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Scalloc Test with size 0" << std::endl;
    int *t = (int *)scalloc(0, sizeof(int));
    if (t != NULL)
    {
        std::cout << "Scalloc Test Failed!" << std::endl;
        std::cout << "Expected: NULL" << std::endl;
        std::cout << "Got: " << t << std::endl;
    }
    else
    {
        std::cout << "Scalloc Test Passed!" << std::endl;
    }
    std::cout << "|-----------------------------------|" << std::endl;
    // test srealloc with size 0
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Srealloc Test with size 0" << std::endl;
    int *u = (int *)srealloc(r, 0);
    if (u != NULL)
    {
        std::cout << "Srealloc Test Failed!" << std::endl;
        std::cout << "Expected: NULL" << std::endl;
        std::cout << "Got: " << u << std::endl;
    }
    else
    {
        std::cout << "Srealloc Test Passed!" << std::endl;
    }
    std::cout << "|-----------------------------------|" << std::endl;
    // test sfree with NULL
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Sfree Test with NULL" << std::endl;
    sfree(NULL);
    std::cout << "Sfree Test Passed!" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    // test sfree with invalid pointer
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Sfree Test with invalid pointer" << std::endl;
    int *v = (int *)malloc(sizeof(int));
    sfree(v);
    sfree(v);
    std::cout << "Sfree Test Passed!" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    // test smalloc with size 128 KB
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Smalloc Test with size 128 KB" << std::endl;
    int *w = (int *)smalloc(128 * 1024);
    if (w == NULL)
    {
        std::cout << "Smalloc Test Failed!" << std::endl;
        std::cout << "Got NULL" << std::endl;
    }
    else
    {
        std::cout << "Smalloc Test Passed!" << std::endl;
    }
    std::cout << "|-----------------------------------|" << std::endl;


    return 0;
}
