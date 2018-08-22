#include "htu21d.h"
#include "i2c.h"

int HTU21D_WriteCommand(unsigned char command, unsigned char *d, int l) {
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = HTU21D_SLAVE;
   i2c_data.direction = 0;
   i2c_data.buffer[0] = &command;
   i2c_data.length[0] = 1;
   i2c_data.buffer[1] = d;
   i2c_data.length[1] = l;
   ok = I2C_Transaction(2,&i2c_data);
   return ok;
}

int HTU21D_SoftReset(void) {
   return HTU21D_WriteCommand(0xfe,0,0);
}

unsigned char HTU21D_ReadUserRegister(void) {
   unsigned char d=0;
   HTU21D_WriteCommand(0xe7,0,0);
   HTU21D_ReadData(&d,1);
   return d;
}

int HTU21D_WriteUserRegister(unsigned char d) {
   int result;
   result = HTU21D_WriteCommand(0xe6,&d,1);
   return result;
}

int HTU21D_MeasureTemperature(void) {
   return HTU21D_WriteCommand(0xf3,0,0);
}

int HTU21D_MeasureHumidity(void) {
   return HTU21D_WriteCommand(0xf5,0,0);
}

int HTU21D_ReadData(unsigned char *data,int l) {
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = HTU21D_SLAVE;
   i2c_data.direction = 1;
   i2c_data.buffer[0] = data;
   i2c_data.length[0] = l;
   i2c_data.buffer[1] = 0;
   i2c_data.length[1] = 0;
   ok = I2C_Transaction(2,&i2c_data);
   return ok;
}
