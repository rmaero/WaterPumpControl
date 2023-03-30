#include <EEPROM.h>

#define EEPROM_FLAG_ADDRESS 0
#define EEPROM_TIME_ADDRESS 1

#define PIN_LEARN 6
#define PIN_CISTERNA 5
#define PIN_TANQUE 4
#define PIN_BOMBA 3
#define PIN_LED_ERROR 2
#define AUX1 7
#define AUX2 8
#define TIEMPO_MAX_ON_MILLIS 180000 //3 min *60 seg * 1000 ms = 180.000 millis
#define MIN_TO_MILLIS 60000 // *60 seg * 1000 ms

//debug
#define PIN_LED_INTERNAL 13
bool ledState;

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
Serial.print("Bomba:");
Serial.println(t);
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
    }
  digitalWrite(PIN_LED_ERROR,ledState);

  //DEBUG 
  // digitalWrite(PIN_LED_INTERNAL,ledState);
  Serial.print("Error Led:");
  Serial.println(ledState);

}


/**************************************************************************
----------------MAIN STATE MACHINE-----------------------------------------
***************************************************************************/

typedef enum States {WAITING, LEARNING, PUMP_ON, SM_ERROR};
States relayStateMachine;

void stateMachine()
{
  Serial.print("Estado:");
  Serial.println(relayStateMachine);
//SWITCH-CASE ANDA MAL EN ARDUINO, REEMPLAZO POR IF
  if(relayStateMachine==WAITING)
  {
      if( isCisternaFull()  &&  !isTanqueFull())
      {
        relayStateMachine = PUMP_ON;          
        startPumpTime=millis();        
        turnRelayOn(true);        
      } 
  }else if(relayStateMachine==LEARNING)
  {
      Serial.println("LEARNING");
      turnRelayOn(true);
      if(!isCisternaFull()  ||  isTanqueFull())
      {
        turnRelayOn(false);
        maxTimeOn=millis()-startLearningTime;
        Serial.print("TIEMPO MAX PRENDIDO:");        
        Serial.println(maxTimeOn);
        /*********EEPROM*****************************************************/
        EEPROM.update(EEPROM_FLAG_ADDRESS,1);
        EEPROM.update(EEPROM_TIME_ADDRESS,maxTimeOn/MIN_TO_MILLIS);
        Serial.print("Tiempo max guardado en EEPROM:");        
        Serial.println(EEPROM.read(EEPROM_TIME_ADDRESS));
        /*********EEPROM*****************************************************/
        relayStateMachine = WAITING;
      }
  }else if(relayStateMachine == PUMP_ON)
  { 
      turnRelayOn(true);
      if( !isCisternaFull()  || isTanqueFull()  )
      {
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
        Serial.println("paso estado a ERROR"); 
        relayStateMachine = SM_ERROR;   
        ledTimer=1;      
        turnRelayOn(false);
      }
  }else if(relayStateMachine==SM_ERROR)
  {   
      Serial.println("ERROR"); 
      blinkErrorLed();
  }else{
      relayStateMachine=WAITING;
  }
}


/**************************************************************************
--------------------------------INIT---------------------------------------
***************************************************************************/
void setup() {

//debug 
  pinMode(PIN_LED_INTERNAL, OUTPUT);
  Serial.begin(9600);
  
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
        relayStateMachine=ERROR;
      }
    }
  }else{
    relayStateMachine=WAITING;
    if(EEPROM.read(EEPROM_FLAG_ADDRESS))
    {
      maxTimeOn = EEPROM.read(EEPROM_TIME_ADDRESS) * MIN_TO_MILLIS ;
      Serial.print("Tomo tiempo guardado en EEPROM (ms):");
      Serial.println(maxTimeOn);
    }else{
      Serial.println("Tomo tiempo default");
      maxTimeOn = TIEMPO_MAX_ON_MILLIS; 
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
  currentTime = millis();  
  stateMachine();
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


