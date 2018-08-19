#ifndef __BMP180_H__
#define __BMP180_H__

#define BMP180_SLAVE (0x77)
#define BMP180_OSS (3)

struct BMP180_Data {
   short AC1;          //0xaa, 0xab
   short AC2;          //0xac, 0xad
   short AC3;          //0xae, 0xaf
   unsigned short AC4; //0xb0, 0xb1
   unsigned short AC5; //0xb2, 0xb3
   unsigned short AC6; //0xb4, 0xb5
   short B1;           //0xb6, 0xb7
   short B2;           //0xb8, 0xb9
   short MB;           //0xba, 0xbb
   short MC;           //0xbc, 0xbd
   short MD;           //0xbe, 0xbf
   int p, t;
};

int BMP180_RegisterRead(unsigned int, unsigned char *, int);
int BMP180_RegisterWrite(unsigned int, unsigned char *, int);
int BMP180_ReadParameters(void);
unsigned char BMP180_GetID(void);
int BMP180_MeasureTemperature(void);
int BMP180_MeasurePressure(void);
int BMP180_ReadData(int);
void BMP180_Calculate(int,int);

#endif
