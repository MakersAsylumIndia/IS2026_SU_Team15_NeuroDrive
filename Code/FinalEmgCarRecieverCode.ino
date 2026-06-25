// EMG Car Receiver — ESP32 WROOM-32 + L298N
//
// L298N IN1 -> GPIO26  (Left motor)
// L298N IN2 -> GPIO27  (Left motor)
// L298N IN3 -> GPIO14  (Right motor)
// L298N IN4 -> GPIO12  (Right motor)
// L298N ENA -> 5V (always enabled)
// L298N ENB -> 5V (always enabled)

#include "BluetoothSerial.h"
BluetoothSerial bt;

void setup() {
  Serial.begin(115200);
  bt.begin("EMG_CAR");
  Serial.print("MAC: ");
  Serial.println(ESP.getEfuseMac(), HEX);
}

void loop() {}

// ======================================================
// Motor Control
// ======================================================

void motorForward()
{
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void motorAnticlockwise()
{
  // Left motor backward, right motor forward → spins anticlockwise
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void motorStop()
{
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ======================================================
// Setup
// ======================================================

void setup()
{
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  motorStop();

  bt.begin("EMG_CAR");
  Serial.println("Bluetooth started. Waiting for connection...");
}

// ======================================================
// Main Loop
// ======================================================

void loop()
{
  if (bt.available())
  {
    char cmd = bt.read();
    Serial.print("[CMD] "); Serial.println(cmd);

    if      (cmd == 'F') { motorForward();       Serial.println("FORWARD");       }
    else if (cmd == 'L') { motorAnticlockwise(); Serial.println("ANTICLOCKWISE"); }
    else if (cmd == 'S') { motorStop();          Serial.println("STOP");          }
  }
}