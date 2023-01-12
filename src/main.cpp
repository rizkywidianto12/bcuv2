#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

/*
KALIBRASI SENSOR PRESSURE


- KONVERSI KE BAR > LIHAT BARIS 134 - 135
*/
const int BAR_0 = 0 ;
const int BAR_1 = 0 ;
const int BAR_2 = 0 ;
const int BAR_3 = 0 ;
const int BAR_4 = 0 ;
const int BAR_5 = 0 ;
const int BAR_6 = 0 ;
const int BAR_7 = 0 ;


/*
KALIBRASI TOMBOL 

~ LOGIC

- BATAS ATAS
    PRESSURE OUT > SETTING BUTTON + BATAS ATAS = VALVE BUANG NYALA
- BATAS BAWAH 
    RESSURE OUT < SETTING BUTTON + BATAS BAWAH = VALVE MASUK NYALA
BACA BARIS 180
*/
// CHARGING VALVE / BATAS ATAS
const int CALH19 = 1;
const int CALH22 = 1;
const int CALH25 = 1;

// VENTING VALVE / BATAS BAWAH
const int CALL19 = 0;
const int CALL22 = 0;
const int CALL25 = 0;



// Konfigurasi Pin

// Sensor preasure
#define pressure_sens_1 A0    // Input
#define pressure_sens_2 A1    // Output

//button
#define pressure_1    2       // 1 BAR
#define pressure_2    3       // 2.2 BAR
#define pressure_3    4       // 2.5 BAR
#define pressure_4    5       // RELEASE 0 BAR

// Pin solenoid valve
#define charging_valve  11
#define venting_valve   12

// lcd 
#define col       16
#define row       4
#define i2c_addr  0x27
LiquidCrystal_I2C lcd(i2c_addr,col,row);  //LCD address

// variable
int aktif;      // mode tombol
float presIn;   // pressure sensor input
float prsOut;   // pressure sensor output
float preset;   // spressure set
float raw_1;    // Nilai sensor 1( belum di konvert ke bar)
float raw_2;
float CALH = 0, CALL = 0;

// debug
bool V1_STATE;
bool V2_STATE;
bool update ; // update parameter
int counting; // delay update lcd

void report(){
  /*
  Layout LCD
  # # # # # # # # # # # # # # # #
  # B C U   P R O T O
  # I N   = # # # # # # # # B A R
  # O U T = # # # # # # # # B A R
  # S E T = # # # # # # # # B A R
  */
  lcd.setCursor(8,0);
  lcd.print(raw_2);
  lcd.setCursor(6,1);
  lcd.print(String(presIn)+" BAR");
  lcd.setCursor(2,2);
  // lcd.print("OUT = ");
  lcd.print(String(prsOut)+" BAR");
  lcd.setCursor(2,3);
  // lcd.print("SET = ");
  lcd.print(String(preset)+" BAR");
}

float ultimap(float val,float min1, float max1, float min2, float max2){
  float y;
  y = (((max2-min2)/(max1-min1))*(val-min1))+min2;
  return y;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("iTs Work.. YEE");
  delay(1000);
  lcd.clear();
  lcd.print("BCU V2");
  lcd.setCursor(0,1);
  lcd.print("IN  = ");
  lcd.setCursor(-4,2);
  lcd.print("OUT = ");
  lcd.setCursor(-4,3);
  lcd.print("SET = ");

  pinMode(pressure_sens_1,0);
  pinMode(pressure_sens_2,0);

  pinMode(pressure_1,2);
  pinMode(pressure_2,2);
  pinMode(pressure_3,2);
  pinMode(pressure_4,2);

  pinMode(charging_valve,1);
  pinMode(venting_valve,1);
  // put your setup code here, to run once:
}

void loop() {
  // delay(1);
  raw_1 = analogRead(pressure_sens_1);
  raw_2 = analogRead(pressure_sens_2);

  presIn = ultimap(raw_1,BAR_0,BAR_7,0,7);
  prsOut = ultimap(raw_2,BAR_0,BAR_7,0,7);
  
  if(!digitalRead(pressure_1) && aktif !=1 ){
    // Serial.println("press 1");
    aktif = 1;
    preset = 1.9;
    CALH = CALH19;
    CALL = CALL19;
  }
  else if (!digitalRead(pressure_2) && aktif !=2 ){
    // Serial.println("press 2");
    aktif = 2;
    preset = 2.2;
    CALH = CALH22;
    CALL = CALL22;
  }
  else if (!digitalRead(pressure_3) && aktif !=3 ){
    // Serial.println("press 3");
    aktif = 3;
    preset = 2.5;
    CALH = CALH25;
    CALL = CALL25;
  }
  else if (!digitalRead(pressure_4)){
    // Serial.println("press 4");
    aktif = 4;
    preset = 0;
    while(!digitalRead(pressure_4)){
      digitalWrite(venting_valve,1);
    }
    digitalWrite(venting_valve,0);
  }


  /*
  INGFO
   - PRSOUT = PRESSURE SENSOR OUTPUT, SATUAN BAR
   - PRESET = PRESSURE SETTING BUTTON
   - CALL & CALH = SETTING BATAS BAWAH DAN BATAS ATAS

  LOGIC BUKAAN RELAY
  */
    if (prsOut < preset + CALL ){// Jika nilai sensor out lebih kecil dari nilai yang di set
      digitalWrite(charging_valve,1); // relay solenoid charging aktif, angin masuk
      digitalWrite(venting_valve,0);  // relay solenoid buang mati
      // V1_STATE = 1;
      // V2_STATE = 0;
    }

    else if (prsOut > preset + CALH){ // Jika nilai sensor out lebih besar dari nilai yang di set
      digitalWrite(charging_valve,0); // relay solenoid charging mati
      digitalWrite(venting_valve,1);  // relay solenoid buang aktif, angin dibuang
      // V1_STATE = 0;
      // V2_STATE = 1;
    }

    else {
      // V1_STATE = 0;
      // V2_STATE = 0;
      digitalWrite(charging_valve,0);
      digitalWrite(venting_valve,0);
    }
  
  counting++;
  if (counting > 1900){
    counting = 0;
    report();
  if (0){
    Serial.print(prsOut);
    Serial.print(";");
    Serial.print(presIn);
    Serial.print(";");
    Serial.print(preset);
    Serial.print("|");
    Serial.print(V1_STATE);
    Serial.print(";");
    Serial.print(V2_STATE);
    Serial.print("|");
    Serial.print(raw_1);
    Serial.print(";");
    Serial.println(raw_2);
  }
  }
}
