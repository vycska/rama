#ifndef __TASK_BMP180_H__
#define __TASK_BMP180_H__

#define PA2MMHG (0.0075006)

struct Task_BMP180_Data {
   int ready;
   double p, t,
          p_base;
};

void Task_BMP180(void);

#endif
