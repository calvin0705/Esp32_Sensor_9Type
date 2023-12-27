#include <iostream>
using namespace std;

int* sum_ptr;
int sum_var = 20;

void geeks()
{
    int var = 10;
 
    // declare pointer variable
    int* ptr;
 
    // note that data type of ptr and var must be same
    ptr = &var;
 
    // assign the address of a variable to a pointer
    printf("Value at ptr = %p \n", ptr);
    printf("Value at var = %d \n", var);
    printf("Value at *ptr = %d \n", *ptr);
}

int myFunction(int x, int y) {

  sum_ptr = &sum_var;
  *sum_ptr = *sum_ptr + 1;

  return *sum_ptr;
}

// int myFunction(int x, int y) {

//   sum_ptr = &sum_var;
//   *sum_ptr = *sum_ptr + 1;

//   return *sum_ptr;
// }