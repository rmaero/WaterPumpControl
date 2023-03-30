#include <EEPROM.h>

#define PIN_LEARN 4
#define PIN_CISTERNA 5
#define PIN_TANQUE 6
#define PIN_BOMBA 7
#define PIN_LED_ERROR 8
#define TIEMPO_MAX_ON_MILLIS 5000//180000 //3 min *60 seg * 1000 ms = 180.000 millis
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
//0V = VACIO, 5V=LLENO
bool isCisternaFull()
{
  return  digitalRead(PIN_CISTERNA);
}

//0V = VACIO, 5V=LLENO
bool isTanqueFull()
{
  return digitalRead(PIN_TANQUE);
}

/**************************************************************************
----------------RELAY OUTPUT-----------------------------------------------
***************************************************************************/
void turnRelayOn(bool t)
{
  digitalWrite(PIN_BOMBA,t);
//debug button  
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
   digitalWrite(PIN_LED_INTERNAL,ledState);
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
/*-------DEBUG-------*/
if(relayStateMachine == SM_ERROR)
{
  Serial.println("IT IS IN ERROR STATE");  
  //blinkErrorLed();
}
/*-------------END DEBUG------*/
Serial.print("Estado:");
Serial.println(relayStateMachine);

  switch(relayStateMachine)    
  {
    case WAITING:

      if( isCisternaFull()  &&  !isTanqueFull())
      {
        relayStateMachine = PUMP_ON;          
        startPumpTime=millis();        
        turnRelayOn(true);        
      }   
      /* LEARN BUTTON
      if(key_getPressEv())
      {
        relayStateMachine = LEARNING;
        startLearningTime = millis();
        turnRelayOn(true);
      } 
      */           
    break;

    case LEARNING:
      Serial.println("LEARNING");
      turnRelayOn(true);
      if(!isCisternaFull()  ||  isTanqueFull())
      {
        turnRelayOn(false);
        maxTimeOn=millis()-startLearningTime;
        Serial.print("TIEMPO MAX PRENDIDO:");        
        Serial.println(maxTimeOn);
        //TODO: SAVE TO EEPROM
        relayStateMachine = WAITING;
      }
    break;
    
    case PUMP_ON:  
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
    break;
    
    case SM_ERROR:     
      Serial.println("ERROR"); 
      blinkErrorLed();
    break;

    default:
      relayStateMachine=WAITING;  
    break;  
  }
}

/**************************************************************************
----------------STATE MACHINE FOR LEARN BUTTON-----------------------------
***************************************************************************/
/*
  typedef enum ButtonStates {WAITING_ACTIVATION,WAITING_DEACTIVATION};
  ButtonStates learnStateMachine;
  
  bool eventSw=0;
  
  uint16_t learnTimer =0;

  void checkLearnButton()
  {
    switch(learnStateMachine)
    {      
      case WAITING_ACTIVATION:
        if(digitalRead(PIN_LEARN)==0)
        {
          learnTimer=3; //wait 3 secs 
          learnStateMachine = WAITING_DEACTIVATION;
        }
      break;
      
      case WAITING_DEACTIVATION:
        if(digitalRead(PIN_LEARN))
        {          
          learnTimer=0;
          eventSw = 0;
          learnStateMachine = WAITING_ACTIVATION;
        }
        if(digitalRead(PIN_LEARN)==0 && learnTimer<=0)
        {
          eventSw = 1;
        }
      break;
      
      default:
        learnStateMachine = WAITING_ACTIVATION;
        eventSw=0;
      break;
    }
  }
  //check for key press events, event = 1 when button pressed
  bool key_getPressEv()
  {
    bool ret =0;
    if(eventSw)
    {
      eventSw = 0;
      ret=1;
    }
    return ret;
  }
*/

/**************************************************************************
--------------------------------INIT---------------------------------------
***************************************************************************/
void setup() {

//debug 
  pinMode(PIN_LED_INTERNAL, OUTPUT);
  Serial.begin(9600);
  
// BOTON LEARN
  pinMode(PIN_LEARN, INPUT_PULLUP);
  
//SENSORES CISTERNA Y TANQUE //DEBUG: PULLUP
  pinMode(PIN_TANQUE, INPUT_PULLUP);
  pinMode(PIN_CISTERNA, INPUT_PULLUP);

//PINES SALIDA LED Y BOMBA  
  pinMode(PIN_LED_ERROR, OUTPUT);
  pinMode(PIN_BOMBA, OUTPUT);


//SET INITIAL STATE 
  if(digitalRead(PIN_LEARN)==0) //si boton LEARN apretado durante el inicio
  {
    delay(20);
    relayStateMachine=LEARNING;
  }else{
    relayStateMachine=WAITING;
    maxTimeOn = TIEMPO_MAX_ON_MILLIS; //TODO: CHOOSE BETWEEN LEARN AND DEFAULT
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
  //checkLearnButton();
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
    /*
    if(learnTimer)    
    {
      learnTimer--;
    }
    */
    if(ledTimer)
    {
      ledTimer--;
    }
  }
  
}


