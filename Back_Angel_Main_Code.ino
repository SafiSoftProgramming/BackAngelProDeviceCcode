   // Power Button
   #define POWER_BUTTON 26
   RTC_DATA_ATTR int PowerState = 0;
   const byte SENSORS_POWER_PIN = 4;
//////////////////////////////////////////////////////////////////////////
   // BATTERY LEVEL 
   #include <Pangodream_18650_CL.h> 
   #define BATTERY_LEVEL_PIN 34
   #define CONV_FACTOR 1.702 //best 1.86 to 100% 1.702
   #define READS 20 //20
   Pangodream_18650_CL BL(BATTERY_LEVEL_PIN, CONV_FACTOR, READS);

   #define RX_QUEUE_SIZE 4000

   // BUZZER AND VIBRATION
   const byte BUZZER_PIN = 16;
   const byte VIBRATION_PIN = 17;

   // ON BORD LED
   const byte ON_BORD_LED = 2;

   // MPU6050
   #include <Adafruit_MPU6050.h>
   #include <Adafruit_Sensor.h>
   #include <Wire.h>
   Adafruit_MPU6050 mpu;

   // WEIGHT SCALES
   #include <Arduino.h>
   #include "HX711.h"
   const int LOADCELL_DOUT_PIN = 14;
   const int LOADCELL_SCK_PIN = 33;
   HX711 scale;

   // BLUETOOTH
   #include "BluetoothSerial.h"
   #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
   #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
   #endif
   BluetoothSerial BluetoothSer;
   String BlueRecv = "";
   //#define RX_QUEUE_SIZE 512
   //#define TX_QUEUE_SIZE 128

   void BuzzerAndVibration();

   // SCALE _shoulder
   float reading ;
   float reading_set ;

   //Back reading set
   float reading_set_back ;

   //tilt reading set
   float reading_set_tilt = 0 ;

   /////////
   int set_state = 0 ;

   //pause and play state
   char pause_play_back_state = 'M' ;
   char pause_play_shoulder_state = 'M' ;
   char pause_play_tilt_state = 'M' ;
   char pause_play_vibration = 'M' ;
   char pause_play_buzzer = 'M' ;
   char delay_amount_char = '0' ; 
   char sensitivity_amount_char = '1' ;
   


   String HintShoulder ;
   String HintBack ;
   String HintTilt ;

   int count_delay_shoulder = 0 ;
   int count_delay_back = 0 ;
   int count_delay_tilt = 0 ;
   int delay_amount = 0 ;

   int shoulder_sensitivity = 4000 ;
   int back_sensitivity = 4 ;
   int tilt_sensitivity = 2 ;

   void setup() {
 
   setCpuFrequencyMhz(240);
   Serial.begin(9600);
   //delay(10); // will pause Zero, Leonardo, etc until serial console opens
  
   // Power Button
   pinMode(POWER_BUTTON, INPUT);
   pinMode(SENSORS_POWER_PIN, OUTPUT);
   if(PowerState == 0 ){
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,1); 
  }

   // BLUETOOTH
   BluetoothSer.begin("Back Angel");
   BluetoothSer.begin(9600);

   // BUZZER AND VIBRATION
   pinMode(BUZZER_PIN, OUTPUT);
   pinMode(VIBRATION_PIN, OUTPUT);

   // ON BORD LED
   pinMode(ON_BORD_LED, OUTPUT);

   // MPU6050
   mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
   mpu.setGyroRange(MPU6050_RANGE_500_DEG);
   mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
   
   // WEIGHT SCALES
   scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  }

 void loop() { ///////////////////////////////////////////////////////////////////////////////////////////////////////////

  //mpu.begin();

  // POWER
 digitalWrite(SENSORS_POWER_PIN, HIGH);
 if (digitalRead(POWER_BUTTON) == HIGH){
  PowerState = 1;
  }
  
  if (PowerState == 1 ){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(200);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  PowerState = 0 ;
  delay(1000);
  esp_deep_sleep_start();
  delay(1000);
  }


 // ON BORD LED
 if(BluetoothSer.connected()){
 digitalWrite(ON_BORD_LED, LOW);
 }else{

 digitalWrite(ON_BORD_LED, HIGH);
 delay(200);
 digitalWrite(ON_BORD_LED, LOW);
 delay(200);
 digitalWrite(ON_BORD_LED, HIGH);
 delay(200);
 digitalWrite(ON_BORD_LED, LOW);
 }
 
 ////////////////////////
                   
 // MPU6050
 if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  }
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
 // Serial.print("Acceleration X: ");
 // Serial.print(a.acceleration.x);
 // Serial.print(", Y: ");
 // Serial.print(a.acceleration.y);
 // Serial.print(", Z: ");
 // Serial.print(a.acceleration.z);
 // Serial.println(" m/s^2");

 // Serial.print("Rotation X: ");
 // Serial.print(g.gyro.x);
 // Serial.print(", Y: ");
 // Serial.print(g.gyro.y);
 // Serial.print(", Z: ");
 // Serial.print(g.gyro.z);
 // Serial.println(" rad/s");

 ////////////////////////

 // WEIGHT SCALES
 if (scale.is_ready()) {
    scale.set_scale();    
    reading = scale.get_units(1);

if(pause_play_shoulder_state == 'X'){
    if(reading < reading_set){

      count_delay_shoulder ++ ;

 if(reading < reading_set && count_delay_shoulder > delay_amount){
      BuzzerAndVibration();
 }

    HintShoulder = "E" ;
}else{
  HintShoulder = "e" ;
  count_delay_shoulder = 0 ;
  }

 }

  } 
  else {
    Serial.println("HX711 not found.");
  }



// Back
if(pause_play_back_state == 'Z'){
if(a.acceleration.z / back_sensitivity > reading_set_back){

     count_delay_back ++ ;   

if(a.acceleration.z / back_sensitivity > reading_set_back && count_delay_back > delay_amount ){
     BuzzerAndVibration();
}
     
    HintBack = "B" ;
}else{
  HintBack = "b" ;
  count_delay_back = 0 ;
  }
}

// tilt
if(pause_play_tilt_state == 'C'){
     if(a.acceleration.y < reading_set_tilt - tilt_sensitivity || a.acceleration.y > reading_set_tilt + tilt_sensitivity ){

      count_delay_tilt ++ ;
      Serial.println(count_delay_tilt);

      if(a.acceleration.y < reading_set_tilt - tilt_sensitivity && count_delay_tilt > delay_amount){
        BuzzerAndVibration();
      }

      if(a.acceleration.y > reading_set_tilt + tilt_sensitivity && count_delay_tilt > delay_amount){
        BuzzerAndVibration();
      }

      HintTilt = "T" ;   
    }else{
     HintTilt = "t" ;
     count_delay_tilt = 0 ;
  }
}



    // BLUETOOTH
    char Message = BluetoothSer.read();
   
    // Send All Data 
    String Send = String(BL.getBatteryChargeLevel())+","+String(temp.temperature - 15)+","+String(a.acceleration.y)+","+String(a.acceleration.z / 4)+","+String(reading)+","+HintShoulder+","+HintBack+","+HintTilt+","+pause_play_back_state+","+pause_play_shoulder_state+","+pause_play_tilt_state+","+pause_play_vibration+","+pause_play_buzzer+","+delay_amount_char+","+sensitivity_amount_char+",";
    BluetoothSer.println(Send); 
    //Serial.println(Send);

   // BlueRecv.trim();
    if (Message != ' '){

      if(Message == 'P'){
        esp_deep_sleep_start();
        delay(1000);
       }

       if(Message == 'S'){
          reading_set = reading - shoulder_sensitivity ;
          reading_set_back = a.acceleration.z / 4 + 0.75;
          reading_set_tilt = a.acceleration.y  ;
        }  

        if(Message == 'Z' || Message == 'z'){
          pause_play_back_state = Message ;
        }

         if(Message == 'X' || Message == 'x'){
          pause_play_shoulder_state = Message ;
        }

         if(Message == 'C' || Message == 'c'){
          pause_play_tilt_state = Message ;
        }

        if(Message == 'V' || Message == 'v'){
          pause_play_vibration = Message ;
        }

        if(Message == 'U' || Message == 'u'){
          pause_play_buzzer = Message ;
        }

        if(Message == '0' || Message == '4' || Message == '6' || Message == '8'){
          delay_amount = Message - '0' ;
          delay_amount_char = Message ;
        }


        if(Message == '1' || Message == '2' || Message == '3'){

          sensitivity_amount_char = Message ;

          if(Message == '1'){
            shoulder_sensitivity = 4000 ;
            reading_set = reading - shoulder_sensitivity ;
            back_sensitivity = 4 ;
            tilt_sensitivity = 2 ;
          }

          if(Message == '2'){
            shoulder_sensitivity = 12000 ;
            reading_set = reading - shoulder_sensitivity ;
            back_sensitivity = 5 ;
            tilt_sensitivity = 3 ;
          }

          if(Message == '3'){
            shoulder_sensitivity = 18000 ;
            reading_set = reading - shoulder_sensitivity ;
            back_sensitivity = 6 ;
            tilt_sensitivity = 4 ;
          }


        }





    }

     // Message = '';
      ////////////////////////

     // Auto Power off
     if (BL.getBatteryChargeLevel() < 2 ){
      digitalWrite(BUZZER_PIN, HIGH); 
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      delay(100);
      digitalWrite(BUZZER_PIN, HIGH); 
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      delay(100);
      digitalWrite(BUZZER_PIN, HIGH); 
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      esp_deep_sleep_start();
      }

    //  Serial.println(BL.getBatteryChargeLevel());


      delay(500);
       
      }

 void BuzzerAndVibration() {

        if(pause_play_vibration == 'V'){
          digitalWrite(VIBRATION_PIN, HIGH);
        }
        if(pause_play_buzzer == 'U'){
          digitalWrite(BUZZER_PIN, HIGH);
         }
 
      delay(100);                       
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(VIBRATION_PIN, LOW);    

      }