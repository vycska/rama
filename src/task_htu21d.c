#include "task_htu21d.h"
#include "fifos.h"
#include "htu21d.h"
#include "os.h"
#include "output.h"
#include "utils.h"
#include <math.h>
#include <string.h>

struct Task_HTU21D_Data task_htu21d_data;

void Task_HTU21D(void) {
   char s[32];
   unsigned char data[2];
   unsigned short value;
   int sensor_ok;

   output("Task_HTU21D has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   Task_Sleep(15); //time needed for reaching idle state, when commands can be accepted

   sensor_ok = HTU21D_SoftReset();
   mysprintf(s,"HTU21D soft reset: %s",sensor_ok?"ok":"error");
   output(s,eOutputSubsystemHTU21D,eOutputLevelNormal,1);

   Task_Sleep(15);

   data[0] = HTU21D_ReadUserRegister(); //b0,7: measurement resolution, b1: disable OTP reload, b2: enable on-chip heater, b3,4,5: reserved, b6: end of battery
   data[0] = data[0] & (~((1<<0) | (1<<7))); //set resolution: 12b RH, 14b Temp
   sensor_ok = HTU21D_WriteUserRegister(data[0]);
   mysprintf(s,"HTU21D user register [%x]: %s",(unsigned int)data[0],sensor_ok?"ok":"error");
   output(s,eOutputSubsystemHTU21D,eOutputLevelNormal,1);

   while(1) {
      sensor_ok = HTU21D_MeasureTemperature();
      Task_Sleep(50);
      sensor_ok = HTU21D_ReadData(data,2) && sensor_ok;
      value = (data[0]<<8) | data[1];
      sensor_ok = ((value&(1<<1))==0) && sensor_ok; //b1 is status bit -- 0 for temperature
      value &= (~(3<<0)); //status bits must be set to 0
      task_htu21d_data.t = -46.85+175.72*value/((double)(1<<16));

      sensor_ok = HTU21D_MeasureHumidity() && sensor_ok;
      Task_Sleep(16); //measuring time
      sensor_ok = HTU21D_ReadData(data,2) && sensor_ok;
      value = (data[0]<<8) | data[1];
      sensor_ok = ((value&(1<<1))==(1<<1)) && sensor_ok; //b1 is status bit -- 1 for humidity
      value &= (~(3<<0)); //status bits must be set to 0
      task_htu21d_data.h = -6.0+125.0*value/((double)(1<<16));

      if(sensor_ok) {
         task_htu21d_data.h_com = task_htu21d_data.h + (25.0-task_htu21d_data.t)*(-0.1);
         task_htu21d_data.t_dew = -(1762.39/(log(task_htu21d_data.h)/2.302585093+(8.1332-1762.39/(task_htu21d_data.t+235.66))-2.0-8.1332)+235.66);
         task_htu21d_data.ready = 1;
         mysprintf(s,"HTU21D t: %f1",(char*)&task_htu21d_data.t);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
         mysprintf(s,"HTU21D h: %f1",(char*)&task_htu21d_data.h);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
         mysprintf(s,"HTU21D h_com: %f1",(char*)&task_htu21d_data.h_com);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
         mysprintf(s,"HTU21D t_dew: %f1",(char*)&task_htu21d_data.t_dew);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
      }
      else task_htu21d_data.ready = 0;

      Task_Sleep(2000);
   }
}
