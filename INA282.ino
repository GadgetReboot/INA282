/*******************************************************************************
       This is an example using the INA282 current sense module.
       The module should have REF1 and REF2 grounded so 0A = 0V out

       Gadget Reboot

*******************************************************************************/

const byte     inputPin = A0;                     // sensor input
const byte         gain = 50;                     // INA282 gain = 50 V/V
const float      rSense = 0.1;                    // sense resistor 0.1r
const float sensitivity = rSense * 0.001 * gain;  // sensor output V/mA  = 1mA * 0.1r * 50 = 0.005 V/mA
int              offset = 0;                      // ADC zero current offset adjust


void setup() {
  Serial.begin(9600);

  // assuming no sensor current during power up
  // take an average reading to determine if there is an offset
  // and subtract from future current readings
  for (int i = 0; i < 100; i++) {
    offset =  offset + analogRead(inputPin);
  }
  offset = offset / 100;
}


void loop() {

  // read sensor current and display on serial monitor
  Serial.print(measureCurrent());
  Serial.println(" mA");
  delay(1000);

}


// ---------------
// measure current
// ---------------
int32_t measureCurrent() {

  // variables for taking average measurements
  float sensorValue = 0;
  float Vcc = 0;

  // take the average of 100 samples
  for (int i = 0; i < 100; i++) {
    sensorValue =  sensorValue + analogRead(inputPin);
    Vcc = Vcc + readVcc();
  }
  sensorValue = (sensorValue / 100) - offset;
  Vcc = Vcc / 100;

  // truncate the float variable
  sensorValue = (int)sensorValue;

  // calculate measured current in mA
  // ((VCC / 1024) * sensorValue) = measured mV on 10 bit ADC ranging from 0V to VCC in 1024 steps (1 step =~ 4.88mV)
  // sensitivity = INA282 output volts per mA
  // /1000 because there's volts and mV being used in the calculation
  int32_t currentmA = (((Vcc / 1024L) * sensorValue) / sensitivity) / 1000;

  Serial.print("ADC: ");
  Serial.print(sensorValue);
  Serial.print(" Voltage: ");
  Serial.print(((Vcc / 1024) / 1000)*sensorValue);
  Serial.print("V @Vcc ");
  Serial.print(Vcc);
  Serial.print (" mV ");
  Serial.print ("Current: ");

  return (currentmA);
}

// ---------------------------------------------------
// calculate Vcc in mV from the 1.1V reference voltage
// ---------------------------------------------------
int32_t readVcc() {
  int32_t result = 5000L;

  // read 1.1V reference against AVcc - hardware dependent
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

#if defined(__AVR__)
  delay(2);                         // wait for Vref to settle
  ADCSRA |= _BV(ADSC);              // convert, result is stored in ADC at end
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  result = ADCL;                    // must read ADCL (low byte) first - it then locks ADCH
  result |= ADCH << 8;              // unlocks both
  result = 1125300L / result;       // back-calculate AVcc in mV; 1125300 = 1.1*1023*1000
#endif

  return (result);
}
