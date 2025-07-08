//ADC calibration magic numbers
#define LPF 150.0
#define ZERO_Volt 0.0
#define TRUE3V3 3.3
#define REF3V3 3.256//UPDATE
#define ADC_12BIT 4095
#define VOLT_3V3_RATIO (REF3V3 / ADC_12BIT)  // Convert the analog reading (which goes from 0 - 4095) to a voltage (0 - 3.3V):
#define BAT_TYPE_12V 0.0165 //adjusted 12V LFP
#define BAT_TYPE_24V 0.027
#define BAT_TYPE_48V 0.014
float OFFSET_VBAT = BAT_TYPE_24V;
#define THRESHOLD_12V 16
#define THRESHOLD_24V 32
#define THRESHOLD_48V 36
String BATTERY = "";
#define MAX_ADC3V3 2.4745
#define MAX_VBAT 55.0
#define HIGH_VOLT_RATIO (MAX_VBAT / MAX_ADC3V3)
#define DEFAULT_VREF 1100  // Default VREF in mV
esp_adc_cal_characteristics_t adc_chars;
#define OFFSET_AMP 0.0135
float ZERO_Amp = 0.318;  //Volts
#define SENSE 0.0457     //adjusted 12V LFP // Volts per Amp