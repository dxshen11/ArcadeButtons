#include <Adafruit_NeoPixel.h>

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 49

// LED definitions
#define LED_PIN 6

// constant pin definitions
const int buttonPin = 13;
const int ledPin = 12;
const int stripPin = 2;
const int STROBE = 4;
const int RESET = 5;
const int DC_One = A0;
const int DC_Two = A1;
const int LED_chunk = 5; //How many LEDS are in 1 chunck?

int buttonState = 0;
int previousState = HIGH;
int freq_amp;
int Frequencies_One[7]; int Frequencies_Two[7];
int i; int j; int k; int l; int m; int n;
int avgFreq[7];
int maxFreq;
int fadeOffRate = 1.25; //defined for now, could be replaced by pot

uint32_t colorDict[LED_COUNT];

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:

void setup() {
  // put your setup code here, to run once:
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);

  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  digitalWrite(RESET, LOW);
  delay(5);

  //buttons
  Serial.begin(19200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)


  // assign LED colors:
  for (m = 0; m < strip.numPixels(); m++) { // For each pixel in strip...
    // Offset pixel hue by an amount to make one full revolution of the
    // color wheel (range of 65536) along the length of the strip
    // (strip.numPixels() steps):
    int pixelHue =  (m * 65536L / strip.numPixels());
    // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
    // optionally add saturation and value (brightness) (each 0 to 255).
    // Here we're using just the single-argument hue variant. The result
    // is passed through strip.gamma32() to provide 'truer' colors
    // before assigning to each pixel:
    colorDict[m] = strip.gamma32(strip.ColorHSV(pixelHue));
  }
}

void loop() {
  // strip.clear();
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && previousState != LOW) {
    Serial.print("Pressed.");
  }
  previousState = buttonState;
  Read_Frequencies();
  // Graph_Frequencies();
  delay(50);

  // process audio
  for (j = 0; j < 7; j++) {
    avgFreq[j] = (Frequencies_One[j] + Frequencies_Two[j]) / 2;
  }
  double avgMax = 0;
  int kMax;
  for (k = 0; k < 7; k++) {
    if (avgFreq[k] > avgMax) {
      avgMax = avgFreq[k];
      kMax = k;
    }
  }

  // Fade off function?
  uint32_t prevColor;
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    prevColor = strip.getPixelColor(i);
    Serial.println(prevColor);
    if (prevColor != 0) {
      uint8_t red = prevColor >> 16; // red
      uint8_t green = prevColor >> 8; //green
      uint8_t blue = prevColor; // blue
      red = red/fadeOffRate;
      green = green/fadeOffRate;
      blue = blue/fadeOffRate;
      strip.setPixelColor(i, strip.gamma32(strip.Color(red, green, blue)));
    }
  }
  for (n = (kMax * LED_chunk); n < ((kMax * LED_chunk) + LED_chunk); n++) {
    strip.setPixelColor(n, colorDict[n]);
  }
  strip.show();
}

void Read_Frequencies() {
  digitalWrite(RESET, HIGH);
  delayMicroseconds(200);
  digitalWrite(RESET, LOW);
  delayMicroseconds(200);

  //Read frequencies for each band
  for (freq_amp = 0; freq_amp < 7; freq_amp++)
  {
    digitalWrite(STROBE, HIGH);
    delayMicroseconds(50);
    digitalWrite(STROBE, LOW);
    delayMicroseconds(50);

    Frequencies_One[freq_amp] = analogRead(DC_One);
    Frequencies_Two[freq_amp] = analogRead(DC_Two);
  }
}

void Graph_Frequencies() {
  for (i = 0; i < 7; i++)
  {
    //    Serial.print(Frequencies_One[i]);
    //    Serial.print(" ");
    //    Serial.print(Frequencies_Two[i]);
    //    Serial.print(" ");
    Serial.print( (Frequencies_One[i] + Frequencies_Two[i]) / 2 );
    Serial.print("    ");
  }
  Serial.println();
}
