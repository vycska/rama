#ifndef __TASK_SENSORS_H__
#define __TASK_SENSORS_H__

#define PA2MMHG (0.0075006)

struct Task_Sensors_Data {
   int bmp180_ready, htu21d_ready, am2320_ready;
   double bmp180_p, bmp180_t, bmp180_pbase,
          htu21d_h, htu21d_t, htu21d_hcom, htu21d_tdew,
          am2320_h, am2320_t;
};

void Task_Sensors(void);

#endif
