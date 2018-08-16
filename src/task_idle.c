#include "task_idle.h"
#include "main.h"
#include "utils-asm.h"

void Task_Idle(void) {
   while(1) {
#if BOARD == BOARD_RELEASE
      WaitForInterrupt();
#endif
   }
}
