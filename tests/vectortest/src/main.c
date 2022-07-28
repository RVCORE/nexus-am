#include <stddef.h>
#include <klib.h>

extern void saxpy(size_t n, const float a, const float *x, float *y);
#define ISA 0x34012d
int main(){

  uint64_t aaa = 0x200;
  asm volatile("csrs mstatus, %0" : : "r"(aaa));
  size_t size=3;
  const float a=1.0;
  const float array1[]={1.0,2.0,3.0};
  float array2[]={1.0,2.0,3.0};

  for(int i =0;i<size;i++){
    printf("%f\n", array2[i]);
  }
  printf("----------------\n");

  //调用saxpy函数
  saxpy(size,a,array1,array2);
  for(int i =0;i<size;i++) {
   printf("%f\n", array2[i]);
  }
  printf("Test End!\n");
  return 0;
}