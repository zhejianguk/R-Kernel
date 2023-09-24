#include <stdio.h>
#include <stdlib.h>
#include "rocc.h"
#include "spin_lock.h"
#include "ght.h"
#include "ghe.h"
#include "tasks.h"


#define NUM_CHECKERS 2
int uart_lock;

int r_ini (int num_checkers);

/* Core_0 thread */
int main(void)
{
  r_ini(NUM_CHECKERS);

  while (ght_get_initialisation() == 0){
    // Wait for the checkers to be completed 
 	}
  uint64_t Hart_id = 0;
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));
  printf("[Boom-C%x]: Test is now started: \r\n", Hart_id);
  ght_set_satp_priv();
  ROCC_INSTRUCTION (1, 0x31); // start monitoring
  ROCC_INSTRUCTION_S (1, 0X01, 0x70); // ISAX_Go
  //===================== Execution =====================//


  float a = 0.1;
  float b = 0.2;
  float c = 0.3;

  /* Testing RCU */
  float d = (a + b + c) * 1.7 * 3.2;
  
  uint64_t CSR = 0;
  /* Testing CSR Registers */
  asm volatile ("csrr %0, cycle"  : "=r"(CSR));
  asm volatile ("csrr %0, instret"  : "=r"(CSR));
  asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));
  
  /* Testing Floating Points */
  double e = (c - b + a) * 1.1;
  double f = ((e + d) * (d - b)) / 2.1;
  double g = (c + 1.1)/2;
  double h = (a - 0.05);
  double i = (f + 1.1);
  double j = a + b + c + d + e + f + g + h + i;


 

  // Test the correctness of the CSR insts
  if ((j * Hart_id) == 0) {
    for (int i; i < 3; i++){
      e = i * 1.2 + 3;
      b = j + 1.7;
      a = (e + b) * 2.2;
      asm volatile ("csrr %0, cycle"  : "=r"(CSR));
      asm volatile ("csrr %0, instret"  : "=r"(CSR));
      asm volatile ("csrr %0, mhartid"  : "=r"(Hart_id));
      a = a + CSR;

      if (a > Hart_id) {
      //=================== Post execution ===================//
      // Testing LR.W & SC.W
      __asm__ volatile(
                    "li   t0,   0x81000000;"     // write pointer
                    "li   t1,   0x55555000;"     // data
                    "li   a5,   0x81000FFF;"     // end address
                    "j    .loop_store;");

      __asm__ volatile(
                    ".loop_store:"
                    "lr.w a0,   (t0);"            // load reserved word from memory to a0
                    "sc.w a0,   t1,   (t0);"      // attempt to store t1 at t0
                    "bnez a0,   .loop_store;"     // retry if sc.w failed
                    "addi t1,   t1,   1;"         // data + 1
                    "addi t0,   t0,   0x100;"     // write address + 0x100
                    "blt  t0,   a5,  .loop_store;");

      __asm__ volatile(
                    "li   t0,   0x81000000;"     // read pointer
                    "li   a5,   0x81000FFF;"     // end address
                    "j    .loop_load;");

      __asm__ volatile(
                    ".loop_load:"
                    "lr.w       t1,   (t0);"      // load word from memory to t1
                    "addi t0,   t0,   0x100;"     // read address + 0x100
                    "blt  t0,   a5,  .loop_load;");


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

      // Testing LW & SW
      __asm__ volatile(
                        "li   t0,   0x81000000;"         // write pointer
                        "li   t1,   0x55555000;"         // data
                        "li   t2,   0x81000000;"         // Read pointer
                        "j    .loop_store2;");

      __asm__ volatile(
                        ".loop_store2:"
                        "li   a5,   0x81000FFF;"
                        "sw         t1,   (t0);"
                        "addi t1,   t1,   1;"            // data + 1
                        "addi t0,   t0,   0x10;"         // write address + 0x10
                        "blt  t0,   a5,  .loop_store2;");

      __asm__ volatile(
                        "li   t0,   0x81000000;"         // read pointer
                        "j    .loop_load2;");

      __asm__ volatile(
                        ".loop_load2:"
                        "li   a5,   0x81000FFF;"
                        "lw         t1,   (t0);"
                        "addi t0,   t0,   0x10;"         // write address + 0x10
                        "blt  t0,   a5,  .loop_load2;");

      

      // Testing LH & SH
      // ROCC_INSTRUCTION_S (1, 0X04, 0x70); // Sys_call
      __asm__ volatile(
                        "li   t0,   0x81000000;"         // write pointer
                        "li   t1,   0x55555000;"         // data
                        "li   t2,   0x81000000;"         // Read pointer
                        "j    .loop_store3;");

      __asm__ volatile(
                        ".loop_store3:"
                        "li   a5,   0x81000FFF;"
                        "sh         t1,   (t0);"
                        "addi t1,   t1,   1;"            // data + 1
                        "addi t0,   t0,   0x10;"         // write address + 0x10
                        "blt  t0,   a5,  .loop_store3;");
      // ROCC_INSTRUCTION_S (1, 0X08, 0x70); // Sys_call_back

      __asm__ volatile(
                        "li   t0,   0x81000000;"         // read pointer
                        "j    .loop_load3;");

      __asm__ volatile(
                        ".loop_load3:"
                        "li   a5,   0x81000FFF;"
                        "lh         t1,   (t0);"
                        "addi t0,   t0,   0x10;"         // write address + 0x10
                        "blt  t0,   a5,  .loop_load3;");

      

      // Testing LB & SB
      __asm__ volatile(
                        "li   t0,   0x81000000;"         // write pointer
                        "li   t1,   0x55555000;"         // data
                        "li   t2,   0x81000000;"         // Read pointerROCC_INSTRUCTION_S
                        "j    .loop_store4;");

      __asm__ volatile(
                        ".loop_store4:"
                        "li   a5,   0x81000FFF;"
                        "sb         t1,   (t0);"
                        "addi t1,   t1,   1;"            // data + 1
                        "addi t0,   t0,   0x10;"         // write address + 0x10
                        "blt  t0,   a5,  .loop_store4;");

      __asm__ volatile(
                        "li   t0,   0x81000000;"         // read pointer
                        "j    .loop_load4;");

      __asm__ volatile(
                        ".loop_load4:"
                        "li   a5,   0x81000FFF;"
                        "lb         t1,   (t0);"
                        "addi t0,   t0,   0x10;"         // write address + 0x10
                        "blt  t0,   a5,  .loop_load4;");

      // Testing LR.D & SC.D
      __asm__ volatile(
                        "li   t0,   0x81000000;"     // write pointer
                        "li   t1,   0x55555000;"     // data
                        "li   a5,   0x81000FFF;"     // end address
                        "j    .loop_store5;");

      __asm__ volatile(
                        ".loop_store5:"
                        "lr.d a0,   (t0);"            // load reserved word from memory to a0
                        "sc.d a0,   t1,   (t0);"      // attempt to store t1 at t0
                        "bnez a0,   .loop_store;"     // retry if sc.w failed
                        "addi t1,   t1,   1;"         // data + 1
                        "addi t0,   t0,   0x100;"     // write address + 0x100
                        "blt  t0,   a5,  .loop_store5;");

      __asm__ volatile(
                        "li   t0,   0x81000000;"     // read pointer
                        "li   a5,   0x81000FFF;"     // end address
                        "j    .loop_load5;");

      __asm__ volatile(
                        ".loop_load5:"
                        "lr.d       t1,   (t0);"      // load word from memory to t1
                        "addi t0,   t0,   0x100;"     // read address + 0x100
                        "blt  t0,   a5,  .loop_load5;");
      
      
      }
    }
  }




  //=================== Post execution ===================//
  ROCC_INSTRUCTION (1, 0x32); // stop monitoring
  ROCC_INSTRUCTION_S (1, 0X02, 0x70); // ISAX_Stop
  __asm__ volatile("nop");
  __asm__ volatile("nop");
  __asm__ volatile("nop");
  __asm__ volatile("nop");


  uint64_t status;
  while ((status = ght_get_status()) < 0x1FFFF) {

  }

  printf("[Boom-C%x]: Test is now completed. \r\n", Hart_id);
	ght_unset_satp_priv();
	ROCC_INSTRUCTION (1, 0x30); // reset monitoring
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

      case 0x03:
        checker(Hart_id);
      break;

      case 0x04:
        checker(Hart_id);
      break;

      
      default:
      break;
  }
  
  idle();
  return 0;
}

int r_ini (int num_checkers){
  //================== Initialisation ==================//
  ght_set_numberofcheckers(num_checkers);
  // ght_debug_filter_width(0x00); // Default width
  
  // Insepct load operations 
  // GID: 0x01 
  // Func: 0x00; 0x01; 0x02; 0x03; 0x04; 0x05; 0x06
  // Opcode: 0x03
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x01, 0x00, 0x03, 0x02); // lb
  ght_cfg_filter(0x01, 0x01, 0x03, 0x02); // lh
  ght_cfg_filter(0x01, 0x02, 0x03, 0x02); // lw
  ght_cfg_filter(0x01, 0x04, 0x03, 0x02); // lbu
  ght_cfg_filter(0x01, 0x05, 0x03, 0x02); // lhu
  ght_cfg_filter(0x01, 0x03, 0x03, 0x02); // ld
  ght_cfg_filter(0x01, 0x06, 0x03, 0x02); // lwu
  // Func: 0x02; 0x03; 0x04
  // Opcode: 0x07
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x01, 0x02, 0x07, 0x02); // flw
  ght_cfg_filter(0x01, 0x03, 0x07, 0x02); // fld
  ght_cfg_filter(0x01, 0x04, 0x07, 0x02); // flq
  // C.load operations 
  // GID: 0x01
  // Func: 0x02; 0x03; 0x04; 0x05; 0x06; 0x07
  // MSB: 0
  // Opcode: 0x00
  ght_cfg_filter_rvc(0x01, 0x02, 0x00, 0x00, 0x02); // c.fld, c.lq
  ght_cfg_filter_rvc(0x01, 0x03, 0x00, 0x00, 0x02); // c.fld, c.lq
  ght_cfg_filter_rvc(0x01, 0x04, 0x00, 0x00, 0x02); // c.lw
  ght_cfg_filter_rvc(0x01, 0x05, 0x00, 0x00, 0x02); // c.lw
  ght_cfg_filter_rvc(0x01, 0x06, 0x00, 0x00, 0x02); // c.flw, c.ld
  ght_cfg_filter_rvc(0x01, 0x07, 0x00, 0x00, 0x02); // c.flw, c.ld

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
  // C.sotre operations 
  // GID: 0x02
  // Func: 0x02; 0x03; 0x04; 0x05; 0x06; 0x07
  // MSB: 1
  // Opcode: 0x00
  ght_cfg_filter_rvc(0x01, 0x02, 0x00, 0x01, 0x02); // c.fsd, c.sq
  ght_cfg_filter_rvc(0x01, 0x03, 0x00, 0x01, 0x02); // c.fsd, c.sq
  ght_cfg_filter_rvc(0x01, 0x04, 0x00, 0x01, 0x02); // c.sw
  ght_cfg_filter_rvc(0x01, 0x05, 0x00, 0x01, 0x02); // c.sw
  ght_cfg_filter_rvc(0x01, 0x06, 0x00, 0x00, 0x02); // c.fsw, c.sd
  ght_cfg_filter_rvc(0x01, 0x07, 0x00, 0x00, 0x02); // c.fsw, c.sd

  // Insepct CSR read operations
  // GID: 0x01
  // Func: 0x02
  // Opcode: 0x73
  // Data path: PRFs - 0x01
  ght_cfg_filter(0x03, 0x02, 0x73, 0x01);

  // Insepct atomic operations
  // GID: 0x2F
  // Func: 0x02; 0x03
  // Opcode: 0x2F
  // Data path: LDQ - 0x02
  ght_cfg_filter(0x01, 0x02, 0x2F, 0x05); // 32-bit
  ght_cfg_filter(0x01, 0x03, 0x2F, 0x05); // 64-bit

  // se: 00, end_id: 0x01, scheduling: rr, start_id: 0x01
  ght_cfg_se(0x00, 0x01, 0x01, 0x01);
  // se: 01, end_id: 0x02, scheduling: rr, start_id: 0x02
  ght_cfg_se(0x01, 0x02, 0x01, 0x02);
  // se: 02, end_id: 0x03, scheduling: rr, start_id: 0x03
  ght_cfg_se(0x02, 0x03, 0x01, 0x03);
  // se: 03, end_id: 0x04, scheduling: rr, start_id: 0x04
  ght_cfg_se(0x03, 0x04, 0x01, 0x04);

  // Map: GIDs for cores
  r_set_corex_p_s(1);
  r_set_corex_p_s(2);
  // r_set_corex_p_s(3);
  // r_set_corex_p_s(4);


  // Shared snapshots
  ght_cfg_mapper (0b00001111, 0b0011);
  ght_cfg_mapper (0b00010111, 0b0011);
  // ght_cfg_mapper (0b00011111, 0b0110);
  // ght_cfg_mapper (0b00100111, 0b1100);

  // To do: add RVC commands
  // ...
  //================== Initialisation ==================//
  lock_acquire(&uart_lock);
  printf("R: Initialisation is completed!\r\n");
  lock_release(&uart_lock);
}