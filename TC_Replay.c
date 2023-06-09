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
  // Func: 0x00; 0x01; 0x02; 0x03; 0x04; 0x05; 0x06
  // Opcode: 0x03
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x01, 0x00, 0x03, 0x02); // lb
  ght_cfg_filter(0x01, 0x01, 0x03, 0x02); // lh
  ght_cfg_filter(0x01, 0x02, 0x03, 0x02); // lw
  ght_cfg_filter(0x01, 0x03, 0x03, 0x02); // ld
  ght_cfg_filter(0x01, 0x04, 0x03, 0x02); // lbu
  ght_cfg_filter(0x01, 0x05, 0x03, 0x02); // lhu
  ght_cfg_filter(0x01, 0x06, 0x03, 0x02); // lwu

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

  // Insepct CSR read operations
  // GID: 0x03
  // Func: 0x02
  // Opcode: 0x73
  // Data path: PRFs - 0x01
  ght_cfg_filter(0x03, 0x02, 0x73, 0x01);
  // Map: GID == 0x03 to SE == 0x00
  ght_cfg_mapper(0x03, 0b0001);



  // se: 00, end_id: 0x01, scheduling: rr, start_id: 0x01
  ght_cfg_se(0x00, 0x01, 0x01, 0x01);

  // To do: add RVC commands
  // ...

  //===================== Execution =====================//
  uint64_t Hart_id = 0;
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));

  float a = 0.1;
  float b = 0.2;
  float c = 0.3;

  /* Testing RCU */
  float d = (a + b + c) * 1.7 * 3.2;

  if (d > Hart_id){
    ROCC_INSTRUCTION (1, 0x70);
  }

  uint64_t CSR = 0;
  /* Testing CSR Registers */
  asm volatile ("csrr %0, cycle"  : "=r"(CSR));
  asm volatile ("csrr %0, instret"  : "=r"(CSR));

  //=================== Post execution ===================//
  // Testing LD & SD
  __asm__ volatile(
                    "li   t0,   0x81000000;"         // write pointer
                    "li   t1,   0x55555000;"         // data
                    "li   t2,   0x81000000;"         // Read pointer
                    "j    .loop_store1;");

  __asm__ volatile(
                    ".loop_store1:"
                    "li   a5,   0x81000FFF;"
                    "sd         t1,   (t0);"
                    "addi t1,   t1,   1;"            // data + 1
                    "addi t0,   t0,   0x10;"         // write address + 0x10
                    "blt  t0,   a5,  .loop_store1;");

  __asm__ volatile(
                    "li   t0,   0x81000000;"         // read pointer
                    "j    .loop_load1;");

  __asm__ volatile(
                    ".loop_load1:"
                    "li   a5,   0x81000FFF;"
                    "ld         t1,   (t0);"
                    "addi t0,   t0,   0x10;"         // write address + 0x10
                    "blt  t0,   a5,  .loop_load1;");



  while(1){
    
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