//#define DEBUG  //Раскоментировать чтобы включить отладку по уарт
#define GS_NO_ACCEL // отключить модуль движения с ускорением (уменьшить вес кода)
#define DRIVER_STEP_TIME 20
//#define USE_MICRO_WIRE
//#include <core.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <ServoSmooth.h>
#include <wire.h>
#include <GyverOLED.h>
#include <GyverStepper2.h>
#include <GyverWDT.h>
#include "myCycle.h"



//работа с памтью EEPROM
#define INIT_ADDR 1023
#define INIT_KEY 254

//функции для отладки
#ifdef DEBUG
#define PRINTS(x) {Serial.print(F(x)); }
#define PRINT(s,v)  { Serial.print(F(s)); Serial.print(v); }
#else 
#define PRINTS(x)
#define PRINT(s,v)
#endif

//сервы
#define AMOUNT 6 // указываем колличество приводов используемых в проекте
#define BPIN A7
#define impulsMin  500  //600
#define impulsMax 2500 //2600
#define smin 2400
#define smax 500
#define servosSpeed 50
#define servosAccel 0.3
#define homePosition 180
#define maxDeg 180

//таймеры
#define MY_PERIOD 1000
#define STEPP_PERIOD 5000
#define BTN_PERIOD 10000
uint32_t tmr1; 
uint32_t tmr2; 
uint32_t tmr3;
uint32_t tmr_stp;
uint32_t tmr_oled;

//Шаговик
#define steps 3200
#define pinStep 8
#define pinDir 7
#define pinEnable 12


//объекты сервы, экран, шаговик
ServoSmooth servos[AMOUNT];
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
GStepper2<STEPPER2WIRE> stepper(steps, pinStep, pinDir, pinEnable);




uint8_t AB;


boolean isRepeatTrue = false;
boolean isManualClick = false;

uint8_t GetAB();
void OledPrint();
void StepperCCW();
void StepperCW();
void writingEEPROM();
void WriteStatusBTN();
void ManualBTN();
void checkEEPROM();

uint32_t eepromTimer = 0;
boolean eepromFlag = false;

int stepperInitialpoint = 0;
int16_t servo0InitialPoint = 2400;
int16_t servo1InitialPoint = 1400;
int16_t servo2InitialPoint = 569;
int16_t servo3InitialPoint = 2150;
int16_t servo4InitialPoint = 640;
int16_t servo5InitialPoint = 640;

char ch;
char res;

unsigned long timer10s;
unsigned long timeSec;
unsigned long cycle5s;
unsigned long cycle10s;
int steppCur;

bool isSoftReset = false;
bool isStepperIsZero = false;

void setup() {

  Serial.begin(9600);
  Serial.setTimeout(10);
  //EEPROM.put(254, 10); // раскоментировать чтобы записать начальные значения в EEPROM
  checkEEPROM();
    

  EEPROM.get(0, stepperInitialpoint);
  EEPROM.get(10, servo0InitialPoint);
  EEPROM.get(20, servo1InitialPoint);
  EEPROM.get(30, servo2InitialPoint);
  EEPROM.get(40, servo3InitialPoint);
  EEPROM.get(50, servo4InitialPoint);
  EEPROM.get(60, servo5InitialPoint);
  
  
  PRINTS("\nversion 1.0, arduino nano");
  PRINT("\nServos connected: ", AMOUNT);
  PRINT("\nSet speed: ", servosSpeed);
  PRINT("\nSet accel: ", servosAccel);
  PRINTS("\nНачнем!");
  PRINT("\nПозиция шаговика: ", stepperInitialpoint);
  PRINT("\nПозиция Серво 0: ", servo0InitialPoint);
  PRINT("\nПозиция Серво 1: ", servo1InitialPoint);
  PRINT("\nПозиция Серво 2: ", servo2InitialPoint);
  PRINT("\nПозиция Серво 3: ", servo3InitialPoint);
  PRINT("\nПозиция Серво 4: ", servo4InitialPoint);


  oled.init();  // инициализация дисплея
  oled.clear();
  oled.autoPrintln(true);
  oled.home(); //устанавливаем курсор в 0,0
  oled.setScale(2);
  oled.print(F("Привет!"));
  delay(500);

  
  servos[5].setDirection(REVERSE);
  servos[5].attach(13,impulsMin,impulsMax, 640);
  
  servos[4].attach(5, impulsMin, impulsMax, 640);

  servos[3].attach(6, impulsMin, 2200, 2150);

  servos[2].attach(9, impulsMin, impulsMax, 569);

  servos[1].attach(10, impulsMin, impulsMax, 1400);

  servos[0].attach(11, impulsMin, impulsMax, 2400);
  

  servos[5].smoothStart();
  servos[4].smoothStart();

  servos[3].smoothStart();

  servos[2].smoothStart();

  servos[1].smoothStart();

  servos[0].smoothStart();
  
  
     for (int i = 0; i < AMOUNT; i++)
  {
    //servos[i].smoothStart();
    servos[i].setAutoDetach(false);
    servos[i].setAccel(servosAccel);
    servos[i].setSpeed(servosSpeed);
    servos[i].setMaxAngle(maxDeg);
    //
  } 
  


/*   servos[2].setTarget(1500);
  delay(1000);
  servos[3].setTarget(2300);
  delay(1000);
  servos[1].setTarget(1400);
  delay(1000);
  servos[0].setTarget(2400);
  delay(1000);
  servos[5].setTarget(500);
  servos[6].setTarget(500);
 */



  oled.clear();
  oled.setCursor(20,2);
  oled.print(F("Приводы подключены"));
  delay(500);



  
  
  OledPrint();
  oled.print(F("ГОТОВ!!!"));
  delay(500);
  oled.clear();



  stepper.setMaxSpeed(500); 
  stepper.setAcceleration(500);
  stepper.setCurrent(stepperInitialpoint);
  steppCur = stepper.getCurrent();
  if (steppCur != 0){isStepperIsZero = true;}
  

  
  
  //stepper.autoPower(false);
}

 inline void Reset() {asm("JMP 0");}

 char receivedData(){
  if (millis() - tmr_stp >= 40) {
    tmr3 = millis();
    if (Serial.available()) {
      ch = Serial.read(); }
    return ch;
  }
 }
  
uint32_t movementStepper(uint32_t step){
  if (!stepper.ready()){
    stepper.enable();
    stepper.setTarget(-step, RELATIVE);
  }
  
  
}

void timersInit() {
  unsigned long uptimeSec = millis() / 1000;
  timer10s  = uptimeSec; 
}

void timersWorks() {
  timeSec = millis() / 1000;
    if (timeSec - timer10s  >=  20)  {timer10s  = timeSec; cycle10s  = true;}
  }


void eraseCycles() {

  cycle10s  = false;

}


void loop() {

  timersWorks();
  
  // Код системных процессов

  



  servos[0].tick();
  servos[1].tick();
  servos[2].tick();
  servos[3].tick();
  servos[4].tick();
  servos[5].tick();
  stepper.tick();
  
  if (isStepperIsZero == true) {stepper.enable(); stepper.setTarget(0, ABSOLUTE); delay(1000); isStepperIsZero = false;}
  //if (cycle10s){writingEEPROM(); oled.home();oled.print(F("Saved"));}
  if (cycle10s){EEPROM.put(0, stepper.getCurrent()); oled.home();oled.print(F("Saved"));}


  if (isRepeatTrue==true)
  {
    res = receivedData();
  }
  
  

  switch (res)
  {

    //платформа лево/право
    case 'L':{
        StepperCCW();
      }
      break;
    case 'R':{
    
        StepperCW();
      }
      break;
    case 'S':{
      stepper.disable();
      break;
    }
    //захват
    case 'C': {
      int close = servos[0].getCurrent();
      if (close <= 500){close = 500; break;}
      servos[0].setTarget(close - 50);
      
    }
        break;
      
    case 'O': {
      int open = servos[0].getCurrent();
      if (open >= 2400){open = 2400; break;}
      servos[0].setTarget(open + 50);
  }
    break;

    //плечо
    case 'U':{
      int up = servos[4].getCurrent();
      if (up >= 2500) {up = 2500; break;}
      servos[4].setTarget(up + 25);
      servos[5].setTarget(up + 25);
    }
    break;

    case 'D':{
      int down = servos[4].getCurrent();
      if (down <= 500) {down = 500; break;}
      servos[4].setTarget(down - 25);
      servos[5].setTarget(down - 25);
    }
    
    break;
    //кисть вверх/низ
    case 'u':{
      int up2 = servos[2].getCurrent();
      if (up2 >= 2400) {up2 = 2400; break;}
      servos[2].setTarget(up2 + 25);
    }
    break;

    case 'd':{
      
      int down2 = servos[2].getCurrent();
      if (down2 <= 500) {down2 = 500; break;}
      servos[2].setTarget(down2 - 25);
    }
    break;
    //захват поворот
    case 'x':{
      int right = servos[1].getCurrent();
      if (right <= 500) {right = 500; break;}
      servos[1].setTarget(right - 25);
    }
    break;

    case 'y':{
      int left = servos[1].getCurrent();
      if (left >= 2400) {left = 2400; break;}
      servos[1].setTarget(left + 25);
    }

    //предплечье вверх/низ
    case 'A':{
      int up = servos[3].getCurrent();
      if  (up >= 2200) {up = 2200; break;}
      servos[3].setTarget(up + 25);
    }
    break;

    case 'B':{
      int down = servos[3].getCurrent();
      if  (down <= 500) {down = 500; break;}
      servos[3].setTarget(down - 25);
    }
    break;



    break;
  default:
    break;
  }
  

 /*  if (millis() - tmr2 >= STEPP_PERIOD)
  {
      tmr2 = millis();
      stepper.disable();
  } */
  if (isManualClick == true) {
    if (millis() - tmr3 >= 40) {
      tmr3 = millis();
      int pos0 = map(analogRead(A0),0,1023,impulsMin,2400);
      int pos1 = map(analogRead(A1),0,1023,impulsMin,impulsMax);
      int pos2 = map(analogRead(A2),0,1023,impulsMin,impulsMax);
      int pos3 = map(analogRead(A3),0,1023,impulsMax, impulsMin);
      int pos4 = map(analogRead(A6),0,1023,impulsMin,impulsMax);
      //int pos5 = map(analogRead(A6),0,1023,smin,smax);
      servos[0].setTarget(pos0);
      servos[1].setTarget(pos1);
      servos[2].setTarget(pos2);
      servos[3].setTarget(pos3);
      servos[5].setTarget(pos4);
      servos[4].setTarget(pos4);

  }
  }

AB = GetAB();
switch (AB)
  {
  case 1:
  {
    if (!stepper.ready())
  {
    stepper.enable();
    stepper.setTarget(0, RELATIVE);
  }
    servos[4].setTarget(640);
    servos[3].setTarget(2150);
    servos[5].setTarget(640);
    servos[6].setTarget(640);
    servos[2].setTarget(569);
    servos[1].setTarget(1400);
    servos[0].setTarget(2400);


    isManualClick = false;
    isRepeatTrue = true;
    PRINT("\nAB", AB);
    OledPrint();
    oled.print(F("Auto"));
  }
    break;
  case 2:
  {
  
    
    ManualBTN();
  }
      
    break;
  case 3:
  {
    WriteStatusBTN();
  }
  break;

  case 4: {
    if (isManualClick==true){StepperCW();}
    else if (isSoftReset==true)
    {
      isSoftReset=false;
      EEPROM.put(0, 0);
      delay(5000);
      Reset();
    }
  }
  
  break;

    case 5: {
      if (isManualClick==true){StepperCCW();}
      else {EEPROM.put(254, 10);  isSoftReset=true; OledPrint(); oled.print(F("Сброс? Нажми кнопку 4"));}
  }
  
  break;
  
    
  default:
    break;
  }

eraseCycles();

  
}

void checkEEPROM(){
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) { // первый запуск
    EEPROM.write(INIT_ADDR, INIT_KEY);    // записали ключ
    // записали стандартное значение 
    // в данном случае это значение переменной
    EEPROM.put(0, stepperInitialpoint);
    EEPROM.put(10, servo0InitialPoint);
    EEPROM.put(20, servo1InitialPoint);
    EEPROM.put(30, servo2InitialPoint);
    EEPROM.put(40, servo3InitialPoint);
    EEPROM.put(50, servo4InitialPoint);
    EEPROM.put(60, servo5InitialPoint);
  }
}

void writingEEPROM() {

    //stepperInitialpoint = stepper.getCurrent();
    EEPROM.put(0, stepper.getCurrent());     // записали в EEPROM текущее положение двигателя
    EEPROM.put(10, servos[0].getCurrent());
    EEPROM.put(20, servos[1].getCurrent());
    EEPROM.put(30, servos[2].getCurrent());
    EEPROM.put(40, servos[3].getCurrent());
    EEPROM.put(50, servos[4].getCurrent());
    EEPROM.put(60, servos[5].getCurrent());
    
  }


void ManualBTN(){
    if (Serial.available())
    {
      Serial.end();
    }
    isRepeatTrue = false;
    isManualClick = true;
    PRINT("\nAB", AB);
    OledPrint();
    oled.print(F("Manual mode"));
}

void WriteStatusBTN(){
  int degr[AMOUNT];

    if (millis() - tmr1 >= MY_PERIOD) {
      tmr1 = millis(); 
      for (int i = 0; i < AMOUNT; i++)
    {
      degr[i] = servos[i].getCurrent();
    }
    
    OledPrint();
    oled.setScale(1);
    oled.textMode(BUF_ADD);
    for (int i = 0; i < AMOUNT; i++)
    {
      oled.print(F("Позиция:")) + oled.println(degr[i]);
    }
    oled.print(F("Motor: ")) + oled.print(stepper.getCurrent());
    oled.setScale(2);
    PRINT("\nAB", AB);

    writingEEPROM();
    PRINT("\nПозиция шаговика: ", stepperInitialpoint);
    }  
}

void StepperCCW(){
   if (stepper.getCurrent() <= 3500)
  {
    if (!stepper.ready())
  {
    stepper.enable();
    stepper.setTarget(200, RELATIVE);
  }
    else{stepper.setTarget(200, RELATIVE);}
  }
  else {stepper.stop(); OledPrint(); oled.print("Ошибка! Достигнут лимит!");}


      PRINT("\nПозиция шаговика: ", stepper.pos);
    

  
  
}

void StepperCW(){
  if (stepper.getCurrent() >= -3500)
  {
    if (!stepper.ready())
  {
    stepper.enable();
    stepper.setTarget(-200, RELATIVE);
  }
    else{stepper.setTarget(-200, RELATIVE);}
  }
  else {stepper.stop(); OledPrint(); oled.print("Ошибка! Достигнут лимит!");}
  
    


    PRINT("\nПозиция шаговика: ", stepper.pos);
    

}

uint8_t GetAB() {                                           // Функция устраняющая дребезг
  static int   count;
  static int   oldKeyValue;                                 // Переменная для хранения предыдущего значения состояния кнопок
  static int   innerKeyValue;
  uint8_t actualKeyValue = (analogRead(BPIN) / 171);        // Получаем актуальное состояние
  if (innerKeyValue != actualKeyValue) {                    // Пришло значение отличное от предыдущего
    count = 0;                                              // Все обнуляем и начинаем считать заново
    innerKeyValue = actualKeyValue;                         // Запоминаем новое значение
  }
  else {
    count += 1;                                             // Увеличиваем счетчик
  }
  if ((count >= 3) && (actualKeyValue != oldKeyValue)) {   // Счетчик преодолел барьер, можно иницировать смену состояний
    oldKeyValue = actualKeyValue;                           // Присваиваем новое значение
  }
  
  return    oldKeyValue;
}

void OledPrint() {
  oled.clear();
  oled.home();
}

