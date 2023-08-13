#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>  
#include <Adafruit_GFX.h>  
/*
Here we including Library ADS1X15 which we can be used for both ADS1015 and ADS1115
ADS1115 module is an analog to digital converter module that has 16-bit precision and can measure a maximum voltage of 7 volts.
The module has 4 analog input channels and can be used to measure a wide range of voltages from 0 to 7 volts.
It uses I2C communication protocol, so it has a high speed and occupies a small number of the microcontroller pins.
*/

/*
<Wire.h> is a library that allows you to communicate with I2C / TWI devices.
<Adafruit_SSD1306.h> is a library that allows you to communicate with the OLED display.
<Adafruit_GFX.h> is a library that allows you to draw graphics on the OLED display.
*/

//Create an ADS1115 ADC (16-bit) instance and SSD1306 OLED display instance
Adafruit_ADS1115 ads;
Adafruit_SSD1306 display;

// Defining the voltage multiplier and voltage variable to store the voltage value
// ads.setGain(GAIN_TWOTHIRDS);  +/- 6.144V  1 bit = 0.1875mV (default) (voltage multiplier = 0.1875)

const float multiplier = 0.1875F;
float Current_sensor_Resolution = 0.100;


//Variables for voltage mode
float Voltage = 0.0;
float resistance_voltage = 0.0;
float battery_voltage = 0.0;
float measured_resistance = 0.0;

//for current mode
const int averageValue = 50;
long int sensorValue = 0;  // variable to store the sensor value read
float sensor_voltage = 0;
float current = 0;

//const int IN_PIN = A0;
int top_but = 2;
int mid_but = 3;
int bot_but = 4;

//inputs/outputs PUT THE PINS AS IN THE SCHEMATIC
int res_2k = 6;
int res_20k = 7;
int res_470k = 8;

#define cap_in_analogPin     0          
#define chargePin           13        
#define dischargePin        11

const int OUT_PIN = A1;

//Modes variables
int mode = 0;
int res_scale = 0;
int cap_scale = 0;
bool mid_but_state = true;
bool top_but_state = true;


//Variables for resistance mode
float Res_2k_value = 2140;         //2K resistor          //CHANGE THIS VALUES. MEASURE YOUT 2k, 20k and 470K and put the real values here
float Res_20k_value = 21.60;      //20K resistor
float Res_470k_value = 0.4655;    //470K resistor


//Variables for big capacitance mode
#define resistorValue  9760.0F  //Remember, we've used a 10K resistor to charge the capacitor MEASURE YOUR VALUE!!!
unsigned long startTime;
unsigned long elapsedTime;
float microFarads;                
float nanoFarads;
const float IN_STRAY_CAP_TO_GND = 47.48;
const float IN_CAP_TO_GND  = IN_STRAY_CAP_TO_GND;
const float R_PULLUP = 34.8;  
const int MAX_ADC_VALUE = 1023;



//Code which runs once when the board is powered on
void setup(void) 
{
  pinMode(top_but,INPUT_PULLUP);
  pinMode(mid_but,INPUT_PULLUP);
  pinMode(bot_but,INPUT_PULLUP);
  pinMode(res_2k,OUTPUT);
  pinMode(res_20k,INPUT);
  pinMode(res_470k,INPUT);
  pinMode(cap_in_analogPin, OUTPUT); 
  digitalWrite(res_2k,LOW);

  pinMode(chargePin, OUTPUT);     
  digitalWrite(chargePin, LOW); 

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  delay(100);  
  display.clearDisplay();
  
  //Print the title on the OLED display
  display.setTextSize(1);
  display.setTextColor(WHITE); 
  display.setCursor(25,0);
  display.print("DIGITAL");  
  display.setCursor(30,20);    
  display.print("MULTIMETER");

  display.display();
  delay(1000);
  display.clearDisplay();
  //This tells the Arduino to get ready to exchange messages with the Serial Monitor at a data rate of 9600 bits per second
  Serial.begin(9600); 
  //Start the ADS1115 ADC
  ads.begin();
}

//Code which runs repeatedly after setup()
void loop(void) 
{
  
  //For push buttons   
  if(!digitalRead(mid_but) && mid_but_state)
  {
    mode = mode + 1;
    res_scale = 0;
    cap_scale = 0;
    mid_but_state = false;
    if(mode > 4)
    {
      mode=0;
    }
    delay(100);
  }

  if(digitalRead(mid_but) && !mid_but_state)
  {
    mid_but_state = true;
  }


  if(!digitalRead(top_but) && top_but_state)
  {
    res_scale = res_scale + 1;
    cap_scale = cap_scale + 1;

    top_but_state = false;
    if(res_scale > 2)
    {
      res_scale=0;
    }
    if(cap_scale > 1)
    {
      cap_scale=0;
    }
     startTime = micros();           //Reset the counters      
     elapsedTime= micros() - startTime;
    delay(100);
    
  }

  if(digitalRead(top_but) && !top_but_state)
  {
    top_but_state = true;
  }

  //end of push buttons


  //current mode

  if ( mode == 2 )
  {
    
  for (int i = 0; i < averageValue; i++)
  {
    //Using A3 pin to read the voltage for current measurement
    sensorValue += analogRead(A2);

    // wait 2 milliseconds before the next loop
    delay(5);
  }
  
  sensorValue = sensorValue / averageValue;
  sensor_voltage = sensorValue * 5.0 / 1024.0;
  current = ((sensor_voltage - 2.5) / 0.185)/2;
  if (current <=0.12)
  {
    current = 0.00;
  }
  // least im getting 0.21 or o.24

  //Print the voltage value on the serial monitor
  Serial.print("   ADC Voltage: ");
  Serial.print(sensor_voltage);
  Serial.print("V");
  Serial.print("   Current: ");
  Serial.print(current);
  Serial.println(" Amps");
        display.clearDisplay(); 
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(5,0);
        display.print("Current");
        display.setTextSize(2);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(15,17);
        display.print(current,2);
        display.print(" A");
        display.display();
        delay(100); 

  }


  //Voltage mode

  if (mode == 3)

  {
    int16_t adc0;  
  
  //Read the voltage value from the analog input channel 0
  adc0 = ads.readADC_SingleEnded(0);
  
  //Calculate the voltage value
  Voltage = (adc0 * multiplier);

  //By default the voltage value is in millivolts, so we divide it by 1000 to get the voltage in volts
  Voltage = (Voltage / 1000);
  if (Voltage>0)
  {
    //Using voltage divider to get the voltage value ( in range 0 to 20 volts)
    Voltage = (Voltage / 0.2440); //R1 = 6.67 ohm    R2 = 2,000 ohm 
    Voltage = Voltage +  0.5; //Diode drop
  }
  //For battery voltage measurement (on ADC2)
    int16_t adc2; // Leemos el ADC, con 16 bits   
    adc2 = ads.readADC_SingleEnded(2);
    battery_voltage = ((adc2 * multiplier)/1000);
    
    //For Displaying battery voltage (on ADC2)
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(BLACK,WHITE); 
    display.setCursor(95,0);    
    display.print(battery_voltage,1); 
    display.print("V");


  //  //Print the voltage value on the serial monitor
  //  Serial.print(Voltage);
  //  Serial.println(" volts"); 
  
   //Print the voltage value on the OLED display
   if(Voltage > 0)
    {    
      display.setTextSize(2);
      display.setTextColor(BLACK,WHITE); 
      display.setCursor(0,4);
      display.print(Voltage); 
      //Serial printing
      Serial.print(Voltage);
      Serial.println(" volts"); 
      display.print(" V"); 
      display.display();
      delay(100);
    }
    else
    {    
      display.setTextSize(2);
      display.setTextColor(BLACK,WHITE); 
      display.setCursor(0,4);
      display.print("0.00"); 
      display.print(" V");      
      //Serial printing
      Serial.print(Voltage);
      Serial.println(" volts");
      display.display();
      delay(100);
    }
  //Wait for 3 seconds to get the next reading
  //delay(3000);  //3 Seconds delay
  display.clearDisplay();
  }

  //Resistance mode

  if (mode == 1)
  
  {
    
    if(res_scale == 0)
    {
      pinMode(res_2k,OUTPUT);
      //pinMode(res_20k,INPUT);
      //pinMode(res_470k,INPUT);     
      digitalWrite(res_2k,LOW);
  
      int16_t adc1; // Leemos el ADC, con 16 bits   
      adc1 = ads.readADC_SingleEnded(1);
      resistance_voltage = (adc1 * multiplier)/1000;
      int16_t adc2; // Leemos el ADC, con 16 bits   

      adc2 = ads.readADC_SingleEnded(2);
      battery_voltage = ((adc2 * multiplier)/1000);
      //battery_voltage =  battery_voltage + 0.38;
      measured_resistance = (Res_2k_value ) * ( (battery_voltage/resistance_voltage)-1  );
      //measured_resistance = ( Res_2k_value ) * (battery_voltage - resistance_voltage - )/(resistance_voltage - 0.3 );
      measured_resistance = measured_resistance -  ( 0.01 * measured_resistance);
      // Serial.println(resistance_voltage);
      // Serial.println(battery_voltage);
      // Serial.println(measured_resistance);
      if(measured_resistance <4000)
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("Ohms");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1);
        display.print("V");
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(measured_resistance,5);
        //Printing on serial monitor
        Serial.print(measured_resistance);
        Serial.println(" ohms"); 

        display.display();
        delay(100);        
      }
      else
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("Ohms");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V");
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(">4000");
        //Printing on serial monitor
        Serial.println(">4000");
        display.display();          
        delay(100);
      }
    }
    if(res_scale == 1)
    {
      //pinMode(res_2k,INPUT);
      pinMode(res_20k,OUTPUT);
      //pinMode(res_470k,INPUT);     
      digitalWrite(res_20k,LOW);
      int16_t adc1; // Leemos el ADC, con 16 bits   
     adc1 = ads.readADC_SingleEnded(1);
      resistance_voltage = (adc1 * 0.1875)/1000;    
      //Serial.println(resistance_voltage);
      int16_t adc2; // Leemos el ADC, con 16 bits   
      adc2 = ads.readADC_SingleEnded(2);
      battery_voltage = ((adc2 * 0.1875)/1000); 
      //battery_voltage = battery_voltage + 0.54;    
      measured_resistance = (Res_20k_value) * (  ( battery_voltage/resistance_voltage)-1  );
      //measured_resistance = (10*Res_20k_value) * (battery_voltage - resistance_voltage )/(resistance_voltage - 0.54);
      measured_resistance = measured_resistance -  ( 0.02 * measured_resistance);
      // Serial.println(resistance_voltage);
      // Serial.println(battery_voltage);
      // Serial.println(measured_resistance);
      if(measured_resistance < 200)
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("Kohms");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V");
        //Printing on serial monitor
        Serial.print(measured_resistance);
        Serial.println(" Kohms");
        
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(measured_resistance,2);  
        display.display();          
        delay(100);        
      }
      else
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("Kohms");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V"); 
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(">200K"); 
        //Printing on serial monitor
        Serial.println(">200K"); 
        display.display();          
        delay(100);
      }
    }
    if(res_scale == 2)
    {
      //pinMode(res_2k,INPUT);
      //pinMode(res_20k,INPUT);
      pinMode(res_470k,OUTPUT);  
      digitalWrite(res_470k,LOW);
      int16_t adc1; // Leemos el ADC, con 16 bits   
      adc1 = ads.readADC_SingleEnded(1);
      resistance_voltage = (adc1 * 0.1875)/1000;
      int16_t adc2; // Leemos el ADC, con 16 bits   
      adc2 = ads.readADC_SingleEnded(2);
      battery_voltage = ((adc2 * 0.1875)/1000);
      measured_resistance = (Res_470k_value) * ( ( battery_voltage/resistance_voltage ) - 1  );
      //measured_resistance = (1000*Res_470k_value) * (battery_voltage - resistance_voltage )/ (resistance_voltage - 0.);
      measured_resistance = measured_resistance -  ( 0.19 * measured_resistance);
      // Serial.println(measured_resistance);
      if( measured_resistance < 400 && measured_resistance > 0 )
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("Mohms");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V"); 
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(measured_resistance,2);  
        //Printing on serial monitor
        Serial.print(measured_resistance);
        Serial.println(" Mohms");

        display.display();          
        delay(200);
      }
      else
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("Mohms"); 
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V");
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(">4M");
        //Printing on serial monitor
        Serial.println(">4M");
        display.display();          
        delay(200);
      }
    }
    //delay(2500);  // 3 Seconds delay
  }

  
  //Capacitance measurement

  if(mode == 0)
  {
    int16_t adc2; // Leemos el ADC, con 16 bits   
    adc2 = ads.readADC_SingleEnded(2);
    battery_voltage = ((adc2 * 0.1875)/1000);
    if(cap_scale == 0)    
    {   
      pinMode(cap_in_analogPin, INPUT);  
      pinMode(OUT_PIN,OUTPUT);
      digitalWrite(OUT_PIN, LOW); 
      pinMode(chargePin, OUTPUT);   
       
      digitalWrite(chargePin, HIGH);  //apply 5 Volts
      startTime = micros();           //Start the counter
      while(analogRead(cap_in_analogPin) < 648){       //While the value is lower than 648, just wait
      }
    
      elapsedTime= micros() - startTime;
      microFarads = ((float)elapsedTime / resistorValue) ; //calculate the capacitance value
  
  
      if (microFarads > 1)
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("uF");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V");
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        //Printing on serial monitor
        Serial.println(microFarads);
        display.print(microFarads);  
        display.display();          
        delay(100);   
      }
    
      else 
      {
        
        nanoFarads = microFarads * 1000.0; 
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("nF");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
       // display.print(battery_voltage,1); 
        display.print("V"); 
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        //Printing on serial monitor
        Serial.println(nanoFarads);
      
        display.print(nanoFarads);  
        display.display();
        delay(100);

      }

  
      digitalWrite(chargePin, LOW);            
      pinMode(dischargePin, OUTPUT);            
      digitalWrite(dischargePin, LOW);     //discharging the capacitor     
      while(analogRead(cap_in_analogPin) > 0){         
      }//This while waits till the capaccitor is discharged
    
      pinMode(dischargePin, INPUT);      //this sets the pin to high impedance
      
              
      display.setTextSize(1);
      display.setTextColor(BLACK,WHITE); 
      display.setCursor(0,54);
      // //Printing on serial monitor
      //   Serial.print("Discharging...");
      // display.print("Discharging...");  
      display.display();       
    }



    if(cap_scale == 1)
    {
       pinMode(cap_in_analogPin, INPUT);  
      pinMode(OUT_PIN,OUTPUT);
      digitalWrite(OUT_PIN, LOW); 
      pinMode(chargePin, OUTPUT);   
       
      digitalWrite(chargePin, HIGH);  //apply 5 Volts
      startTime = micros();           //Start the counter
      while(analogRead(cap_in_analogPin) < 648){       //While the value is lower than 648, just wait
      }
    
      elapsedTime= micros() - startTime;
      microFarads = ((float)elapsedTime / 9760000) ; //calculate the capacitance value
  
      nanoFarads = microFarads * 1000.0;
      
    
       if (nanoFarads > 1)
      {
        
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("nF");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V"); 
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        display.print(nanoFarads);  
        //Printing on serial monitor
        Serial.println(nanoFarads);
        display.display();
        delay(100);

      }
      else 
     {
       float picoFarads;
        picoFarads = nanoFarads * 1000.0;
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE); 
        display.setCursor(0,0);
        display.print("pF");
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(95,0);    
        display.print(battery_voltage,1); 
        display.print("V"); 
        
        display.setTextSize(1);
        display.setTextColor(BLACK,WHITE); 
        display.setCursor(0,22);
        //Printing on serial monitor
        Serial.println(picoFarads);
        display.print(picoFarads);  
        display.display();
        delay(100);
      }
  
      digitalWrite(chargePin, LOW);            
      pinMode(dischargePin, OUTPUT);            
      digitalWrite(dischargePin, LOW);     //discharging the capacitor     
      while(analogRead(cap_in_analogPin) > 0){         
      }//This while waits till the capaccitor is discharged
    
      pinMode(dischargePin, INPUT);      //this sets the pin to high impedance
      
              
      display.setTextSize(1);
      display.setTextColor(BLACK,WHITE); 
      display.setCursor(0,54);
      // //Printing on serial monitor
      // Serial.print("Discharging...");
      // display.print("Discharging...");  
      display.display(); 
    }
    
  }//end of mode
  



}