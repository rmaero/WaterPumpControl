/*---------------------------------------------------------------------------------
Rodrigo Maero 2023


---------------------------------------------------------------------------------*/


#include <EEPROM.h>

/*********************************************************************************
****************************PINS*************************************************/

#define PIN_LEARN 6
#define PIN_CISTERNA 5
#define PIN_TANQUE 4
#define PIN_BOMBA 3
#define PIN_LED_ERROR 2
#define PIN_AUX1 7
#define PIN_AUX2 8

/*************************DEFAULT TIME*****************************************/
#define TIEMPO_MAX_ON_MILLIS 180000 //3 min *60 seg * 1000 ms = 180.000 millis
#define ONE_MIN_TO_MILLIS 60000  // *60 seg * 1000 ms
#define MIN_TO_MILLIS 60000 // *60 seg * 1000 ms

/******************************************************************************
*****************************EEPROM*******************************************/
#define EEPROM_ADDRESS 0

struct Flash {
  char flag;
  uint32_t maxTimeMillis;
};

/************************************DEBUG**********************************/
#define PIN_LED_INTERNAL 13
bool ledState;

/***************************************************************************/
uint32_t maxTimeOn;
uint32_t startLearningTime;
uint32_t startPumpTime;

uint32_t currentTime;
uint32_t shortLoopTime;
uint32_t longLoopTime;


/**************************************************************************
----------------SENSOR INPUTS-----------------------------------------------
***************************************************************************/
//0V = LLENO, 5V=VACIO
bool isCisternaFull()
{
  return  !digitalRead(PIN_CISTERNA);
}

//0V = LLENO, 5V=VACIO
bool isTanqueFull()
{
  return !digitalRead(PIN_TANQUE);
}

/**************************************************************************
----------------RELAY OUTPUT-----------------------------------------------
***************************************************************************/
void turnRelayOn(bool t)
{
  digitalWrite(PIN_BOMBA,t);
//debug led  
  digitalWrite(PIN_LED_INTERNAL,t);
}


/**************************************************************************
----------------ERROR LED OUTPUT-------------------------------------------
***************************************************************************/
uint16_t ledTimer;
void blinkErrorLed()
{
  if(ledTimer==0)
    {
      ledTimer=1;
      ledState=  !ledState;
        //DEBUG 
        // digitalWrite(PIN_LED_INTERNAL,ledState);
        Serial.print("Error Led:");
        Serial.println(ledState);
    }
  digitalWrite(PIN_LED_ERROR,ledState);  
}


/**************************************************************************
----------------MAIN STATE MACHINE-----------------------------------------
***************************************************************************/
typedef enum States {WAITING, LEARNING, PUMP_ON, SM_ERROR};
States relayStateMachine;

void stateMachine()
{
//SWITCH-CASE ANDA MAL EN ARDUINO, REEMPLAZO POR IF
  if(relayStateMachine==WAITING)
  {
      if( isCisternaFull()  &&  !isTanqueFull())
      {        
        relayStateMachine = PUMP_ON;          
        startPumpTime=millis();        
        turnRelayOn(true);        
          //DEBUG
          Serial.print("Estado:");
          Serial.println(relayStateMachine);
          Serial.println("ENCIENDO BOMBA");
      } 
  }else if(relayStateMachine==LEARNING)
  {
      //Serial.println("LEARNING");
      turnRelayOn(true);
      if(!isCisternaFull()  ||  isTanqueFull())
      {
        Serial.println("Terminado ciclo learning");
        turnRelayOn(false);
        maxTimeOn=millis()-startLearningTime + ONE_MIN_TO_MILLIS;//maxTimeOn = tiempo medido + 1 min
        Serial.print("TIEMPO MAX PRENDIDO:");        
        Serial.println(maxTimeOn);
        /*********SAVE TO EEPROM*****************************************************/
        Flash saveToEEPROM;
        saveToEEPROM.flag='S';
        saveToEEPROM.maxTimeMillis=maxTimeOn;
        
        EEPROM.put(EEPROM_ADDRESS,saveToEEPROM);
        EEPROM.get(EEPROM_ADDRESS,saveToEEPROM);
        
        Serial.print("Tiempo max guardado en EEPROM:");        
        Serial.println(saveToEEPROM.maxTimeMillis);
        Serial.print("Guardado (S= SAVED, N=NOT saved):");
        Serial.println(saveToEEPROM.flag);
        /**************************************************************************/
        relayStateMachine = WAITING;
        Serial.println("Estado: WAITING");
      }
  }else if(relayStateMachine == PUMP_ON)
  { 
      turnRelayOn(true);
      if( !isCisternaFull()  || isTanqueFull()  )
      {
        Serial.println("Bomba apagada\nEstado: WAITING");
        turnRelayOn(false);
        relayStateMachine = WAITING;
      }
      unsigned long elapsedTime = millis()-startPumpTime;
      
      Serial.print("Quedan ");
      Serial.print((maxTimeOn-elapsedTime)/1000);
      Serial.println(" segundos hasta error");

      if(elapsedTime >= maxTimeOn)
      {
        Serial.print("Pasaron:"); 
        Serial.print(elapsedTime);                 
        Serial.println("ms"); 
        Serial.println("Bomba apagada\nEstado: ERROR");
        relayStateMachine = SM_ERROR;   
        ledTimer=1;      
        turnRelayOn(false);
      }
  }else if(relayStateMachine==SM_ERROR)
  {   
      blinkErrorLed();
  }else{
      relayStateMachine=WAITING;
  }
}


/**************************************************************************
--------------------------------INIT---------------------------------------
***************************************************************************/
void setup() {
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
//debug 
  pinMode(PIN_LED_INTERNAL, OUTPUT);
    
// BOTON LEARN
  pinMode(PIN_LEARN, INPUT_PULLUP);
  
//SENSORES CISTERNA Y TANQUE //DEBUG WITH BUTTONS: PULLUP
  pinMode(PIN_TANQUE, INPUT);
  pinMode(PIN_CISTERNA, INPUT);

//PINES SALIDA LED Y BOMBA  
  pinMode(PIN_LED_ERROR, OUTPUT);
  pinMode(PIN_BOMBA, OUTPUT);


//SET INITIAL STATE 
  if(digitalRead(PIN_LEARN)==0) //si boton LEARN presionado durante el inicio
  {
    delay(1000);
    if(digitalRead(PIN_LEARN)==0 ) //si sigue presionado 1 seg despues
    {
      if( !isTanqueFull() && isCisternaFull()){ //si esta en el estado correcto (cist llena, tanque vacio)
        relayStateMachine=LEARNING;
      }else{
        relayStateMachine=SM_ERROR;
      }
    }
  }else{
    relayStateMachine=WAITING;
    Flash readFromEEPROM;
    EEPROM.get(EEPROM_ADDRESS,readFromEEPROM);
    
    Serial.print("Tiempo max guardado en EEPROM:");
    Serial.println(readFromEEPROM.maxTimeMillis);
    Serial.print("Guardado (S= SAVED, N=NOT saved):");
    Serial.println(readFromEEPROM.flag);

    if(readFromEEPROM.flag=='S')
    {
      maxTimeOn = readFromEEPROM.maxTimeMillis;
      Serial.print("Tomo tiempo guardado en EEPROM (ms):");
      Serial.println(maxTimeOn);
    }else{
      maxTimeOn = TIEMPO_MAX_ON_MILLIS; 
      Serial.print("Tomo tiempo default:");
      Serial.println(maxTimeOn);
      Serial.println("Estado: WAITING");
    }
  }
  ledTimer=1;
  ledState=0;
  longLoopTime =millis();  
  shortLoopTime=millis();
}

/**************************************************************************
--------------------------------MAIN LOOP----------------------------------
***************************************************************************/
void loop() {
  stateMachine();
  currentTime = millis();  
  /*every 50ms
  if( (currentTime-shortLoopTime) >=50 )
  {
    shortLoopTime = currentTime;
    
  }
  */
  //every 1000ms 
  if( (currentTime-longLoopTime) >=1000)  
  {
    longLoopTime = currentTime;
    if(ledTimer)
    {
      ledTimer--;
    }
  }
}


