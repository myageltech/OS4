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
    expected = expected - _num_meta_data_bytes();
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
    expected = expected - _num_meta_data_bytes();
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

void surroundingTest() {
  // test smalloc
    check_num_allocated_blocks(32);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32) );
    check_num_meta_data_bytes(40 * 32);
    std::cout << "|-----------------------------------|" << std::endl<< "Smalloc Test" << std::endl;
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;

    check_num_allocated_blocks(42);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(41);
    check_num_free_bytes((128 * 1024 * 32) - 128);
    check_num_meta_data_bytes(40 * 42);

    // test sfree
    std::cout << "|-----------------------------------|" << std::endl << "Sfree Test" << std::endl;
    sfree(p);
    
    check_num_allocated_blocks(32);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32));
    check_num_meta_data_bytes(40 * 32);

    std::cout << "|-----------------------------------|" << std::endl;
    // test scalloc
    std::cout << std::endl << "|-----------------------------------|" << std::endl;
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
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "Smalloc Test with size 0" << std::endl;
    int *s = (int *)smalloc(0);
    if (s == NULL)
    {
        std::cout << "Smalloc Test with size 0 Failed!" << std::endl;
        std::cout << "Got NULL" << std::endl;
    }
    else
    {
        std::cout << "Smalloc Test with size 0 Failed!" << std::endl;
        std::cout << "Expected: NULL" << std::endl;
        std::cout << "Got: " << s << std::endl;
    }
    sfree(r);
}

void test2malloc1afteranother(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)smalloc(sizeof(int));
    *q = 10;
    std::cout << "2 smalloc " << (q == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(42);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(40);
    check_num_free_bytes((128 * 1024 * 32) - 256);
    check_num_meta_data_bytes(40 * 42);
    sfree(p);
    sfree(q);
}

void test2malloc1afteranotherwithfree() {
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)smalloc(sizeof(int));
    *q = 10;
    std::cout << "2 smalloc " << (q == NULL ? "fail" : "success!") << std::endl;
    sfree(p);
    check_num_allocated_blocks(42);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(41);
    check_num_free_bytes((128 * 1024 * 32) - 128);
    check_num_meta_data_bytes(40 * 42);
    sfree(q);
}

void test2malloc1afteranotherwithfreeandrealloc() {
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
    check_num_allocated_blocks(42);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(41);
    check_num_free_bytes((128 * 1024 * 32) - 128);
    check_num_meta_data_bytes(40 * 42);
    sfree(r);

}

void test2malloc1afteranotherwithfreeandreallocandcalloc(){
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
    check_num_allocated_blocks(42);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(40);
    check_num_free_bytes((128 * 1024 * 32) - 128*2);
    check_num_meta_data_bytes(40 * 42);
    sfree(r);
    sfree(s);

}

void testsmalloc(){
    int *p = (int *)smalloc(sizeof(int));
    *p = 10;
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(42);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(41);
    check_num_free_bytes((128 * 1024 * 32) - 128);
    check_num_meta_data_bytes(40 * 42);
    sfree(p);
}

void smallochigherthen128KB(){
    int *p = (int *)smalloc(128 * 1024 + 1);
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(33);
    check_num_allocated_bytes(128 * 1024 * 32 + 128 * 1024 + 1 + 40);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32));
    check_num_meta_data_bytes(40 * 33);
    sfree(p);
    check_num_allocated_blocks(32);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32));
    check_num_meta_data_bytes(40 * 32);
}

void scallochigherthen128KB(){
    int yoram = 128 * 1024 * 32;
    int *p = (int *)scalloc(2, 128 * 1024);
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(33);
    check_num_allocated_bytes(yoram + (2* 128 * 1024) + 40);
    check_num_free_blocks(32);
    check_num_free_bytes((yoram));
    check_num_meta_data_bytes(40 * 33);
    sfree(p);
    check_num_allocated_blocks(32);
    check_num_allocated_bytes(yoram);
    check_num_free_blocks(32);
    check_num_free_bytes((yoram));
    check_num_meta_data_bytes(40 * 32);
}

void reallochigherthen128KB(){
    int *p = (int *)smalloc(128 * 1024);
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    int *q = (int *)srealloc(p, 128 * 1024 + 1);
    std::cout << "1 srealloc " << (q == NULL ? "fail" : "success!") << std::endl;
    check_num_allocated_blocks(33);
    check_num_allocated_bytes(128 * 1024 * 32 + 128 * 1024 + 1 + 40);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32));
    check_num_meta_data_bytes(40 * 33);
    sfree(q);
    check_num_allocated_blocks(32);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32));
    check_num_meta_data_bytes(40 * 32);
}

void sfreehigherthen128KB(){
    int *p = (int *)smalloc(128 * 1024+1);
    std::cout << "1 smalloc " << (p == NULL ? "fail" : "success!") << std::endl;
    sfree(p);
    check_num_allocated_blocks(32);
    check_num_allocated_bytes(128 * 1024 * 32);
    check_num_free_blocks(32);
    check_num_free_bytes((128 * 1024 * 32));
    check_num_meta_data_bytes(40 * 32);
}

int main(int argc, char const *argv[])
{
    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "smalloc higherthen128KB Tests" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    smallochigherthen128KB();

    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "scallochigherthen128KB higherthen128KB Tests" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    scallochigherthen128KB();

    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "reallochigherthen128KB higherthen128KB Tests" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;
    reallochigherthen128KB();

    std::cout << "|-----------------------------------|" << std::endl;
    std::cout << "sfreehigherthen128KB higherthen128KB Tests" << std::endl;
    std::cout << "|-----------------------------------|" << std::endl;

    sfreehigherthen128KB();

    std::cout << "*******************************************" << std::endl;
    std::cout << "*******************************************" << std::endl;

    std::cout << "so long bitches!!!!! passed all malloc_3" << std::endl;
    std::cout << "*******************************************" << std::endl;
    std::cout << "*******************************************" << std::endl;
    return 0;
}
