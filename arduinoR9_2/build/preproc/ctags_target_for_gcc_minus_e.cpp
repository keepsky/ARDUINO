# 1 "D:\\SRC\\ARDUINO\\arduinoR9_2\\arduinoR9_2.ino"

////////////////////
// R9_2

// 로그출력과 테스트 모드를 위한 매크로
# 14 "D:\\SRC\\ARDUINO\\arduinoR9_2\\arduinoR9_2.ino"
//////////////////////////////////////////////////////////////
// define pin number assignment on Arduino leonardo board
# 32 "D:\\SRC\\ARDUINO\\arduinoR9_2\\arduinoR9_2.ino"
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



//////////////////////////////////////////////////////////////
// 
// VDC지원 SSR 릴레이를 사용하는 경우


// MIN, MAX 버튼을 누를경우 delay 값 조정
# 68 "D:\\SRC\\ARDUINO\\arduinoR9_2\\arduinoR9_2.ino"
///////////////////////////////////////////////////////////////
# 77 "D:\\SRC\\ARDUINO\\arduinoR9_2\\arduinoR9_2.ino"
int state;


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void setup(void)
{

    Serial.begin(9600);
    while(!Serial)
      delay(10);
    Serial.println("setup() started");

    setupDigitalPins();

    // turn on power led
    //analogWrite(LED_POWER, LED_PULSE_HIGH);  

    // set initial state of relay switches
    relayControll(12, 0);
    relayControll(13 /* note: analog pin*/, 0);

    // flash all LEDs during 1sec
    flashAllLeds(1, 500);

    //setupISR();


    state = 100;


    Serial.println("setup() finished");

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void loop(void)
{
    switch(state)
    {
//////////////////////////////////////////////////////////
        case 100:
            if(digitalRead(8) == 0x0)
            {

                Serial.println("(INIT) ON pressed");

                state = 600;
                processTESTstart();
            }
            else if(digitalRead(7 /*2*/) == 0x0)
            {

                Serial.println("(INIT) PROG pressed");

                state = 200;
                processPROG();
                blinkLedSet(5);
            }
            break;

//////////////////////////////////////////////////////////
        case 200:

            blinkLed(5);

            if(digitalRead(4) == 0x0)
            {

                Serial.println("(PROG) MIN pressed");

                state = 300;
                processMIN();
                blinkLedSet(3 /*6*/);
            }
            else if(digitalRead(7 /*2*/) == 0x0)
            {

                Serial.println("(PROG) PROG pressed");

                state = 100;
                processEND();
            }
            break;

//////////////////////////////////////////////////////////
        case 300:

            blinkLed(3 /*6*/);
            if(digitalRead(2 /*7*/) == 0x0)
            {

                Serial.println("(MIN) MAX pressed");

                //state = STATE_MAX;
                state = 100;
                processMAX();
            }
            else if(digitalRead(7 /*2*/) == 0x0)
            {

                Serial.println("(MIN) PROG pressed");

                state = 100;
                processEND();
            }

            break;

//////////////////////////////////////////////////////////
        case 400:
            if(digitalRead(7 /*2*/) == 0x0)
            {

                Serial.println("(MAX) PROG pressed");

                state = 500;
                //processEND();
            }
            break;

        case 600:
            if(digitalRead(8) == 0x0)
            {

                Serial.println("(TEST) ON pressed");

                state = 100;
                processTESTend();
            }
            break;

//////////////////////////////////////////////////////////
        case 500:
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
        analogWrite(num, 250 /* analog level*/);
        toggle = 0;
    }
    else{
        analogWrite(num, 0);
        toggle = 1;
    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processPROG(void) // 320, 250
{

    Serial.print(">>> Start processEIN(), delay = 24V:");
    Serial.print(320);
    Serial.print(", 34V:");
    Serial.println(250);


    analogWrite(9, 250 /* analog level*/);
    analogWrite(6 /*3*/, 250 /* analog level*/);


    // V24(ON), V34(OFF)
    relayControll(13 /* note: analog pin*/, 0);
    relayControll(12, 1);
    delay(320);


    // V24(OFF), V34(ON)
    relayControll(12, 0);
    relayControll(13 /* note: analog pin*/, 1);

    delay(250);

    // V24(ON), V34(OFF)
    relayControll(13 /* note: analog pin*/, 0);
    relayControll(12, 1);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processEND(void)
{

    Serial.println(">>> Start processEND()");


    // 24V(OFF), 34V(ON)
    relayControll(12, 0);
    relayControll(13 /* note: analog pin*/, 0);

    delay(1500); // handle dummy key event

    analogWrite(5, 0);
    analogWrite(3 /*6*/, 0);
    analogWrite(6 /*3*/, 0);

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processMIN(void) // 50
{

    Serial.print(">>> Start processMIN(), delay = 34V:");
    Serial.println(50);


    analogWrite(5, 250 /* analog level*/);

    // V24(OFF), V34(ON)
    relayControll(12, 0);
    relayControll(13 /* note: analog pin*/, 1);

    delay(50);

    // V24(ON), V34(OFF)
    relayControll(13 /* note: analog pin*/, 0);
    relayControll(12, 1);

    delay(2000);
    analogWrite(5, 0);

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processMAX(void) // 100
{

    Serial.print(">>> Start processMAX(), delay = 34V:");
    Serial.println(100);


    //flashLed(2, 500, LED_MAX, LED_PULSE_HIGH);
    analogWrite(3 /*6*/, 250 /* analog level*/);

    // V24(OFF), V34(ON)
    relayControll(12, 0);
    relayControll(13 /* note: analog pin*/, 1);
    delay(100);

    // V24(ON), V34(OFF)
    relayControll(13 /* note: analog pin*/, 0);
    relayControll(12, 1);
    delay(2500);

    analogWrite(3 /*6*/, 0);

    // V24(OFF), V34(ON)
    relayControll(12, 0);
    relayControll(13 /* note: analog pin*/, 1);
    delay(250);

    // V24(ON), V34(OFF)
    relayControll(13 /* note: analog pin*/, 0);
    relayControll(12, 1);
    delay(5000);

    relayControll(12, 0);
    TurnOnOffAllLeds(0);

}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processTESTstart(void)
{

    Serial.println("(TEST) Start processTESTstart()");


    analogWrite(9, 250 /* analog level*/);
    analogWrite(5, 250 /* analog level*/);
    analogWrite(3 /*6*/, 250 /* analog level*/);

    // V24(ON), V34(OFF)
    relayControll(12, 1);

    delay(1500);
}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void processTESTend(void)
{

    Serial.println("(TEST) Start processTESTend()");


    analogWrite(9, 0);
    analogWrite(5, 0);
    analogWrite(3 /*6*/, 0);

    // V24(ON), V34(OFF)
    relayControll(12, 0);

    delay(1500);
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void setupDigitalPins(void)
{

    Serial.println(">>> Start setupDigitalPins()");

    pinMode(6 /*3*/, 0x1);
    pinMode(5, 0x1);
    pinMode(3 /*6*/, 0x1);
    pinMode(9, 0x1);

    pinMode(7 /*2*/, 0x2);
    pinMode(4, 0x2);
    pinMode(2 /*7*/, 0x2);
    pinMode(8, 0x2);

    pinMode(12, 0x1);
    pinMode(13 /* note: analog pin*/, 0x1);
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// flash all LED during given period
void flashAllLeds(int num, int ms)
{

    Serial.println(">>> Start flashAllLeds()");


    for(int i = 0;i < num;i++)
    {
        TurnOnOffAllLeds(250 /* analog level*/);
        delay(ms);
        TurnOnOffAllLeds(0);
        delay(ms);
    }
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// turn on/off all LED during given period
void TurnOnOffAllLeds(int state)
{

    Serial.println(">>> Start TurnOnOffAllLeds()");


    analogWrite(9, state);
    analogWrite(6 /*3*/, state);
    analogWrite(5, state);
    analogWrite(3 /*6*/, state);
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// flash all LED during given period
void flashLed(int num, int ms, int led, int last_state)
{

    Serial.print(">>> Start flashLed() : led = ");
    Serial.println(led);


    if(led < 3 || led > 9)
    {
        Serial.println("led number is wrong");
        return;
    }

    for(int i = 0;i < num;i++)
    {
        analogWrite(led, 250 /* analog level*/);
        delay(ms);
        analogWrite(led, 0);
        delay(ms);
    }

    analogWrite(led, last_state);
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void relayControll(int relay, int value)
{

    Serial.print(">>> relayControll() : relay="); Serial.print(relay);
    Serial.print(", value=");Serial.println(value);

  if(relay == 12)
  {
    if(value == 1)
      digitalWrite(12, 0x1);
    else
      digitalWrite(12, 0x0);
  }
  else
  {
    if(value == 1)
      analogWrite(13 /* note: analog pin*/, 250 /* analog level*/);
    else
      analogWrite(13 /* note: analog pin*/, 0);
  }
}
# 531 "D:\\SRC\\ARDUINO\\arduinoR9_2\\arduinoR9_2.ino"
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
