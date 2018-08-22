#ifndef __TASK_HTU21D_H__
#define __TASK_HTU21D_H__

struct Task_HTU21D_Data {
   int ready;
   double h, t,
          h_com, t_dew;
};

void Task_HTU21D(void);

#endif
