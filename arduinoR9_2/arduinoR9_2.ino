
////////////////////
// R9_2

// 로그출력과 테스트 모드를 위한 매크로
//#define DEBUG 

#define BAUDRATE    9600
#define LED_PULSE_HIGH   250    // analog level
#define LED_PULSE_LOW    0
#define RELAY_HIGH         250  // analog level
#define RELAY_LOW         0

//////////////////////////////////////////////////////////////
// define pin number assignment on Arduino leonardo board
#define LED_PROG    6 //3
#define LED_MIN     5
#define LED_MAX     3 //6
#define LED_POWER   9

#define BTN_PROG    7 //2
#define BTN_MIN     4
#define BTN_MAX     2 //7
#define BTN_ON      8

#define RELAY24     12
#define RELAY34     13  // note: analog pin
#define RELAY_CONNECT   1
#define RELAY_DISCONN   0


//////////////////////////////////////////////////////////////
// 
// 전압이 0로 떨어지지 않도록 릴레이 제어
// 더이상 사용하지 않음 과전류로 센서 고장 원인이 될수 있음
//#define NO_0_VOLTAGE  

//////////////////////////////////////////////////////////////
// 
// 첫번째 EIN 버튼 눌렀을때 24V 없이 바로 34V 출력 
//#define GO_24V_WHEN_EIN1

// 두번째 EIN 버튼 눌렀을때 24V에서 delay후에 0V로 떨어짐
#define GO_24V_WHEN_EIN2


//////////////////////////////////////////////////////////////
// 
// VDC지원 SSR 릴레이를 사용하는 경우
#define USE_NEW_VDC_RELAY  

// MIN, MAX 버튼을 누를경우 delay 값 조정
#define EIN_24V_DELAY   320
#define EIN_34V_DELAY   250
#define MIN_DELAY   50
#define MAX_DELAY   100
#define MAX1_34V_DELAY   100
#define MAX2_24V_DELAY   2500
#define MAX3_34V_DELAY   250
#define MAX4_24V_DELAY   5000

#define DUMMY_DELAY     1500

#define END_34_DELAY   250
#define END_24_DELAY   320
#define RESET_DELAY 50

///////////////////////////////////////////////////////////////


#define STATE_INIT      100
#define STATE_PROG      200
#define STATE_MIN       300
#define STATE_MAX       400
#define STATE_END       500
#define STATE_TEST      600
int state;


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void setup(void)
{
#ifdef DEBUG
    Serial.begin(BAUDRATE);
    while(!Serial)
      delay(10);
    Serial.println("setup() started");
#endif
    setupDigitalPins();

    // turn on power led
    //analogWrite(LED_POWER, LED_PULSE_HIGH);  

    // set initial state of relay switches
    relayControll(RELAY24, RELAY_DISCONN);
    relayControll(RELAY34, RELAY_DISCONN);

    // flash all LEDs during 1sec
    flashAllLeds(1, 500);

    //setupISR();


    state = STATE_INIT;

#ifdef DEBUG    
    Serial.println("setup() finished");
#endif
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void loop(void)
{
    switch(state)
    {
//////////////////////////////////////////////////////////
        case STATE_INIT:
            if(digitalRead(BTN_ON) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(INIT) ON pressed");
#endif
                state = STATE_TEST;
                processTESTstart();
            } 
            else if(digitalRead(BTN_PROG) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(INIT) PROG pressed");
#endif
                state = STATE_PROG;
                processPROG();
                blinkLedSet(LED_MIN);
            }
            break;

//////////////////////////////////////////////////////////
        case STATE_PROG:

            blinkLed(LED_MIN);

            if(digitalRead(BTN_MIN) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(PROG) MIN pressed");
#endif
                state = STATE_MIN;
                processMIN();
                blinkLedSet(LED_MAX);
            }
            else if(digitalRead(BTN_PROG) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(PROG) PROG pressed");
#endif
                state = STATE_INIT;
                processEND();
            }
            break;

//////////////////////////////////////////////////////////
        case STATE_MIN:

            blinkLed(LED_MAX);
            if(digitalRead(BTN_MAX) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(MIN) MAX pressed");
#endif
                //state = STATE_MAX;
                state = STATE_INIT;
                processMAX();
            }
            else if(digitalRead(BTN_PROG) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(MIN) PROG pressed");
#endif
                state = STATE_INIT;
                processEND();
            }

            break;

//////////////////////////////////////////////////////////
        case STATE_MAX:
            if(digitalRead(BTN_PROG) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(MAX) PROG pressed");
#endif
                state = STATE_END;
                //processEND();
            }
            break;

        case STATE_TEST:
            if(digitalRead(BTN_ON) == LOW)
            {
#ifdef DEBUG    
                Serial.println("(TEST) ON pressed");
#endif
                state = STATE_INIT;
                processTESTend();
            }
            break;

//////////////////////////////////////////////////////////
        case STATE_END:
            break;

        default:
            break;

    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
int toggle=0;
int tbase, tt;
void blinkLedSet(int num)
{
    tbase = millis()/500;
}

void blinkLed(int num)
{
    tt = millis()/500;
    if(tt <= tbase)
        return;
    
    tbase = tt;

    if (toggle){
        analogWrite(num, LED_PULSE_HIGH);
        toggle = 0;
    }
    else{
        analogWrite(num, LED_PULSE_LOW);
        toggle = 1;
    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processPROG(void) // 320, 250
{
#ifdef DEBUG    
    Serial.print(">>> Start processEIN(), delay = 24V:");
    Serial.print(EIN_24V_DELAY);
    Serial.print(", 34V:");  
    Serial.println(EIN_34V_DELAY);          
#endif

    analogWrite(LED_POWER, LED_PULSE_HIGH);  
    analogWrite(LED_PROG, LED_PULSE_HIGH);

#ifndef GO_24V_WHEN_EIN1
    // V24(ON), V34(OFF)
    relayControll(RELAY34, RELAY_DISCONN);
    relayControll(RELAY24, RELAY_CONNECT);
    delay(EIN_24V_DELAY);
#endif

    // V24(OFF), V34(ON)
    relayControll(RELAY24, RELAY_DISCONN);
    relayControll(RELAY34, RELAY_CONNECT);

    delay(EIN_34V_DELAY);

    // V24(ON), V34(OFF)
    relayControll(RELAY34, RELAY_DISCONN);
    relayControll(RELAY24, RELAY_CONNECT);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processEND(void)
{
#ifdef DEBUG    
    Serial.println(">>> Start processEND()");
#endif

    // 24V(OFF), 34V(ON)
    relayControll(RELAY24, RELAY_DISCONN);
    relayControll(RELAY34, RELAY_DISCONN);

    delay(DUMMY_DELAY); // handle dummy key event

    analogWrite(LED_MIN, LED_PULSE_LOW);
    analogWrite(LED_MAX, LED_PULSE_LOW);
    analogWrite(LED_PROG, LED_PULSE_LOW);

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processMIN(void)   // 50
{
#ifdef DEBUG    
    Serial.print(">>> Start processMIN(), delay = 34V:");
    Serial.println(MIN_DELAY);
#endif

    analogWrite(LED_MIN, LED_PULSE_HIGH);

    // V24(OFF), V34(ON)
    relayControll(RELAY24, RELAY_DISCONN);
    relayControll(RELAY34, RELAY_CONNECT);

    delay(MIN_DELAY);

    // V24(ON), V34(OFF)
    relayControll(RELAY34, RELAY_DISCONN);
    relayControll(RELAY24, RELAY_CONNECT);

    delay(2000);
    analogWrite(LED_MIN, LED_PULSE_LOW);

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processMAX(void)   // 100
{
#ifdef DEBUG    
    Serial.print(">>> Start processMAX(), delay = 34V:");
    Serial.println(MAX_DELAY);
#endif

    //flashLed(2, 500, LED_MAX, LED_PULSE_HIGH);
    analogWrite(LED_MAX, LED_PULSE_HIGH);

    // V24(OFF), V34(ON)
    relayControll(RELAY24, RELAY_DISCONN);
    relayControll(RELAY34, RELAY_CONNECT);
    delay(MAX1_34V_DELAY);

    // V24(ON), V34(OFF)
    relayControll(RELAY34, RELAY_DISCONN);
    relayControll(RELAY24, RELAY_CONNECT);
    delay(MAX2_24V_DELAY);

    analogWrite(LED_MAX, LED_PULSE_LOW);

    // V24(OFF), V34(ON)
    relayControll(RELAY24, RELAY_DISCONN);
    relayControll(RELAY34, RELAY_CONNECT);
    delay(MAX3_34V_DELAY);

    // V24(ON), V34(OFF)
    relayControll(RELAY34, RELAY_DISCONN);
    relayControll(RELAY24, RELAY_CONNECT);
    delay(MAX4_24V_DELAY);

    relayControll(RELAY24, RELAY_DISCONN);
    TurnOnOffAllLeds(LED_PULSE_LOW);

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processTESTstart(void)
{
#ifdef DEBUG    
    Serial.println("(TEST) Start processTESTstart()");
#endif

    analogWrite(LED_POWER, LED_PULSE_HIGH);  
    analogWrite(LED_MIN, LED_PULSE_HIGH);    
    analogWrite(LED_MAX, LED_PULSE_HIGH);  

    // V24(ON), V34(OFF)
    relayControll(RELAY24, RELAY_CONNECT);

    delay(DUMMY_DELAY);
}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processTESTend(void)
{
#ifdef DEBUG    
    Serial.println("(TEST) Start processTESTend()");
#endif

    analogWrite(LED_POWER, LED_PULSE_LOW);  
    analogWrite(LED_MIN, LED_PULSE_LOW);    
    analogWrite(LED_MAX, LED_PULSE_LOW);  

    // V24(ON), V34(OFF)
    relayControll(RELAY24, RELAY_DISCONN);

    delay(DUMMY_DELAY);
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void setupDigitalPins(void)
{
#ifdef DEBUG    
    Serial.println(">>> Start setupDigitalPins()");
#endif
    pinMode(LED_PROG, OUTPUT);
    pinMode(LED_MIN, OUTPUT);
    pinMode(LED_MAX, OUTPUT);
    pinMode(LED_POWER, OUTPUT);

    pinMode(BTN_PROG, INPUT_PULLUP);
    pinMode(BTN_MIN, INPUT_PULLUP);
    pinMode(BTN_MAX, INPUT_PULLUP);
    pinMode(BTN_ON, INPUT_PULLUP);

    pinMode(RELAY24, OUTPUT);
    pinMode(RELAY34, OUTPUT);    
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// flash all LED during given period
void flashAllLeds(int num, int ms)
{
#ifdef DEBUG    
    Serial.println(">>> Start flashAllLeds()");
#endif

    for(int i = 0;i < num;i++)
    {
        TurnOnOffAllLeds(LED_PULSE_HIGH);
        delay(ms);
        TurnOnOffAllLeds(LED_PULSE_LOW);
        delay(ms);
    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// turn on/off all LED during given period
void TurnOnOffAllLeds(int state)
{
#ifdef DEBUG    
    Serial.println(">>> Start TurnOnOffAllLeds()");
#endif

    analogWrite(LED_POWER, state);
    analogWrite(LED_PROG, state);
    analogWrite(LED_MIN, state);    
    analogWrite(LED_MAX, state);   
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// flash all LED during given period
void flashLed(int num, int ms, int led, int last_state)
{
#ifdef DEBUG    
    Serial.print(">>> Start flashLed() : led = ");
    Serial.println(led);
#endif
    
    if(led < 3 || led > 9)
    {
        Serial.println("led number is wrong");
        return;
    }

    for(int i = 0;i < num;i++)
    {
        analogWrite(led, LED_PULSE_HIGH);
        delay(ms);
        analogWrite(led, LED_PULSE_LOW);
        delay(ms);
    }

    analogWrite(led, last_state);
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
#ifdef USE_NEW_VDC_RELAY
void relayControll(int relay, int value)
{
#ifdef DEBUG    
    Serial.print(">>> relayControll() : relay="); Serial.print(relay);
    Serial.print(", value=");Serial.println(value);
#endif    
  if(relay == RELAY24)
  {
    if(value == RELAY_CONNECT)
      digitalWrite(RELAY24, HIGH);
    else
      digitalWrite(RELAY24, LOW);
  }
  else
  {
    if(value == RELAY_CONNECT)
      analogWrite(RELAY34, RELAY_HIGH);
    else
      analogWrite(RELAY34, RELAY_LOW);    
  }
}
#else
void relayControll(int relay, int value)
{
  if(relay == RELAY24)
  {
    if(value == RELAY_CONNECT)
      digitalWrite(RELAY24, LOW);
    else
      digitalWrite(RELAY24, HIGH);
  }
  else
  {
    if(value == RELAY_CONNECT)
      analogWrite(RELAY34, RELAY_LOW);
    else
      analogWrite(RELAY34, RELAY_HIGH);    
  }
}
#endif

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
