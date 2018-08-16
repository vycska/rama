#include "output.h"
#include "fifos.h"
#include "os.h"
#include "task_oled.h"
#include "uart.h"
#include "utils-asm.h"
#include "lpc824.h"

extern u8log_t u8log;
extern u8g2_t u8g2;
extern struct Task_Oled_Data task_oled_data;

struct Output_Data output_data;

void output(char *buf, enum eOutputSubsystem subsystem, enum eOutputLevel level, int block) {
   int smphrFinished;
   if(level & output_data.mask[subsystem]) {
      if(task_oled_data.log_enabled) {
         u8g2_SetFont(&u8g2,u8g2_font_tom_thumb_4x6_mf);
         u8log_WriteString(&u8log, buf);
      }
      if(block==-1) {
         UART_Transmit(buf);
      }
      else if(block==1) {
         Semaphore_Init(&smphrFinished,0);
         Fifo_Uart_Output_Put(buf, &smphrFinished);
         Task_Blocking_Wait(&smphrFinished);
      }
      else if(block==0) {
         Fifo_Uart_Output_Put(buf, 0);
      }
   }
}
