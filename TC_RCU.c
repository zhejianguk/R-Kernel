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
  //================== Initialisation ==================//
  ght_set_numberofcheckers(2);
  ght_set_satp_priv();
  // ght_debug_filter_width(0x00); // Default width

  // se: 01, end_id: 0x01, scheduling: rr, start_id: 0x01
  ght_cfg_se (0x01, 0x01, 0x01, 0x01);
  ght_cfg_mapper (0x07, 0b0010);

  lock_acquire(&uart_lock);
  printf("C0: Test is now started: \r\n");
  lock_release(&uart_lock);
  // ght_set_status (0x01); // start monitoring
  //===================== Execution =====================//
  uint64_t Hart_id = 0;
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));

  double           a = 0.1;
  double           b = 0.2;
  double           c = 0.3;

  /* Testing RCU */
  double           d = (a + b + c) * 1.7 * 3.2;

  if (d > Hart_id){
    ROCC_INSTRUCTION (1, 0x70);
  }

  d = (Hart_id + 1.3) * 1.7 * 3.2;

  if (d > Hart_id){
  //=================== Post execution ===================//
  __asm__ volatile(
                    "li   t0,   0x81000000;"         // write pointer
                    "li   t1,   0x55555000;"         // data
                    "li   t2,   0x81000000;"         // Read pointer
                    "j    .loop_store;");

  __asm__ volatile(
                    ".loop_store:"
                    "li   a5,   0x81000FFF;"
                    "lw         t1,   (t0);"
                    "addi t1,   t1,   1;"            // data + 1
                    "addi t0,   t0,   4;"            // write address + 4
                    "blt  t0,   a5,  .loop_store;"
                    "li   t0,   0x82000000;"
                    "li   t2,   0x81000000;");
  }

  while(1){
    
  }

  uint64_t status;
  while (((status = ght_get_status()) < 0x1FFFF) || (ght_get_buffer_status() != GHT_EMPTY)) {

  }

  uint64_t m_counter;
  uint64_t i_counter;
  uint64_t g_counter;

  ght_unset_satp_priv();
  ght_set_status (0x00);

  m_counter = debug_mcounter();
  i_counter = debug_icounter();
  g_counter = debug_gcounter();

  lock_acquire(&uart_lock);
  printf("Debug, m-counter: %x \r\n", m_counter);
  printf("Debug, i-counter: %x \r\n", i_counter);
  printf("Debug, g-counter: %x \r\n", g_counter);
  lock_release(&uart_lock);

  while (ght_get_initialisation() == 1){ 
  }

  return 0;
}


/* Core_1 & 2 thread */
int __main(void)
{
  uint64_t Hart_id = 0;
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));
  
  switch (Hart_id){
      case 0x01:
        checker(Hart_id);
      break;

      case 0x02:
        checker(Hart_id);
      break;
      
      default:
      break;
  }
  
  idle();
  return 0;
}