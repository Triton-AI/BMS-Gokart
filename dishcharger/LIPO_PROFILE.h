//PROFILE LIPO CUSTOM CONFIG
#define CellCount 4   //Battery cells: 2S, 3S, 4S.
#define LIPOCAP 2200.0  //Battery capacity label.
#define CellVoltage 3.7
#define voltNOM (CellCount * CellVoltage)  //volts nominal
#define CAPACITY_SET (LIPOCAP / 1000.00)   //max battery capacity
#define lowVoltageOFF (CellCount * 3.4)   //cutoff low volts
#define highVoltageOFF (CellCount * 4.3)   //cutoff high volts
#define lowAmpsOFF 0.5                     //cutoff low amps
#define highAmpsOFF 15                     //cutoff high amps
String RESULT = "LIPO";