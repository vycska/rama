#include "task_oled.h"
#include "adc.h"
#include "fifos.h"
#include "i2c.h"
#include "main.h"
#include "mrt.h"
#include "os.h"
#include "output.h"
#include "switch.h"
#include "task_sensors.h"
#include "utils-asm.h"
#include "utils.h"
#include "lpc824.h"
#include <math.h>

extern volatile long long int millis;
extern struct tcb *RunPt;
extern volatile struct ADC_Data adc_data;
extern volatile struct Switch_Data switch_data;
extern struct Task_Sensors_Data task_sensors_data;

u8log_t u8log;
u8g2_t u8g2;
struct Task_Oled_Data task_oled_data;

void Task_Oled(void) {
   char buf[32],buf2[16];
   int val_int;
   double val_double;

   output("Task_Oled has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   u8g2_InitDisplay(&u8g2); //send init sequence to the display, display is in sleep mode
   u8g2_SetPowerSave(&u8g2,0); //wake up display
   u8g2_ClearBuffer(&u8g2);

   while(1) {
      if(task_oled_data.log_enabled) {
         u8g2_FirstPage(&u8g2);
         do {
            u8g2_DrawLog(&u8g2, 0, u8g2_GetAscent(&u8g2), &u8log);
         } while(u8g2_NextPage(&u8g2));
      }
      else {
         switch(task_oled_data.screen) {
            default:
               task_oled_data.screen = 1;
            case 1:
               mysprintf(buf,"BMP180: temper., %s","\xb0""C");
               if(task_sensors_data.bmp180_ready) {
                  val_double = task_sensors_data.bmp180_t+0.05;
                  mysprintf(buf2,"%f1",(char*)&val_double);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 2:
               mysprintf(buf,"HTU21D: temper., %s","\xb0""C");
               if(task_sensors_data.htu21d_ready) {
                  val_double = task_sensors_data.htu21d_t+0.05;
                  mysprintf(buf2,"%f1",(char*)&val_double);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 3:
               mysprintf(buf,"AM2320: temper., %s","\xb0""C");
               if(task_sensors_data.am2320_ready) {
                  val_double = task_sensors_data.am2320_t+0.05;
                  mysprintf(buf2,"%f1",(char*)&val_double);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 4:
               mysprintf(buf,"HTU21D: humidity, %%");
               if(task_sensors_data.htu21d_ready) {
                  val_int = task_sensors_data.htu21d_hcom+0.5;
                  mysprintf(buf2,"%d",val_int);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 5:
               mysprintf(buf,"AM2320: humidity, %%");
               if(task_sensors_data.am2320_ready) {
                  val_int = task_sensors_data.am2320_h+0.5;
                  mysprintf(buf2,"%d",val_int);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 6:
               mysprintf(buf,"BMP180: pressure, mmHg");
               if(task_sensors_data.bmp180_ready) {
                  val_int = task_sensors_data.bmp180_p+0.5;
                  mysprintf(buf2,"%d",val_int);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 7:
               mysprintf(buf,"HTU21D: dew point, %s","\xb0""C");
               if(task_sensors_data.htu21d_ready) {
                  val_double = task_sensors_data.htu21d_tdew+0.05;
                  mysprintf(buf2,"%f1",(char*)&val_double);
               }
               else
                  mysprintf(buf2,"**.*");
               break;
            case 8:
               mysprintf(buf, "Delta z, m");
               if(task_sensors_data.bmp180_pbase!=0) {
                  val_double = 287.053 / 9.8 * (273.15+task_sensors_data.bmp180_t) * log(task_sensors_data.bmp180_pbase/task_sensors_data.bmp180_p);
                  val_int = (val_double<0.0); //ar reiksme neigiama [viso sito reikia, kad pries skaiciu butu - +]
                  if(val_int) val_double = -val_double;
                  val_double += 0.05; //suapvalinam, nes mysprintf() nukerta
                  mysprintf(buf2,"%s%f1",val_int?"-":"+",(char*)&val_double);
               }
               else mysprintf(buf2,"**.*");
               break;
            case 9: //adc
               mysprintf(buf,"Li-Ion battery, V");
               val_double = ((double)adc_data.sum/adc_data.count)/4095.0*3.3 * 2;
               mysprintf(buf2,"%f2",(char*)&val_double);
               break;
         }
         if(switch_data.active)
            switch((millis-switch_data.start)/2000) {
               case 1:
                  mysprintf(buf,"p_base");
                  break;
               case 2:
                  mysprintf(buf,"LED");
                  break;
               case 3:
                  mysprintf(buf,"config");
                  break;
               case 4:
                  mysprintf(buf,"UART");
                  break;
               case 5:
                  mysprintf(buf,"log");
                  break;
            }
         u8g2_ClearBuffer(&u8g2);

         u8g2_SetFont(&u8g2,u8g2_font_6x10_tf);
         u8g2_DrawStr(&u8g2,MAX2(0,(128-u8g2_GetStrWidth(&u8g2,buf))/2),10,buf);

         u8g2_SetFont(&u8g2,u8g2_font_fur35_tn);
         u8g2_DrawStr(&u8g2,MAX2(0,(128-u8g2_GetStrWidth(&u8g2,buf2))/2),58,buf2);

         u8g2_SendBuffer(&u8g2);
      }

      Task_Sleep(500);
   }
}

uint8_t u8x8_gpio_and_delay_sw(u8x8_t *u8x8,uint8_t msg,uint8_t arg_int,void *arg_ptr) {
   switch(msg) {
      case U8X8_MSG_GPIO_AND_DELAY_INIT: //called once during init phase of u8g2; can be used to setup pins
         PINENABLE0 |= (1<<22 | 1<<23); //ADC_9 disabled on PIO0_17, ADC_10 disabled on PIO0_13
         //SDA is PIO0_17
         PIO0_17 = (0<<3 | 0<<5 | 0<<6 | 1<<10 | 0<<11 | 0<<13); //no pd/pu, hysteresis disable, input not inverted, open drain mode, bypass input filter, IOCONCLKDIV0
         //SCL is PIO0_13
         PIO0_13 = (0<<3 | 0<<5 | 0<<6 | 1<<10 | 0<<11 | 0<<13); //no pd/pu, hysteresis disable, input not inverted, open drain mode, bypass input filter, IOCONCLKDIV0
         //direction is output
         DIR0 |= (1<<13 | 1<<17);
         //initially released
         SET0 = (1<<13 | 1<<17);
         break;
      case U8X8_MSG_DELAY_100NANO: //delay arg_int*100 nano seconds
         MRT1_Delay(arg_int*100);
         break;
      case U8X8_MSG_DELAY_10MICRO: //delay arg_int * 10 micro seconds
         MRT1_Delay(arg_int*10*1000);
         break;
      case U8X8_MSG_DELAY_MILLI: //delay arg_int*1 milli second
         MRT1_Delay(arg_int*1000*1000);
         break;
      case U8X8_MSG_DELAY_I2C: //arg_int is the I2C speed in 100 kHz, e.g. 4=400 Hz; arg_int=1: delay by 5 us, arg_int=4: delay by 1.25 us
         MRT1_Delay(5.0/arg_int*1000);
         break;
      case U8X8_MSG_GPIO_I2C_CLOCK: //arg_int=0: output low at I2C clock pin; arg_int=1: input dir with pullup high for I2C clock pin
         if(arg_int==0) CLR0 = (1<<13);
         else SET0 = (1<<13);
         break;
      case U8X8_MSG_GPIO_I2C_DATA: //arg_int=0: output low at I2C data pin; arg_int=1: input dir with pullup high for I2C data pin
         if(arg_int==0) CLR0 = (1<<17);
         else SET0 = (1<<17);
         break;
      default:
         u8x8_SetGPIOResult(u8x8,1); //default return value
         break;
   }
   return 1;
}

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
   static unsigned char buf_idx,
                        buffer[32]; //u8g2/u8x8 will never send more than 32B between START_TRANSFER and END_TRANSFER
   static struct I2C_Data i2c_data;
   unsigned char *data;

   switch(msg) {
      case U8X8_MSG_BYTE_INIT: //send once during the init phase of the display
         I2C1_Init();
         i2c_data.slave = u8x8_GetI2CAddress(u8x8)>>1;
         i2c_data.direction = 0;
         i2c_data.buffer[0] = buffer;
         i2c_data.length[1] = 0;
         i2c_data.buffer[1] = NULL;
         break;
      case U8X8_MSG_BYTE_SET_DC: //ignored for i2c
         break;
      case U8X8_MSG_BYTE_START_TRANSFER:
         i2c_data.length[0] = buf_idx = 0;
         break;
      case U8X8_MSG_BYTE_SEND: //send one or more bytes
         i2c_data.length[0] += arg_int;
         for(data=(unsigned char*)arg_ptr; arg_int>0; arg_int--)
            buffer[buf_idx++] = *data++;
         break;
      case U8X8_MSG_BYTE_END_TRANSFER:
         I2C_Transaction(1,&i2c_data);
         break;
      default:
         return 0;
   }
   return 1;
}

uint8_t u8x8_gpio_and_delay_hw(u8x8_t *u8x8,uint8_t msg,uint8_t arg_int,void *arg_ptr) {
   switch(msg) {
      case U8X8_MSG_GPIO_AND_DELAY_INIT: //called once during init phase of u8g2; can be used to setup pins
         break;
      case U8X8_MSG_DELAY_MILLI: //delay arg_int*1 milli second
         MRT1_Delay(arg_int*1000*1000);
         break;
      default:
         u8x8_SetGPIOResult(u8x8,1); //default return value
         break;
   }
   return 1;
}
