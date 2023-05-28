#include <stdio.h>
#include <stdlib.h>
#include "rocc.h"
#include "spin_lock.h"
#include "ght.h"
#include "ghe.h"
#include "tasks.h"


int uart_lock;


/* Core_0 thread */
int main(void)
{
  uint64_t Hart_id = 0;
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));


  //================== Initialisation ==================//
  ght_set_numberofcheckers(2);
  ght_set_satp_priv();

  // se: 01, end_id: 0x01, scheduling: rr, start_id: 0x01
  // Arch-RFs
  ght_cfg_se (0x01, 0x01, 0x01, 0x01);
  ght_cfg_mapper (0x07, 0b0010);

  ROCC_INSTRUCTION (1, 0x70);



  lock_acquire(&uart_lock);
  printf("C0: Test is now started: \r\n");
  lock_release(&uart_lock);
  
  double a = 3.74;
  double b = 7.47;
  double c = a * b * 2.1 * Hart_id;


  //=================== Post execution ===================//
  ght_set_status (0x02); // ght: stop
  uint64_t status;
  while (((status = ght_get_status()) < 0x1FFFF) || (ght_get_buffer_status() != GHT_EMPTY)) {

  }

  if (c >= 30) {
    lock_acquire(&uart_lock);
    printf("Results: %.2f \r\n", c);
    lock_release(&uart_lock);
  }
}


/* Core_1 & 2 thread */
int __main(void)
{
  uint64_t Hart_id = 0;
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));

  ghe_go();
  
  double a = 3.74;
  double b = 7.47;
  double c = a * b * 2.1 * Hart_id;

  //=================== Post execution ===================//
  if (c >= 30) {
    lock_acquire(&uart_lock);
    printf("Results: %.2f \r\n", c);
    lock_release(&uart_lock);
  }


  ghe_release();

  idle();
  return 0;
}