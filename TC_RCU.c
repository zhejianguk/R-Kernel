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

  // Map: GID == 0x07 to SE == 0x00
  ght_cfg_mapper (0x07, 0b0001);
  
  // Insepct load operations 
  // GID: 0x01 
  // Func: 0x00; 0x01; 0x02; 0x03; 0x04; 0x05
  // Opcode: 0x03
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x01, 0x00, 0x03, 0x02); // lb
  ght_cfg_filter(0x01, 0x01, 0x03, 0x02); // lh
  ght_cfg_filter(0x01, 0x02, 0x03, 0x02); // lw
  ght_cfg_filter(0x01, 0x03, 0x03, 0x02); // ld
  ght_cfg_filter(0x01, 0x04, 0x03, 0x02); // lbu
  ght_cfg_filter(0x01, 0x05, 0x03, 0x02); // lhu
  // Func: 0x02; 0x03; 0x04
  // Opcode: 0x07
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x01, 0x02, 0x07, 0x02); // flw
  ght_cfg_filter(0x01, 0x03, 0x07, 0x02); // fld
  ght_cfg_filter(0x01, 0x04, 0x07, 0x02); // flq
  // Map: GID == 0x01 to SE == 0x00
  ght_cfg_mapper (0x01, 0b0001);
  

  // Insepct store operations 
  // GID: 0x02
  // Func: 0x00; 0x01; 0x02; 0x03
  // Opcode: 0x23
  // Data path: STQ - 0x03
  ght_cfg_filter(0x02, 0x00, 0x23, 0x03); // sb
  ght_cfg_filter(0x02, 0x01, 0x23, 0x03); // sh
  ght_cfg_filter(0x02, 0x02, 0x23, 0x03); // sw
  ght_cfg_filter(0x02, 0x03, 0x23, 0x03); // sd
  // Func: 0x02; 0x03; 0x04
  // Opcode: 0x27
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x02, 0x02, 0x27, 0x03); // fsw
  ght_cfg_filter(0x02, 0x03, 0x27, 0x03); // fsd
  ght_cfg_filter(0x02, 0x04, 0x27, 0x03); // fsq
  // Map: GID == 0x02 to SE == 0x00
  ght_cfg_mapper(0x02, 0b0001);

  // se: 00, end_id: 0x01, scheduling: rr, start_id: 0x01
  ght_cfg_se(0x00, 0x01, 0x01, 0x01);

  // To do: add RVC commands
  // ...

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
    ght_set_status (0x01); // start monitoring
    ROCC_INSTRUCTION (1, 0x70);
  }

  double e = (c - b + a) * 1.1;

  if (e > Hart_id) {
    //=================== Post execution ===================//
    __asm__ volatile(
                      "li   t0,   0x81000000;"         // write pointer
                      "li   t1,   0x55555000;"         // data
                      "li   t2,   0x81000000;"         // Read pointer
                      "j    .loop_store;");

    __asm__ volatile(
                      ".loop_store:"
                      "li   a5,   0x81000FFF;"
                      "sw         t1,   (t0);"
                      "addi t1,   t1,   1;"            // data + 1
                      "addi t0,   t0,   0x10;"         // write address + 0x10
                      "blt  t0,   a5,  .loop_store;");

    __asm__ volatile(
                      "li   t0,   0x81000000;"         // read pointer
                      "j    .loop_load;");

    __asm__ volatile(
                      ".loop_load:"
                      "li   a5,   0x81000FFF;"
                      "lw         t1,   (t0);"
                      "addi t0,   t0,   0x10;"         // write address + 0x10
                      "blt  t0,   a5,  .loop_load;");
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