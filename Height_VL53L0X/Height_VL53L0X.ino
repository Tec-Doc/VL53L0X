#include <Wire.h>
#include <VL53L0X.h>
#include <Adafruit_SSD1306.h>

// Erstelle ein Objekt für den VL53L0X-Sensor
VL53L0X sensor;

// Erstelle ein Objekt für das OLED-Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Erforderliche Konstanten
const uint8_t MEASUREMENT_ACCURACY = 1; // 1 = hohe Genauigkeit, 0 = schnellere Messung
const uint16_t SENSOR_HEIGHT_MM = 1900; // Höhe des Sensors in mm

// Definiere bekannte Entfernung als Konstante für die Kalibrierung
const uint16_t KNOWN_DISTANCE_MM = 100; // Beispiel: 100mm

// Verzögerung für die Kalibrierung
uint16_t calibrationDelayMs = 5000; // Standardwert

// Variable für das Kalibrierungsoffset
int16_t calibrationOffset = 0;

void setup() {
  // Starte die serielle Kommunikation für Debugging
  Serial.begin(115200);

  // Starte die I2C-Kommunikation
  Wire.begin();

  // Initialisiere das OLED-Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("Fehler: OLED-Display konnte nicht initialisiert werden!");
    while (1);
  }

  // Testanzeige
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Test");
  display.drawRect(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("VL53L0X Init");
  display.display();

  // Initialisiere den Sensor
  if (!sensor.init()) {
    Serial.println("Fehler: Sensor konnte nicht initialisiert werden!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor-Fehler");
    display.display();
    while (1);
  }
  
  // Dynamisch berechneter Timing-Budget-Wert basierend auf gewünschter Messgenauigkeit
  uint32_t timingBudget = calculateTimingBudget(MEASUREMENT_ACCURACY);
  sensor.setMeasurementTimingBudget(timingBudget);

  // Start der Kalibrierung
  calibrateSensor();

  Serial.println("Kalibrierung abgeschlossen.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("CAL OK");
  display.display();
}

void loop() {
  // Kontinuierliche Messungen nach der Kalibrierung
  uint16_t distance = sensor.readRangeSingleMillimeters();

  // Überprüfen, ob eine Messung gültig ist
  display.clearDisplay();
  display.setCursor(0, 0);
  if (sensor.timeoutOccurred()) {
    Serial.println("Messfehler: Zeitüberschreitung!");
    display.println("Fehler");
  } else {
    // Berechnung der Körpergröße
    distance += calibrationOffset; // Offset-Korrektur anwenden
    if (distance < SENSOR_HEIGHT_MM) {
      float personHeightMeters = (SENSOR_HEIGHT_MM - distance) / 1000.0;
      Serial.print("Körpergröße: ");
      Serial.print(personHeightMeters, 2);
      Serial.println(" m");
      display.print("Gr: ");
      display.print(personHeightMeters, 2);
      display.println("m");
    } else {
      Serial.println("Keine Person erkannt.");
      display.println("Keine Pers");
    }
  }
  display.display();
  delay(5000); // Messungen alle 5000ms
}

void calibrateSensor() {
  // Beispiel-Kalibrierung: Setzen von Offsets
  Serial.println("Starte Offset-Kalibrierung...");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Kalib...");
  display.display();

  // Beispiel: Messen einer bekannten Entfernung
  uint16_t measuredDistance;

  Serial.print("Bitte halte ein Objekt in ");
  Serial.print(KNOWN_DISTANCE_MM);
  Serial.println("mm Entfernung...");

  Serial.print("Wartezeit für Positionierung: ");
  Serial.print(calibrationDelayMs);
  Serial.println(" ms");

  display.setCursor(0, 0);
  display.print("Pos: ");
  display.print(KNOWN_DISTANCE_MM);
  display.println("mm");
  display.display();

  delay(calibrationDelayMs); // Zeit geben, das Objekt zu positionieren

  measuredDistance = sensor.readRangeSingleMillimeters();

  if (sensor.timeoutOccurred()) {
    Serial.println("Fehler: Zeitüberschreitung bei der Kalibrierung!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Kalib fehl");
    display.display();
    return;
  }

  calibrationOffset = KNOWN_DISTANCE_MM - measuredDistance;
  Serial.print("Gemessene Entfernung: ");
  Serial.print(measuredDistance);
  Serial.println(" mm");
  Serial.print("Berechneter Offset: ");
  Serial.print(calibrationOffset);
  Serial.println(" mm");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Gem: ");
  display.print(measuredDistance);
  display.println("mm");
  display.setCursor(0, 10);
  display.print("Off: ");
  display.print(calibrationOffset);
  display.println("mm");
  display.display();

  Serial.println("Offset-Kalibrierung abgeschlossen.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Kalib fer");
  display.display();
}

uint32_t calculateTimingBudget(uint8_t accuracyLevel) {
  // Berechnet den Timing-Budget-Wert abhängig von der gewünschten Genauigkeit
  if (accuracyLevel == 1) {
    return 200000; // Hohe Genauigkeit
  } else {
    return 100000; // Schnellere Messung
  }
}
