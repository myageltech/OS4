#include <iostream>

#include "my_stdlib.h"
#include <unistd.h>
#include <cmath>
#include <sys/wait.h>
#include <unistd.h>

void check_num_allocated_blocks(int expected){
    std::cout << "check_num_allocated_blocks" << (_num_allocated_blocks() == expected ? " success!" : "Fail!") << std::endl;
    if (_num_allocated_blocks() != expected) {
    std::cout << "Number of allocated blocks is: " << _num_allocated_blocks() << " but should be: " << expected << std::endl;
    exit(2);
    }
}

void check_num_allocated_bytes(int expected){
    std::cout << "check_num_allocated_bytes" << (_num_allocated_bytes() == expected ? " success!" : "Fail!") << std::endl;
    if (_num_allocated_bytes() != expected) {
    std::cout << "number of allocated bytes: " << _num_allocated_bytes() << " but should be: " << expected << std::endl;
    exit(2);
    }
}

void check_num_free_blocks(int expected){
    std::cout << "check_num_free_blocks" << (_num_free_blocks() == expected ? " success!" : "Fail!") << std::endl;
    if (_num_free_blocks() != expected) {
    std::cout << "number of free block : " << _num_free_blocks() << " but should be: " << expected << std::endl;
    exit(2);
    }
}

void check_num_free_bytes(int expected){
    std::cout << "check_num_free_bytes" << (_num_free_bytes() == expected ? " success!" : "Fail!") << std::endl;
    if (_num_free_bytes() != expected) {
    std::cout << "number of free bytes: " << _num_free_bytes() << " but should be: " << expected << std::endl;
    exit(2);
    }
}

void check_num_meta_data_bytes(int expected){
    std::cout << "check_num_meta_data_bytes" << (_num_meta_data_bytes() == expected ? " success!" : "Fail!") << std::endl;
    if (_num_meta_data_bytes() != expected) {
    std::cout << "number of meta_data byte: " << _num_meta_data_bytes() << " but should be: " << expected << std::endl;
    exit(2);
    }
}

void testsmalloc(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(1);
    check_num_allocated_bytes(sizeof(int));
    check_num_free_blocks(0);
    check_num_free_bytes(0);
    check_num_meta_data_bytes(40);
    sfree(p);
}

void tests2smalloc(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)smalloc(sizeof(int));
    *q = 10;
    std::cout << "2 smalloc " << (q == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(2);
    check_num_allocated_bytes(2*sizeof(int));
    check_num_free_blocks(0);
    check_num_free_bytes(0);
    check_num_meta_data_bytes(40*2);
    sfree(p);
    sfree(q);
}

void tests2smalloc1sfree(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)smalloc(sizeof(int));
    *q = 10;
    std::cout << "2 smalloc " << (q == NULL ? "fail" : "success!") << std::endl;
    sfree(p);
    check_num_allocated_blocks(2);
    check_num_allocated_bytes(2*sizeof(int));
    check_num_free_blocks(1);
    check_num_free_bytes(sizeof(int));
    check_num_meta_data_bytes(40*2);
    sfree(q);
}

void tests2smalloc1sfree1srealloc(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)smalloc(sizeof(int));
    *q = 10;
    std::cout << "2 smalloc " << (q == NULL ? "fail" : "success!") << std::endl;
    sfree(p);
    int *r = (int *)srealloc(q, 2 * sizeof(int));
    *r = 10;
    std::cout << "1 srealloc " << (r == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(2);
    check_num_allocated_bytes(2*sizeof(int));
    check_num_free_blocks(1);
    check_num_free_bytes(sizeof(int));
    check_num_meta_data_bytes(40*2);
    sfree(r);
}

void tests2smalloc1sfree1srealloc1scalloc(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)smalloc(sizeof(int)); // free 40 alloc 42
    *q = 10;
    std::cout << "2 smalloc " << (q == NULL ? "fail" : "success!") << std::endl;
    sfree(p); // alloc 42 free 41
    int *r = (int *)srealloc(q, 2 * sizeof(int)); // alloc 42 free 41
    *r = 10;
    std::cout << "1 srealloc " << (r == NULL ? "fail" : "success!") << std::endl;
    int *s = (int *)scalloc(5, sizeof(int)); // alloc 42 free 40
    if (s == NULL)
    {
        std::cout << "Scalloc Test Failed!" << std::endl;
        std::cout << "Got NULL" << std::endl;
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (s[i] != 0)
            {
                std::cout << "Scalloc Test Failed!" << std::endl;
                std::cout << "Expected: 0" << std::endl;
                std::cout << "Got: " << s[i] << std::endl;
                break;
            }
        }
    }
    std::cout << "Scalloc Test Passed!" << std::endl;
    check_num_allocated_blocks(2);
    check_num_allocated_bytes(2*sizeof(int));
    check_num_free_blocks(1);
    check_num_free_bytes(sizeof(int));
    check_num_meta_data_bytes(40*2);
    sfree(r);
    sfree(s);
}

int main(int argc, char const *argv[])
{
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "test_2" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "smalloc Test" << std::endl;
    testsmalloc();
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "2smalloc Test" << std::endl;
    tests2smalloc();
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "2smallocandfree Test" << std::endl;
    tests2smalloc1sfree();
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "2smalloc1free1realloc Test" << std::endl;
    tests2smalloc1sfree1srealloc();
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "2smalloc1free1realloc1calloc Test" << std::endl;
    tests2smalloc1sfree1srealloc1scalloc();
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "*****************************************" << std::endl;
    std::cout << "so long bitches!!!!! passed all malloc_2" << std::endl;
    std::cout << "*******************************************" << std::endl;
    std::cout << "*******************************************" << std::endl;
    return 0;
}
