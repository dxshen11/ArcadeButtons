#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>

// LED globals
#define LED_STRIP_PIN 6
#define LED_CHUNK 5 // number of LEDs in 1 chunk
#define FADE_RESIST A6

const int STROBE = 4;
const int RESET = 5;
const int DC_One = A0;
const int DC_Two = A1;

int freq_amp;
int freqsOne[7];
int freqsTwo[7];
int avgFreq[7];
int maxFreq;
double fadeRate; // dictated by pot

const int NUM_CHANNELS = 7;
const int LED_COUNT = NUM_CHANNELS * LED_CHUNK;
uint32_t colorDict[LED_COUNT];

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed

// button globals
#define LEFT 7 // joystick controls
#define RIGHT 8
#define UP 9
#define DOWN 10

const int BUT_PIN = 13;
const int LED_BUT_PIN = 12;

int buttonState = 0;
int previousState = HIGH;
String previousDirection; 

void setup() {
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);

  // Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  digitalWrite(RESET, LOW);
  delay(5);

  // set up buttons
  pinMode(BUT_PIN, INPUT_PULLUP);
  pinMode(LED_BUT_PIN, OUTPUT);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  digitalWrite(LED_BUT_PIN, HIGH);

  // initialize LED strip
  pinMode(FADE_RESIST, INPUT);
  strip.begin();
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)


  // assign strip pixel colors such that the length of the strip
  // makes one full revolution of the color wheel (range of 6536)
  for (int i = 0; i < strip.numPixels(); i++) {
    int pixelHue =  (i * 65536L / strip.numPixels());
    colorDict[i] = strip.gamma32(strip.ColorHSV(pixelHue));
  }

  Keyboard.begin();
  Serial.begin(19200);
}

void Graph_Frequencies() {
  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    //    Serial.print(freqsOne[i]);
    //    Serial.print(" ");
    //    Serial.print(freqsTwo[i]);
    //    Serial.print(" ");
    Serial.print( (freqsOne[i] + freqsTwo[i]) / 2 );
    Serial.print("    ");
  }
  Serial.println();
}

double map_floats(int value, int input_low, double input_high, int output_low, double output_high) {
  double mappedValue = output_high - value / ((input_high - input_low) / (output_high - output_low));
  if (mappedValue < 1)
    return 1;
  return (mappedValue);
}

void Read_Frequencies() {
  digitalWrite(RESET, HIGH);
  delayMicroseconds(200);
  digitalWrite(RESET, LOW);
  delayMicroseconds(200);

  //Read frequencies for each band
  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    digitalWrite(STROBE, HIGH);
    delayMicroseconds(50);
    digitalWrite(STROBE, LOW);
    delayMicroseconds(50);

    freqsOne[i] = analogRead(DC_One);
    freqsTwo[i] = analogRead(DC_Two);
  }
}

String checkDirection() {
  // joystick press
  String directionPressed = "";
  bool leftStickState = !digitalRead(LEFT);
  bool rightStickState = !digitalRead(RIGHT);
  bool upStickState = !digitalRead(UP);
  bool downStickState = !digitalRead(DOWN);
  if (leftStickState){
    directionPressed += "l";
  }
  if (rightStickState){
    directionPressed += "r";
  }
  if (upStickState){
    directionPressed += "u";
  }
  if (downStickState){
    directionPressed += "d";
  }
  return (directionPressed);
}

void loop() {
  // check potentiometer state
  fadeRate = map_floats(analogRead(FADE_RESIST), 0, 1023, 0.5, 10);
  Serial.println(fadeRate);

  // check button press
  buttonState = digitalRead(BUT_PIN);
  if (buttonState == LOW && previousState != LOW) {
    //Keyboard.write('k');
    //    Keyboard.press('n');
    //    delay(100);
    //    Keyboard.releaseAll();
    Keyboard.write('k');
    // Keyboard.println("keeb");
    Serial.println("Pressed.");
  }
  previousState = buttonState;

  // joystick movement
  if ((checkDirection().indexOf("l") != -1) && (previousDirection.indexOf("l") == -1)){
    Keyboard.write(KEY_LEFT_ARROW);
  }
  if ((checkDirection().indexOf("r") != -1) && (previousDirection.indexOf("r") == -1)){
    Keyboard.write(KEY_RIGHT_ARROW);
  }
  if ((checkDirection().indexOf("u") != -1) && (previousDirection.indexOf("u") == -1)){
    Keyboard.write(KEY_UP_ARROW);
  }
  if ((checkDirection().indexOf("d") != -1) && (previousDirection.indexOf("d") == -1)){
    Keyboard.write(KEY_DOWN_ARROW);
  }
  previousDirection = checkDirection();
  
  // detect audio
  Read_Frequencies();
  delay(50);

  // find max avg frequency band
  for (int i = 0; i < NUM_CHANNELS; i++) {
    avgFreq[i] = (freqsOne[i] + freqsTwo[i]) / 2;
  }
  double avgMax = 0;
  int idxMax;
  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (avgFreq[i] > avgMax) {
      avgMax = avgFreq[i];
      idxMax = i;
    }
  }

  // fade all pixels with color
  uint32_t prevColor;
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    prevColor = strip.getPixelColor(i);
//    Serial.println(prevColor);
    if (prevColor != 0) {
      // unpack RGB from uint32_t color
      uint8_t red = prevColor >> 16;
      uint8_t green = prevColor >> 8;
      uint8_t blue = prevColor;
      red = red / fadeRate;
      green = green / fadeRate;
      blue = blue / fadeRate;
      strip.setPixelColor(i, strip.gamma32(strip.Color(red, green, blue)));
    }
  }

  // set color of max frequency band
  for (int i = (idxMax * LED_CHUNK); i < ((idxMax + 1)* LED_CHUNK); i++) {
    strip.setPixelColor(i, colorDict[i]);
  }

  strip.show();
}
