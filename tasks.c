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
  ghe_initailised(1);
  
  //===================== Execution =====================// 
  while (ghe_rsur_status() != 0x01 ){
  }
  ROCC_INSTRUCTION (1, 0x60);

  __asm__ volatile ("nop");
  __asm__ volatile ("nop");
  __asm__ volatile ("nop");
  __asm__ volatile ("nop");

  /* Testing load PC*/
  R_INSTRUCTION_JLR (3, 0x00);

 



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