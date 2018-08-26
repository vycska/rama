#ifndef __AM2320_H__
#define __AM2320_H__

#define AM2320_SLAVE (0x5C)

int AM2320_Write(unsigned char *d, int l);
int AM2320_Read(unsigned char *d, int l);
int AM2320_WakeUp(void);
int AM2320_RequestHumidity(void);
unsigned short AM2320_ReadHumidity(void);
int AM2320_RequestTemperature(void);
unsigned short AM2320_ReadTemperature(void);
int AM2320_RequestModel(void);
unsigned short AM2320_ReadModel(void);
int AM2320_RequestVersion(void);
unsigned char AM2320_ReadVersion(void);
int AM2320_RequestID(void);
unsigned int AM2320_ReadID(void);

#endif

