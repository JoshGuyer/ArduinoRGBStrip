#include "FastLED.h"
#include "arduinoFFT.h"
#define SAMPLES 32             //Must be a power of 2
#define SAMPLING_FREQUENCY 4500 //Hz, must be less than 10000 due to ADC
#define NUM_LEDS 300
#define len 15 //rate at which the leds move down the strip (set to 300 to be synchronous)
#define spd 1 //speed that the rainbow moves
//This program takes a frequency from a microphone or aux input
//and uses the FFT to find the set of frequencies present in the signal.
//Depending on the amplitude of the frequencies, it sets the color of
//an array of leds based off the highs, mids, and lows.

CRGB leds[NUM_LEDS];
arduinoFFT FFT = arduinoFFT();
unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[SAMPLES];
double vImag[SAMPLES];

int r, g, b;
int data = 48;
int state = 0;
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

#define DATA_PIN 6
void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  for (int i = 0; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}

void loop() {
  state = data;
  if (Serial.available()) {
    data = Serial.read();
  } else {
    data = state;
  }
  if (data == 48) {
    state = 48;
    for (int i = 0; i < NUM_LEDS ; i++) {
      leds[i] = CRGB(0, 0, 0);
    }
    FastLED.show();
  }
  if (data == 49) {
    state = 49;
    setBasedOnFrequency();
  }
  if (data == 50) {
    state = 50;
    static uint8_t startIndex = 0;
    startIndex = startIndex + spd; /* motion speed */
    setRainbow(startIndex);
    FastLED.show();
  }
}
void setBasedOnFrequency() {
  for (int i = 0; i < SAMPLES; i++)
  {
    microseconds = micros();    //Overflows after around 70 minutes!

    vReal[i] = analogRead(0);
    vImag[i] = 0;

    while (micros() < (microseconds + sampling_period_us)) {
    }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  int totalLow = 0, totalMid = 0, totalHigh = 0;
  for (int i = 3; i <= 8; i++) {//finds the total of low frequencies
    totalLow += vReal[i];
  }
  for (int i = 8; i <= 10; i++) {//finds the total of mid frequencies
    totalMid += vReal[i];
  }
  for (int i = 10; i < 16; i++) {//finds the total of high frequencies
    totalHigh += vReal[i];
  }
  r = totalMid / 10;
  if (r < 9) {
    r = 0;
  }
  g = totalHigh / 10;
  if (g < 9) {
    g = 0;
  }
  b = totalLow / 15;
  if (b < 10) {
    b = 0;
  }
  if (r > 255) {
    r = 255;
  }
  if (g > 255) {
    g = 255;
  }
  if (b > 255) {
    b = 255;
  }
  printColors();
  for (int i = 0; i < len; i++) {
    leds[i] = CRGB(g, r, b);
  }
  FastLED.show();
  for (int i = NUM_LEDS; i >= len; i--) {
    for (int j = 0; j <= len; j++) {
      leds[i] = leds[i - j];
    }
  }
}
void setRainbow(uint8_t colorIndex) {

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, 255, currentBlending);
    colorIndex += 1;
  }
}
void printColors() {
  Serial.print(b);
  Serial.print(" ");
  Serial.print(r);
  Serial.print(" ");
  Serial.print(g);
  Serial.print(" ");
  Serial.println(255);
}
