#include <stdio.h>
#include <stdint.h>
///#include <string.h>
int binaryNum[31];
int cache_size;
char manufa[10];
int ihk;
typedef union value_Store 
{
  uint32_t value;
  char     ch[4];
}
Val_Store;

void cpuid(uint32_t op, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    
    
    int a[4];

    asm volatile ("cpuid":"=a"(a[0]),"=b"(a[1]),"=c"(a[2]),"=d"(a[3]):"a"(op));


    *eax = (uint32_t)a[0];
    *ebx = (uint32_t)a[1];
    *ecx = (uint32_t)a[2];
    *edx = (uint32_t)a[3];
    
}

void print_ebchar4(uint32_t a)
{
  Val_Store temp;
  temp.value = a;
  
  printf("%c%c%c%c",temp.ch[0],temp.ch[1],temp.ch[2],temp.ch[3]);
}
void decToBinary(int n)
{

    int i = 0;
    while (n > 0) {

        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
}
void binarytoDec(int n)
{

    int base = 1;
    int rem;
    while(n > 0)
    {
        rem = n % 10;
        cache_size = cache_size + rem*base;
        n = n / 10;
        base = base*2;
    }
}

int convertToInt(char string[])
{
    int num =0;
    int len = 8;
    for(int i = 0; i < len; i++)
    {
        num = num*10 + (string[i] - 48);
    }
    return num;
}


int main()
{
    /* TODO: Find the processor's manufacturer ID string, 
     *       the processor's cache line size (in bytes!), and the 
     *       availability bits of the given features from the wiki 
     *       using the cpuid instruction and inline assembly. 
     *       Do not use builtin functions.
     *       Print the values to stdout in the given formats.
     *       Feel free to modify this file wherever you want.
     */

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    printf("Manufacturer id: ");
    print_ebchar4(ebx);
    print_ebchar4(edx);
    print_ebchar4(ecx);
    
    printf("\n");
    
    cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
    decToBinary(eax);
    char smth[8];
    for(int i = 15; i >= 8; i--)
    {
       char value = binaryNum[i] + '0';
       smth[15 - i] = value;
    }
    int smthInt = convertToInt(smth);
    binarytoDec(smthInt);
    cache_size = cache_size;
    //printf("Cache line size: %u\n", cache_size);
    printf("Number of Linear Address Bits: %u\n", cache_size);

    cpuid(1, &eax, &ebx, &ecx, &edx);
    printf("fpu: %u, apic: %u, acpi: %u, sse: %u,", 
        (edx &  (1<< 0))?1:0, ( edx &  (1<< 9))?1:0,
        ( edx &  (1<< 22))?1:0, ( edx &  (1<< 25))?1:0); 
    printf(" aes: %u, ", (ecx &  (1<< 25))?1:0); 

    ecx = 0;
    cpuid(7, &eax, &ebx, &ecx, &edx);

    printf("avx512: %u, smap: %u, ", (ebx &  (1<< 16))?1:0, ( ebx &  (1<< 20))?1:0); 

    printf("vaes: %u\n", ( ecx &  (1<< 9))?1:0);

    return 0;
}