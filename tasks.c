#include <stdio.h>
#include <stdlib.h>
#include "rocc.h"
#include "spin_lock.h"
#include "ght.h"
#include "ghe.h"
#include "tasks.h"


int checker (int hart_id)
{
  int pmc = 0;
  // lock_acquire(&uart_lock);
  // printf("Hello, World! From Hart %d. \n", hart_id);
  // lock_release(&uart_lock);

  //================== Initialisation ==================//
  ghe_asR();
  ghe_initailised(1);
  
  //===================== Execution =====================// 

  while (ghe_rsur_status() != 0x01 ){
  }
  ROCC_INSTRUCTION (1, 0x60);

  // __asm__ volatile ("nop");
  // __asm__ volatile ("nop");
  // __asm__ volatile ("nop");
  // __asm__ volatile ("nop");

  R_INSTRUCTION_JLR (3, 0x00);


  //=================== Post execution ===================//
  /*
  __asm__(
          "li   t0,   0x81000000;"         // write pointer
          "li   t1,   0x55555000;"         // data
          "li   t2,   0x81000000;"         // Read pointer
          "j    .loop_store;");

  __asm__(
          ".loop_store:"
          "li   a5,   0x81000FFF;"
          "sw         t1,   (t0);"
          "addi t1,   t1,   1;"            // data + 1
          "addi t0,   t0,   4;"            // write address + 4
          "blt  t0,   a5,  .loop_store;"
          "li   t0,   0x82000000;"
          "li   t2,   0x81000000;"
          "j    .loop_load;");

  __asm__(
          ".loop_load:"
          "li   a5,   0x82000FFF;"
          "lb   t1,   (t2);"
          "sb         t1,   (t0);"
          "addi t0,   t0,   4;"
          "addi t2,   t2,   4;"
          "blt  t0,   a5,  .loop_load;");

  __asm__(
          ".loop_load2:"
          "li   a5,   0x82000FFF;"
          "ld   t1,   (t2);"
          "sb         t1,   (t0);"
          "addi t0,   t0,   4;"
          "addi t2,   t2,   4;"
          "blt  t0,   a5,  .loop_load;");
  */


  //=================== Post execution ===================//
  lock_acquire(&uart_lock);
  printf("[C%x Checker]: Completed! PMC = %x \r\n", hart_id, pmc);
  lock_release(&uart_lock);
  ghe_release();

  ghe_initailised(0);  

  while(1){

  }

  return 0;
}


uint64_t task_synthetic_malloc (uint64_t base)
{
  int *ptr = NULL;
  int ptr_size = 32;
  int sum = 0;

  ptr = (int*) malloc(ptr_size * sizeof(int));

  // if memory cannot be allocated
  if(ptr == NULL) {
    printf("Error! memory not allocated. \r\n");
    exit(0);
  }

  for (int i = 0; i < ptr_size; i++)
  {
    *(ptr + i) = base + i;
  }

  for (int i = 0; i < ptr_size; i++)
  {
    sum = sum + *(ptr+i);
  }

  free(ptr);

  return sum;
}