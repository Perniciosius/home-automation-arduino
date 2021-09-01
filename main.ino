#include <LiquidCrystal.h>
#include <IRremote.h>

#define switchMode 0xFD30CF				// Button 0

#define switchBulb 0xFD08F7				// Button 1
#define increaseLightIntensity 0xFD8877 // Button 2
#define decreaseLightIntensity 0xFD48B7	// Button 3

#define switchMotor 0xFD28D7			// Button 4
#define increaseMotorSpeed 0xFDA857		// Button 5
#define decreaseMotorSpeed 0xFD6897		// Button 6


bool autoMode = true;
bool isBulbOn = false;
bool isMotorOn = false;

int bulbPin = 6;
int motorPin = 5;
int ultrasonicPin = 8;
int temperaturePin = A1;
int photoResistorPin = A2;
int irPin = 4;

int manualMotorSpeed = 255;		//motor speed in manual mode
int manualLightIntensity = 255;	//light intensity of bulb

int lightIntensity;

long duration;
long distance;

float temperature;
float motorSpeed;

IRrecv receiver(irPin);
decode_results result;

LiquidCrystal lcd(7, 9, 10, 12, 11, 13);

void setup() {
  //Serial.begin(9600);
  pinMode(bulbPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(temperaturePin, INPUT);
  pinMode(photoResistorPin, INPUT);
  receiver.enableIRIn();
  lcd.begin(16, 2);
}


void loop() {
  temperature = analogRead(temperaturePin);  //read temperature
  temperature *= 0.48828125 / 3.5;           //convert temperature to Celsius
  lcd.setCursor(0, 0);
  lcd.print("TEMP : ");
  lcd.setCursor(7, 0);
  lcd.print(temperature);					 //display temperature on lcd
  lcd.setCursor(12,0);
  lcd.print("C");
  lcd.setCursor(0, 1);
  if(autoMode) {							 //display working mode - auto
  	lcd.print("AUTO MODE  ");				 // or manual
  } else {
  	lcd.print("MANUAL MODE");
  }
  
  //
  //
  //MANUAL MODE
  //
  //
  
  if (receiver.decode(&result)) {			 //decode ir signal
    //Serial.println(result.value, HEX);
    switch(result.value) {					 //from ir remote
    	case switchMode: 
      		autoMode = !autoMode;			 //switch working mode
      		break;							 //auto or manual
      	case switchBulb:
      		if(!autoMode) {
              if(isBulbOn) {
              	digitalWrite(bulbPin, LOW);  //in manual mode switch
                isBulbOn = !isBulbOn;		 //bulb on or off
              } else {
              	analogWrite(bulbPin, manualLightIntensity);
                isBulbOn = !isBulbOn;
              }
      		}
      		break;
      	case switchMotor:
      		if(!autoMode) {
              if(isMotorOn) {				 //in manual mode, switch
              	digitalWrite(motorPin, LOW); //motor on or off
                isMotorOn = !isMotorOn;
              } else {
              	analogWrite(motorPin, manualMotorSpeed);
                isMotorOn = !isMotorOn;
              }
            }
      		break;
      	case increaseLightIntensity:			//increase light intensity
      		if(!autoMode && isBulbOn) {			//of bulb
            	manualLightIntensity += 43;
              	if(manualLightIntensity > 255) {
              		manualLightIntensity = 255;
                }
              	analogWrite(bulbPin, manualLightIntensity);
            }
      		break;
      	case decreaseLightIntensity:			//decrese light intensity
      		if(!autoMode && isBulbOn) {			//of bulb
            	manualLightIntensity -= 43;
              	if(manualLightIntensity < 0) {
              		manualLightIntensity = 0;
                }
              	analogWrite(bulbPin, manualLightIntensity);
            }
      		break;
      	case increaseMotorSpeed:				//increase motor speed
      		if(!autoMode && isMotorOn) {
            	manualMotorSpeed += 43;
              	if(manualMotorSpeed > 255) {
                	manualMotorSpeed = 255;
                }
              	analogWrite(motorPin, manualMotorSpeed);
            }
      		break;
      	case decreaseMotorSpeed:				//decrease motor speed
      		if(!autoMode && isMotorOn) {
            	manualMotorSpeed -= 43;
              	if(manualMotorSpeed < 0) {
                	manualMotorSpeed = 0;
                }
              	analogWrite(motorPin, manualMotorSpeed);
            }
    }
    receiver.resume();                       //resume ir sensor for next
  }											 //input signal
  
  //
  //
  //AUTO MODE
  //
  //
  
  pinMode(ultrasonicPin, OUTPUT);            //set ultrasonic sensor to send
  digitalWrite(ultrasonicPin, LOW);			 //wave signal
  delayMicroseconds(2);
  digitalWrite(ultrasonicPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(ultrasonicPin, LOW);
  pinMode(ultrasonicPin, INPUT);
  duration = pulseIn(ultrasonicPin, HIGH);   //read duration of return signal
  distance = duration / 29 / 2;              //covert time to distance
  if(autoMode) {
    if(distance < 100) {					 //if someone present in room
    	lightIntensity = analogRead(photoResistorPin);
  		lightIntensity /= 4;				 //read light intensity
  	  	lightIntensity = 243 - lightIntensity; //calculate intensity of bulb
      	if(lightIntensity > 0) {
      		analogWrite(bulbPin, lightIntensity); //change intesity of bulb
      		isBulbOn = true;
        } else {
        	digitalWrite(bulbPin, lightIntensity);
          	isBulbOn = false;
        }
      	
      	if(temperature > 30) {
          	motorSpeed = 12.75 * (temperature - 30);
        	analogWrite(motorPin, motorSpeed);  //change motor speed based on
          	isMotorOn = true;					//temperature
        } else {
        	digitalWrite(motorPin, LOW);
          	isMotorOn = false;
        }
      
    } else {									//turn off bulb and motor
    	digitalWrite(bulbPin, LOW);				//when no one is in room
      	isBulbOn = false;
      	digitalWrite(motorPin, LOW);
      	isMotorOn = false;
    }
  }
  
  lcd.setCursor(15, 1);		//show status of bulb on lcd
  isBulbOn ? lcd.print('B') : lcd.print(' ');
  
  lcd.setCursor(13, 1);		//show status of fan on lcd
  isMotorOn ? lcd.print('F') : lcd.print(' ');
}
