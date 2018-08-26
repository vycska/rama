#include "task_sensors.h"
#include "am2320.h"
#include "bmp180.h"
#include "fifos.h"
#include "htu21d.h"
#include "utils-asm.h"
#include "os.h"
#include "output.h"
#include "utils.h"
#include <math.h>
#include <string.h>

extern struct BMP180_Data bmp180_data;

struct Task_Sensors_Data task_sensors_data;

void Task_Sensors(void) {
   char s[32];
   unsigned char data[2];
   int sensor_ok,
       bmp180_ut, bmp180_up;
   unsigned int value;

   output("Task_Sensors has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   Task_Sleep(15); //start-up time (10 for BMP180, 15 for HTU21D)

   /* *** BMP180 *** */
   sensor_ok = (BMP180_GetID()==0x55);
   mysprintf(s,"BMP180 id: %s",sensor_ok?"ok":"error");
   output(s, eOutputSubsystemBMP180, eOutputLevelNormal, 1);
   BMP180_ReadParameters();

   /* *** HTU21D *** */
   sensor_ok = HTU21D_SoftReset();
   mysprintf(s,"HTU21D soft reset: %s",sensor_ok?"ok":"error");
   output(s, eOutputSubsystemHTU21D, eOutputLevelNormal, 1);

   Task_Sleep(15);

   data[0] = HTU21D_ReadUserRegister(); //b0,7: measurement resolution, b1: disable OTP reload, b2: enable on-chip heater, b3,4,5: reserved, b6: end of battery
   data[0] = data[0] & (~((1<<0) | (1<<7))); //set resolution: 12b RH, 14b Temp
   sensor_ok = HTU21D_WriteUserRegister(data[0]);
   mysprintf(s,"HTU21D user register [%x]: %s",(unsigned int)data[0],sensor_ok?"ok":"error");
   output(s,eOutputSubsystemHTU21D,eOutputLevelNormal,1);

   /* *** AM2320 *** */
   AM2320_WakeUp();
   Task_Sleep(15);
   AM2320_RequestModel();
   value = AM2320_ReadModel();
   mysprintf(s,"AM2320 model: 0x%x",value);
   output(s, eOutputSubsystemAM2320, eOutputLevelNormal, 1);
   AM2320_RequestVersion();
   value = AM2320_ReadVersion();
   mysprintf(s,"AM2320 version: 0x%x",value);
   output(s, eOutputSubsystemAM2320, eOutputLevelNormal, 1);
   AM2320_RequestID();
   value = AM2320_ReadID();
   mysprintf(s,"AM2320 ID: 0x%x",value);
   output(s, eOutputSubsystemAM2320, eOutputLevelNormal, 1);

   while(1) {
      /* *** BMP180 *** */
      BMP180_MeasureTemperature();
      Task_Sleep(5);
      bmp180_ut = BMP180_ReadData(0);

      BMP180_MeasurePressure();
      Task_Sleep(26);
      bmp180_up = BMP180_ReadData(1);

      if(!(bmp180_ut==0 && bmp180_up==0)) {
         BMP180_Calculate(bmp180_ut,bmp180_up);
         task_sensors_data.bmp180_t = bmp180_data.t/10.0;
         task_sensors_data.bmp180_p = bmp180_data.p*PA2MMHG;
         if(task_sensors_data.bmp180_pbase==0) task_sensors_data.bmp180_pbase=task_sensors_data.bmp180_p;
         task_sensors_data.bmp180_ready = 1;
         mysprintf(s, "bmp180 t: %f2 C", (char *)&task_sensors_data.bmp180_t);
         output(s, eOutputSubsystemBMP180, eOutputLevelDebug, 1);
         mysprintf(s, "bmp180 p: %f2 mmHg", (char *)&task_sensors_data.bmp180_p);
         output(s, eOutputSubsystemBMP180, eOutputLevelDebug, 1);
      }
      else {
         task_sensors_data.bmp180_ready = 0;
         output("BMP180 read error", eOutputSubsystemBMP180, eOutputLevelDebug, 1);
      }

      /* *** HTU21D *** */
      sensor_ok = HTU21D_MeasureTemperature();
      Task_Sleep(50);
      sensor_ok = HTU21D_ReadData(data,2) && sensor_ok;
      value = (data[0]<<8) | data[1];
      sensor_ok = ((value&(1<<1))==0) && sensor_ok; //b1 is status bit -- 0 for temperature
      value &= (~(3<<0)); //status bits must be set to 0
      task_sensors_data.htu21d_t = -46.85+175.72*value/((double)(1<<16));

      sensor_ok = HTU21D_MeasureHumidity() && sensor_ok;
      Task_Sleep(16); //measuring time
      sensor_ok = HTU21D_ReadData(data,2) && sensor_ok;
      value = (data[0]<<8) | data[1];
      sensor_ok = ((value&(1<<1))!=0) && sensor_ok; //b1 is status bit -- 1 for humidity
      value &= (~(3<<0)); //status bits must be set to 0
      task_sensors_data.htu21d_h = -6.0+125.0*value/((double)(1<<16));

      if(sensor_ok) {
         task_sensors_data.htu21d_hcom = task_sensors_data.htu21d_h + (25.0-task_sensors_data.htu21d_t)*(-0.1);
         task_sensors_data.htu21d_tdew = -(1762.39/(log(task_sensors_data.htu21d_hcom)/2.302585093+(8.1332-1762.39/(task_sensors_data.htu21d_t+235.66))-2.0-8.1332)+235.66);
         task_sensors_data.htu21d_ready = 1;
         mysprintf(s,"HTU21D t: %f1",(char*)&task_sensors_data.htu21d_t);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
         mysprintf(s,"HTU21D h: %f1",(char*)&task_sensors_data.htu21d_h);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
         mysprintf(s,"HTU21D h_com: %f1",(char*)&task_sensors_data.htu21d_hcom);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
         mysprintf(s,"HTU21D t_dew: %f1",(char*)&task_sensors_data.htu21d_tdew);
         output(s,eOutputSubsystemHTU21D,eOutputLevelDebug,1);
      }
      else task_sensors_data.htu21d_ready = 0;

      /* *** AM2320 *** */
      AM2320_WakeUp();
      Task_Sleep(15);
      AM2320_RequestHumidity();
      value = AM2320_ReadHumidity();
      task_sensors_data.am2320_h = value/10.0;

      AM2320_RequestTemperature();
      value = AM2320_ReadTemperature();
      if((value&(1u<<15))==0)
         task_sensors_data.am2320_t = value/10.0;
      else {
         value&=(~(1u<<15));
         task_sensors_data.am2320_t = -value/10.0;
      }
      if(!(task_sensors_data.am2320_h==0 && task_sensors_data.am2320_t==0)) {
         task_sensors_data.am2320_ready = 1;
         mysprintf(s,"AM2320 t: %f1",(char*)&task_sensors_data.am2320_t);
         output(s,eOutputSubsystemAM2320,eOutputLevelDebug,1);
         mysprintf(s,"AM2320 h: %f1",(char*)&task_sensors_data.am2320_h);
         output(s,eOutputSubsystemAM2320,eOutputLevelDebug,1);
      }
      else task_sensors_data.am2320_ready = 0;

      Task_Sleep(2000);
   }
}
