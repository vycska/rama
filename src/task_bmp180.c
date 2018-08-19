#include "task_bmp180.h"
#include "bmp180.h"
#include "fifos.h"
#include "os.h"
#include "output.h"
#include "utils.h"
#include <string.h>

extern struct BMP180_Data bmp180_data;

struct Task_BMP180_Data task_bmp180_data;

void Task_BMP180(void) {
   char s[24];
   int ut,up,result;

   output("Task_BMP180 has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   Task_Sleep(10); //start-up time after power-up, before first communication

   result = BMP180_ReadParameters();
   if(result==1)
      output("BMP180 parameters ok", eOutputSubsystemBMP180, eOutputLevelDebug, 1);
   else
      output("BMP180 parameters error", eOutputSubsystemBMP180, eOutputLevelDebug, 1);

   if(BMP180_GetID()==0x55)
      output("BMP180 id ok", eOutputSubsystemBMP180, eOutputLevelDebug, 1);
   else
      output("BMP180 id read error", eOutputSubsystemBMP180, eOutputLevelDebug, 1);

   while(1) {
      BMP180_MeasureTemperature();
      Task_Sleep(5);
      ut = BMP180_ReadData(0);

      BMP180_MeasurePressure();
      Task_Sleep(26);
      up = BMP180_ReadData(1);

      if(!(ut==0 && up==0)) {
         BMP180_Calculate(ut,up);
         task_bmp180_data.ready = 1;
         task_bmp180_data.t = bmp180_data.t/10.0;
         task_bmp180_data.p = bmp180_data.p*PA2MMHG;
         mysprintf(s, "bmp180 t: %f2 C", (char *)&task_bmp180_data.t);
         output(s, eOutputSubsystemBMP180, eOutputLevelDebug, 1);
         mysprintf(s, "bmp180 p: %f2 mmHg", (char *)&task_bmp180_data.p);
         output(s, eOutputSubsystemBMP180, eOutputLevelDebug, 1);
      }
      else {
         task_bmp180_data.ready = 0;
         output("BMP180 read error", eOutputSubsystemBMP180, eOutputLevelDebug, 1);
      }

      Task_Sleep(2000);
   }
}
