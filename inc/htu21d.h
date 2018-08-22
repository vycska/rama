#ifndef __HTU21D_H__
#define __HTU21D_H__

#define HTU21D_SLAVE (0x40)

int HTU21D_WriteCommand(unsigned char,unsigned char*,int);
int HTU21D_SoftReset(void);
unsigned char HTU21D_ReadUserRegister(void);
int HTU21D_WriteUserRegister(unsigned char);
int HTU21D_MeasureTemperature(void);
int HTU21D_MeasureHumidity(void);
int HTU21D_ReadData(unsigned char*,int);

#endif
