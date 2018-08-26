#include "am2320.h"
#include "i2c.h"

int AM2320_Write(unsigned char *d, int l) {
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = AM2320_SLAVE;
   i2c_data.direction = 0;
   i2c_data.buffer[0] = d;
   i2c_data.length[0] = l;
   i2c_data.buffer[1] = 0;
   i2c_data.length[1] = 0;
   ok = I2C_Transaction(2,&i2c_data);
   return ok;
}

int AM2320_Read(unsigned char *d, int l) {
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = AM2320_SLAVE;
   i2c_data.direction = 1;
   i2c_data.buffer[0] = d;
   i2c_data.length[0] = l;
   i2c_data.buffer[1] = 0;
   i2c_data.length[1] = 0;
   ok = I2C_Transaction(2,&i2c_data);
   return ok;
}

int AM2320_WakeUp(void) {
   return AM2320_Write(0,0);
}

int AM2320_RequestHumidity(void) {
   unsigned char d[3]={0x3,0x0,2};
   return AM2320_Write(d,3);
}

unsigned short AM2320_ReadHumidity(void) {
   unsigned char d[5]; //0x3 + 2 + reg0 + reg1 + CRC
   unsigned short h;
   if(AM2320_Read(d,5) && d[0]==0x3 && d[1]==2)
      h = (d[2]<<8) | d[3];
   else h=0;
   return h;
}

int AM2320_RequestTemperature(void) {
   unsigned char d[3]={0x3,0x2,2};
   return AM2320_Write(d,3);
}

unsigned short AM2320_ReadTemperature(void) {
   unsigned char d[5]; //0x3 + 2 + reg0 + reg1 + CRC
   unsigned short t;
   if(AM2320_Read(d,5) && d[0]==0x3 && d[1]==2)
      t = (d[2]<<8) | d[3];
   else t=0;
   return t;
}

int AM2320_RequestModel(void) {
   unsigned char d[3]={0x3,0x8,2};
   return AM2320_Write(d,3);
}

unsigned short AM2320_ReadModel(void) {
   unsigned char d[5]; //0x3 + 2 + reg0 + reg1 + CRC
   unsigned short m;
   if(AM2320_Read(d,5) && d[0]==0x3 && d[1]==2)
      m = (d[2]<<8) | d[3];
   else m=0xffff;
   return m;
}

int AM2320_RequestVersion(void) {
   unsigned char d[3]={0x3,0xa,1};
   return AM2320_Write(d,3);
}

unsigned char AM2320_ReadVersion(void) {
   unsigned char d[4], //0x3 + 1 + reg0 + CRC
                 v;
   if(AM2320_Read(d,4) && d[0]==0x3 && d[1]==1)
      v=d[2];
   else v=0xff;
   return v;
}

int AM2320_RequestID(void) {
   unsigned char d[3]={0x3,0xb,4};
   return AM2320_Write(d,3);
}

unsigned int AM2320_ReadID(void) {
   unsigned char d[7]; //0x3 + 4 + reg0 + reg1 + reg2 + reg3 + CRC
   unsigned int id;
   if(AM2320_Read(d,7) && d[0]==0x3 && d[1]==4)
      id = (d[2]<<24) | (d[3]<<16) | (d[4]<<8) | (d[5]<<0);
   else id = 0xffffffff;
   return id;
}
