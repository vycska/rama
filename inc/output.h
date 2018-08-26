#ifndef __OUTPUT_H__
#define __OUTPUT_H__

enum eOutputSubsystem {
   eOutputSubsystemADC        = 0,
   eOutputSubsystemAM2320     = 1,
   eOutputSubsystemBMP180     = 2,
   eOutputSubsystemHTU21D     = 3,
   eOutputSubsystemOled       = 4,
   eOutputSubsystemSystem     = 5,
   eOutputSubsystemSwitch     = 6,
   eOutputSubsystemLast       = 7
};

enum eOutputLevel {
   eOutputLevelNone           = 0,
   eOutputLevelDebug          = 1,
   eOutputLevelNormal         = 2,
   eOutputLevelImportant      = 4
};

struct Output_Data {
   unsigned char mask[eOutputSubsystemLast];
};

void output(char*, enum eOutputSubsystem, enum eOutputLevel, int);

#endif
