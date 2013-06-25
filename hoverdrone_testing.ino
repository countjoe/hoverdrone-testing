#include <Servo.h>

const int MOTOR_PIN0 = 2;
const int MOTOR_PIN1 = 3;
const int MOTOR_PIN2 = 4;
const int MOTOR_PIN3 = 5;

const int LED_PIN0 = 8;
const int LED_PIN1 = 9;
const int LED_PIN2 = 10;
const int LED_PIN3 = 11;

const int ENC_PINA = 12;
const int ENC_PINB = 13;

const int IR_PIN = 3;

const int MAX_MOTOR_VALUE = 180;

int lastEnc = 0;
int enc = 0;

int leds[4];
int ledIdx;
const int LED_DELAY = 1000;
unsigned long nextMillis = 0;

int encA = 0;
int encB = 0;

int encValue = 0;

Servo motor0;

void setup() {
  Serial.begin(9600);
  
  motor0.attach(MOTOR_PIN0);
  //pinMode(MOTOR_PIN0, OUTPUT);
  //pinMode(MOTOR_PIN1, OUTPUT);
  //pinMode(MOTOR_PIN2, OUTPUT);
  //pinMode(MOTOR_PIN3, OUTPUT);
  
  pinMode(LED_PIN0, OUTPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);

  pinMode(ENC_PINA, INPUT);
  pinMode(ENC_PINB, INPUT);
  
  digitalWrite( LED_PIN0, LOW );
  digitalWrite( LED_PIN1, LOW );
  digitalWrite( LED_PIN2, LOW );
  digitalWrite( LED_PIN3, LOW );
  
  led_init_blink();
  //accel_init();
}

void loop() {
  //accel_update();
  
  encA = digitalRead( ENC_PINA );
  encB = digitalRead( ENC_PINB );
  
  enc = ( encA | ( encB << 1 ) );
  
  if ( enc != lastEnc ) {
    switch ( enc + ( enc - lastEnc ) ) {
      case 4:
      case -1:
        encValue -= 1;
        break;
      default:
        encValue += 1;
    }
    
    if ( encValue > MAX_MOTOR_VALUE ) {
      encValue = MAX_MOTOR_VALUE;
    } else if ( encValue < 0 ) {
      encValue = 0;
    }
    
    update_motor();
    
    //led_count();
  }
  
  led_blink();
  
  lastEnc = enc;
}

void update_motor() {
  motor0.write( encValue );
  
  int ledVal = map( encValue, 0, MAX_MOTOR_VALUE, 0, 255 );
  analogWrite( LED_PIN3, ledVal );
  
  Serial.println( encValue );
}