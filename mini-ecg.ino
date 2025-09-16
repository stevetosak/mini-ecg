#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define ECG_PIN 34   // AD8232 OUTPUT -> GPIO34 

// kruzen buffer za sekoj sample
#define BUFFER_SIZE 128
int ecgBuffer[BUFFER_SIZE];
int bufferIndex = 0;

unsigned long lastBeatTime = 0;
int bpm = 0;
bool beatDetected = false;
int threshold = 3000;  // se nad ova se smeta za otcukuvanje

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("FINKI MINI ECG");
  display.display();
  delay(2800);

  // Initialize buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    ecgBuffer[i] = SCREEN_HEIGHT / 2;
  }
}

void loop() {
  // Analogen signal 0-4095
  int rawValue = analogRead(ECG_PIN);

  // Skaliranje za da moze da se pretstavi vo ekranot
  int ecgValue = map(rawValue, 0, 4096, SCREEN_HEIGHT-1, 0);

  
  ecgBuffer[bufferIndex] = ecgValue;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

  unsigned long now = millis();

  if (rawValue > threshold && !beatDetected) {
    beatDetected = true;
    unsigned long rrInterval = now - lastBeatTime;

    if (lastBeatTime > 0 && rrInterval > 200) { 
      bpm = 60000 / rrInterval;
      Serial.print("BPM: ");
      Serial.println(bpm);
    }
    lastBeatTime = now;
  }

  if (rawValue < threshold - 200) {
    beatDetected = false; //  ako padne signalot reset
  }

  display.clearDisplay();

  // Branot
  for (int i = 1; i < BUFFER_SIZE; i++) {
    int x1 = i - 1;
    int y1 = ecgBuffer[(bufferIndex + i - 1) % BUFFER_SIZE];
    int x2 = i;
    int y2 = ecgBuffer[(bufferIndex + i) % BUFFER_SIZE];
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // Labels
  display.setCursor(0, 0);
  display.println("MINI ECG       IO");

  display.setCursor(0, 54);
  display.print("BPM: ");
  display.print(bpm);
  display.setCursor(0,52);
  display.print(rawValue);

  display.display();

  delay(10);  // ~100 Hz sampling
}
