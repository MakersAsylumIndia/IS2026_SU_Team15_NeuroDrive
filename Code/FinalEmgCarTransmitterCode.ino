// EMG Transmitter — ESP32 WROOM-32 + AD8232
// WiFi station mode — connects to receiver hotspot, sends UDP commands
//
// AD8232 OUTPUT -> GPIO34
// AD8232 LO-    -> GPIO4
// AD8232 LO+    -> GPIO5
//
// Peak-to-peak over 50 samples, 100ms delay:
//


#include <WiFi.h>
#include <WiFiUdp.h>

const char* AP_SSID     = "EMG_CAR";
const char* AP_PASS     = "12345678";
const char* RECEIVER_IP = "192.168.4.1";
const int   UDP_PORT    = 4210;

WiFiUDP udp;

const int EMG_PIN  = 34;
const int LO_MINUS =  4;
const int LO_PLUS  =  5;

const int   WINDOW_SIZE      = 40;
const float RELAXED_MAX      = 250.0;
const float WEAK_MIN         = 500.0;
const float WEAK_MAX         = 800.0;
const float STRONG_MIN       = 1050.0;
const float STRONG_MAX       = 1800.0;

int window[WINDOW_SIZE];
int windowIndex = 0;
bool windowFull = false;

char lastCommand = 'S';

// ======================================================
// Send command only on state change
// ======================================================
void sendCommand(char cmd)
{
  if (cmd != lastCommand)
  {
    udp.beginPacket(RECEIVER_IP, UDP_PORT);
    udp.write((uint8_t)cmd);
    udp.endPacket();

    lastCommand = cmd;
    Serial.print("[SENT] "); Serial.println(cmd);
  }
}

// ======================================================
// Compute peak-to-peak of rolling window
// ======================================================
float computePeakToPeak()
{
  int count = windowFull ? WINDOW_SIZE : windowIndex;
  if (count == 0) return 0;

  int maxVal = window[0];
  int minVal = window[0];

  for (int i = 1; i < count; i++)
  {
    if (window[i] > maxVal) maxVal = window[i];
    if (window[i] < minVal) minVal = window[i];
  }

  return maxVal - minVal;
}

// ======================================================
// Setup
// ======================================================
void setup()
{
  Serial.begin(115200);
  pinMode(LO_MINUS, INPUT);
  pinMode(LO_PLUS,  INPUT);

  WiFi.begin(AP_SSID, AP_PASS);
  Serial.print("Connecting to EMG_CAR hotspot");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  udp.begin(UDP_PORT);

  Serial.println("RELAXED     = Stop");
  Serial.println("WEAK FLEX   = Anticlockwise");
  Serial.println("STRONG FLEX = Forward");
  Serial.println("System Running...");
}

// ======================================================
// Main Loop
// ======================================================
void loop()
{
  if (digitalRead(LO_PLUS) == HIGH || digitalRead(LO_MINUS) == HIGH)
  {
    Serial.println("LEADS OFF — check electrodes");
    sendCommand('S');
    delay(100);
    return;
  }

  window[windowIndex] = analogRead(EMG_PIN);
  windowIndex++;

  if (windowIndex >= WINDOW_SIZE)
  {
    windowIndex = 0;
    windowFull  = true;
  }

  float peakToPeak = computePeakToPeak();

  Serial.println(peakToPeak);

  if (peakToPeak >= STRONG_MIN && peakToPeak <= STRONG_MAX)
  {
    Serial.println("STRONG FLEX — ANTICLOCKWISE");
    sendCommand('L');;
  }
  else if (peakToPeak >= WEAK_MIN && peakToPeak <= WEAK_MAX)
  {
    Serial.println("Weak FLEX — FORWARD");
    sendCommand('F');
  }
  else if (peakToPeak <= RELAXED_MAX)
  {
    sendCommand('S');
  }

  delay(100);
}