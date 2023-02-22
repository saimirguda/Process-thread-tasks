#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

// Implement the function call in assembly in the TODO section below.
// Do not change the functionality or the signature of the function to be called.
// Do not modify this file outside the TODO section.

// Function to be called:
uint32_t countInRange(const double *number_array, uint32_t array_len, double lower, double upper, uint32_t *first_match);

int main(int argc, char **argv)
{
    setbuf(stdout, NULL); // Disable buffering

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <number1,number2,..> <lower> <upper>\n", argv[0]);
        return 1;
    }

    char *numbers_string = argv[1];

    uint32_t array_len = 0;
    for (size_t i = 0; i < strlen(numbers_string); i++)
    {
        if (numbers_string[i] == ',')
        {
            array_len++;
        }
    }
    array_len++;

    if (array_len == 0)
    {
        fprintf(stderr, "Error: length of numbers == 0\n");
        return 1;
    }

    double numbers[array_len];
    char *number_str;
    for (uint32_t i = 0; (number_str = strtok(numbers_string, ",")); i++, numbers_string = NULL)
    {
        numbers[i] = atof(number_str);
    }

    double lower = atof(argv[2]);
    double upper = atof(argv[3]);

    uint32_t nr_matches = (uint32_t)-1;
    uint32_t first_match = -1;

    printf("Calling function with parameters: number_array=%p, array_len=%u, lower=%f, upper=%f, first_match=%p\n",
           numbers, array_len, lower, upper, &first_match);

    // Use (only!) GCC inline assembly to call `countInRange`. Always consider the expected data types.
    // Parameters: numbers, array_len, lower, upper, &first_match
    // Store the return value in nr_matches
    // You may want to use movq to set up the registers for floating-point arguments, 
    // as there are no proper constraints for double parameters!

    // ########## TODO START ##########
    // System V AMD64 ABI implementation (64-bit)
/*
    asm volatile(
        "push %%rbp\n"
        "mov  %%rbp, %%rsp\n"
        "push %[first_match]\n"
        "push %[upper]\n"
        "push %[lower]\n"
        "push %[array_len]\n"
        "push %[numbers]\n"  
        "call countInRange\n" 
        "add  %%esp, 20\n"
        "mov %%eax, %[nr_matches]\n"
        "mov $0, %%eax\n"
        "mov %%rsp, %%rbp\n"  
        "pop %%rbp"
        :  [nr_matches] "=g" (nr_matches)
        :  [numbers] "g" (numbers),[array_len] "g" (array_len),[lower] "g" (lower),[upper] "g" (upper), [first_match] "g" (&first_match)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11","r12","r13","r14","r15", "memory", "cc",
          "xmm0","xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8","xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "mm0","mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm6",
          "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );

*/
        
          asm volatile(
        

        "mov %[numbers], %%rdi\n"  
        "mov %[array_len], %%esi\n"
        "movsd %[lower], %%xmm0\n"
        "movsd %[upper], %%xmm1\n"
        "mov %[first_match], %%rdx\n"
        "call countInRange\n" 
        "mov %%eax, %[nr_matches]\n"
        "mov $0, %%eax\n"  

        :  [nr_matches] "=g" (nr_matches)
        :  [numbers] "g" (numbers),[array_len] "g" (array_len), [lower] "g" (lower),[upper] "g" (upper),[first_match] "g" (&first_match)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory", "cc",
          "xmm0","xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8","xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "mm0","mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm6",
          "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
    // ########## TODO END ##########

    printf("Function returned: %d in range.\n", nr_matches);
    printf("First match index is %u.\n", first_match);

    return 0;
}
