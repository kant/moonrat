#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH 128
#define HEIGHT 64
#define SPLIT 16
#define LEFT_MARGIN 9
#define LINE_HEIGHT 9

#define OLED_RESET 4
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

#define HEAT_PIN 8
#define BTN_UP 6
#define BTN_DOWN 7
#define BTN_SELECT 5

int sensePin = A0;  //This is the Arduino Pin that will read the sensor output
int sensorInput;    //The variable we will use to store the sensor input
int targetTemperature = 85;
int rawTemp = 0;
double temperature;        //The variable we will use to store temperature in degrees.
bool heating = false;

//graph variables
int graphTimeLength = 24;
int graphMaxTemp = 100;
int graphMinTemp = 60;

//menu variables
int menuSelection = 0;
bool inMenu = true;
bool up = false;
bool down = false;
bool select = true;

void showNumber(float number){
  display.clearDisplay();
  display.setCursor(28,24);
  display.setTextSize(2);
  display.println(number);
  display.display();
}

/*
 * Shows a graph of the temperature data over a period of time
 */
void showGraph(){
  
}

/* Displays the menu.
 * 
 * Menu Options:
 * - Show Temperature : switches to temperature display mode
 * - Show Graph : switches to graph dispay mode
 * - Start/Stop : starts or stops data collection
 * - Set Target : sets the target incubation temperature
 * - Export : send temperature data through serial port to a pc
 * - Reset : resets temperature data
 */
void showMenu(){
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  //print title
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("Menu");
  //print options
  display.setTextSize(1);
  display.setCursor(LEFT_MARGIN,SPLIT);
  display.println("Temperature");
  display.setCursor(LEFT_MARGIN,SPLIT + LINE_HEIGHT);
  display.println("Graph");
  display.setCursor(LEFT_MARGIN,SPLIT + 2*LINE_HEIGHT);
  display.println("Start");
  //print cursor
  display.setCursor(0,SPLIT + menuSelection*LINE_HEIGHT);
  display.fillCircle(3, SPLIT + menuSelection*LINE_HEIGHT + 3, 3, SSD1306_WHITE);
  
  display.display();
}

void heatOn(){
  digitalWrite(HEAT_PIN, HIGH);
}

void heatOff(){
  digitalWrite(HEAT_PIN, LOW);
}

void convertTemp(int raw){
  double temp = (double)raw / 1024;       //find percentage of input reading
  temp = temp * 5;                 //multiply by 5V to get voltage
  temp = temp - 0.5;               //Subtract the offset 
  temp = temp * 180 + 32 ;          //Convert to degrees
  temperature = (3*temperature + temp)/4;
}

//starts the interrupts
void startInterrupts(){
  noInterrupts();
  
  //set timer1 interrupt at 8Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 1952;// = (16*10^6) / (8*1024) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  interrupts();
}

//setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //Start the Serial Port at 9600 baud (default)

  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_SELECT, INPUT);
  pinMode(HEAT_PIN, OUTPUT);
  heatOff();

  startInterrupts();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(28,25);
  display.println("Hello");
}

//main
void loop() {
  convertTemp(rawTemp);
  //toggle heat
  if(!heating && temperature < targetTemperature - 1){
    heating = true;
  }
  else if(heating && temperature > targetTemperature + 1){
    heating = false;
  }

  if(heating){
    heatOn();
  }else{
    heatOff();
  }

  delay(250);
  Serial.println("tick");
  if(inMenu){
    //read buttons and menu
    if(up && menuSelection > 0){
      menuSelection--;
    }
    else if(down != 0 && menuSelection < 2){
      menuSelection++;
    }
    else if(select != 0){
      inMenu = false;
    }
    showMenu();
  }
  else{
    if(digitalRead(BTN_SELECT) != 0){
      inMenu = true;
    }
    if(menuSelection == 0){
      Serial.println("tick");
      showNumber(temperature);
    }
  }
  
  //Serial.print("Current Temperature: ");
  //Serial.println(temperature);
}

ISR(TIMER1_COMPA_vect){
  rawTemp = analogRead(A0);    //read the analog sensor and store it
  //read buttons
  up = digitalRead(BTN_UP);
  down = digitalRead(BTN_DOWN);
  select = digitalRead(BTN_SELECT);
}