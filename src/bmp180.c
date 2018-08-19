#include "bmp180.h"
#include "fifos.h"
#include "i2c.h"
#include "os.h"
#include "utils.h"
#include "lpc824.h"

struct BMP180_Data bmp180_data;

int BMP180_RegisterRead(unsigned int r, unsigned char *d, int l) {
   unsigned char reg[1];
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = BMP180_SLAVE;
   i2c_data.direction = 2; //write then read
   reg[0] = r;
   i2c_data.buffer[0] = reg;
   i2c_data.length[0] = 1;
   i2c_data.buffer[1] = d;
   i2c_data.length[1] = l;
   ok = I2C_Transaction(0,&i2c_data);
   return ok;
}

int BMP180_RegisterWrite(unsigned int r, unsigned char *d, int l) {
   unsigned char reg[1];
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = BMP180_SLAVE;
   i2c_data.direction = 0;
   reg[0] = r;
   i2c_data.buffer[0] = reg;
   i2c_data.length[0] = 1;
   i2c_data.buffer[1] = d;
   i2c_data.length[1] = l;
   ok = I2C_Transaction(0,&i2c_data);
   return ok;
}

int BMP180_ReadParameters(void) {
   unsigned char data[2] = {0};
   int result = 1;

   result = result && BMP180_RegisterRead(0xaa, data, 2);
   bmp180_data.AC1 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xac, data, 2);
   bmp180_data.AC2 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xae, data, 2);
   bmp180_data.AC3 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xb0, data, 2);
   bmp180_data.AC4 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xb2, data, 2);
   bmp180_data.AC5 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xb4, data, 2);
   bmp180_data.AC6 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xb6, data, 2);
   bmp180_data.B1 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xb8, data, 2);
   bmp180_data.B2 = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xba, data, 2);
   bmp180_data.MB = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xbc, data, 2);
   bmp180_data.MC = (data[0] << 8) | data[1];

   result = result && BMP180_RegisterRead(0xbe, data, 2);
   bmp180_data.MD = (data[0] << 8) | data[1];

   result = result &&
      bmp180_data.AC1!=0 && bmp180_data.AC1!=0xffff
      &&
      bmp180_data.AC2!=0 && bmp180_data.AC2!=0xffff
      &&
      bmp180_data.AC3!=0 && bmp180_data.AC3!=0xffff
      &&
      bmp180_data.AC4!=0 && bmp180_data.AC4!=0xffff
      &&
      bmp180_data.AC5!=0 && bmp180_data.AC5!=0xffff
      &&
      bmp180_data.AC6!=0 && bmp180_data.AC6!=0xffff
      &&
      bmp180_data.B1!=0 && bmp180_data.B1!=0xffff
      &&
      bmp180_data.B2!=0 && bmp180_data.B2!=0xffff
      &&
      bmp180_data.MB!=0 && bmp180_data.MB!=0xffff
      &&
      bmp180_data.MC!=0 && bmp180_data.MC!=0xffff
      &&
      bmp180_data.MD!=0 && bmp180_data.MD!=0xffff;

   return result;
}

unsigned char BMP180_GetID(void) {
   unsigned char id=0;
   BMP180_RegisterRead(0xd0, &id, 1);
   return id;
}

int BMP180_MeasureTemperature(void) {
   unsigned char d=0x2e;
   int result;
   result = BMP180_RegisterWrite(0xf4,&d,1);
   return result;
}

int BMP180_MeasurePressure(void) {
   unsigned char d=(BMP180_OSS<<6) | (1<<5) | (0x14<<0);
   int result;
   result = BMP180_RegisterWrite(0xf4,&d,1);
   return result;
}

int BMP180_ReadData(int k) {
   unsigned char data[3] = {0};
   int result;

   result = BMP180_RegisterRead(0xf6, data, 3);

   return result ? (k==0 ? ((data[0]<<8) | (data[1]<<0)) : (((data[0]<<16) | (data[1]<<8) | (data[2]<<0)) >> (8-BMP180_OSS))) : 0;
}

void BMP180_Calculate(int ut, int up) {
   int X1,X2,X3,B3,B5,B6;
   unsigned int B4,B7;

   X1 = ((ut-bmp180_data.AC6)*bmp180_data.AC5)>>15;
   X2 = (bmp180_data.MC<<11)/(X1+bmp180_data.MD);
   B5 = X1+X2;
   bmp180_data.t = (B5+8)>>4;

   B6 = B5-4000;
   X1 = (bmp180_data.B2*((B6*B6)>>12))>>11;
   X2 = (bmp180_data.AC2*B6)>>11;
   X3 = X1+X2;
   B3 = ((((int)bmp180_data.AC1*4+X3)<<BMP180_OSS)+2)>>2;
   X1 = (bmp180_data.AC3*B6)>>13;
   X2 = (bmp180_data.B1*((B6*B6)>>12))>>16;
   X3 = (X1+X2+2)>>2;
   B4 = (bmp180_data.AC4*(unsigned int)(X3+32768))>>15;
   B7 = ((unsigned int)up-B3)*(50000>>BMP180_OSS);
   bmp180_data.p = B7<0x80000000 ? (B7*2/B4) : (B7/B4*2);
   X1 = (bmp180_data.p>>8)*(bmp180_data.p>>8);
   X1 = (X1*3038)>>16;
   X2 = (-7357*bmp180_data.p)>>16;
   bmp180_data.p = bmp180_data.p + ((X1+X2+3791)>>4);
}
