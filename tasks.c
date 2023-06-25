#include <stdio.h>
#include <stdlib.h>
#include "rocc.h"
#include "spin_lock.h"
#include "ght.h"
#include "ghe.h"
#include "tasks.h"


int checker (int hart_id)
{
  //================== Initialisation ==================//
  ghe_asR();
  ght_set_satp_priv();
  ghe_go();
  ghe_initailised(1);

  //===================== Execution =====================//
  ROCC_INSTRUCTION (1, 0x64); // Record PC

  for (int sel_elu = 0; sel_elu < 2; sel_elu ++){
    ROCC_INSTRUCTION_S (1, sel_elu, 0x65);

    while (elu_checkstatus() != 0){
      printf("C%x: Error detected for ELU %x.\r\n", hart_id, sel_elu);
      ROCC_INSTRUCTION_S (1, sel_elu, 0x63);
    }
  }
  
  while (ghe_checkght_status() != 0x02){
    if ((ghe_rsur_status() & 0x01) == 0x01 ){
      ROCC_INSTRUCTION (1, 0x60);
      R_INSTRUCTION_JLR (3, 0x00);
    }
  }

	ghe_initailised(0);
	ghe_release();

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