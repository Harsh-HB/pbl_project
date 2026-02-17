#include <SoftwareSerial.h>

#define RELAY_PIN 8

SoftwareSerial sim(10, 11);

const char alertNumber[] = "+919755498881";
const char alertMsg[] =
"Alert someone is trying to turn on the car. Send \"off\" to turn off the car";

bool alertActive = true;
unsigned long lastAlertTime = 0;
bool gsmBusy = false;

void sendSMS(const char* number, const char* text) {
  gsmBusy = true;

  sim.println("AT+CMGF=1");
  delay(200);

  sim.print("AT+CMGS=\"");
  sim.print(number);
  sim.println("\"");
  delay(300);

  sim.print(text);
  sim.write(26);        // CTRL+Z
  delay(3000);          // GSM TX time

  gsmBusy = false;
}

void setup() {
  Serial.begin(115200);
  sim.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);   // relay ON at startup

  delay(3000);

  sim.println("AT+CMGF=1");
  delay(300);
  sim.println("AT+CNMI=2,2,0,0,0");
  delay(300);

  sendSMS(alertNumber, alertMsg);
  lastAlertTime = millis();

  Serial.println("ALERT SYSTEM ACTIVE");
}

void loop() {

  /* ---------- HANDLE INCOMING SMS (PRIORITY) ---------- */
  if (sim.available()) {

    gsmBusy = true;          // stop alerts temporarily
    delay(300);              // allow full SMS to arrive

    String sms = "";
    while (sim.available()) {
      sms += (char)sim.read();
    }

    gsmBusy = false;

    sms.toLowerCase();

    Serial.println("---- SMS RECEIVED ----");
    Serial.println(sms);

    if (sms.indexOf("off") != -1) {
      digitalWrite(RELAY_PIN, LOW);
      alertActive = false;
      Serial.println("CAR OFF - ALERT STOPPED");
    }

    if (sms.indexOf("loc") != -1) {
      sendSMS(alertNumber, "GPS not fixed yet");
      Serial.println("LOC command received");
    }
  }

  /* ---------- PERIODIC ALERT ---------- */
  if (alertActive && !gsmBusy) {
    if (millis() - lastAlertTime >= 10000) {
      sendSMS(alertNumber, alertMsg);
      lastAlertTime = millis();
      Serial.println("Alert SMS sent");
    }
  }
}
