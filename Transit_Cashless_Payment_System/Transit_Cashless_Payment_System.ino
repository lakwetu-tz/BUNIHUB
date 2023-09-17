#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
SoftwareSerial gprs(12, 13);

#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
TinyGsm modem(gprs);
TinyGsmClient client(modem);
HttpClient http = HttpClient(client, "server-bunihub.herokuapp.com/api/transit", 80);

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 6
#define LED_RED 8
#define LED_GREEN 7

MFRC522 reader(SS_PIN, RST_PIN);

bool success_read = false;
String userId = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  gprs.begin(9600);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.println(F("Transit Payment System"));
  delay(2000);

  Serial.println(F("Connecting to Internet ..."));
  if (modem.gprsConnect("", "", "")) {
    Serial.print(" success");
    delay(1000);
  }
}

void loop() {
  bool success_read = false;
  success_read = MFRC522_Reader();
  if (success_read == true) {
    Serial.print(F("LED_RED ON"));
    Control(LED_RED);

    if (modem.isGprsConnected()) {
      Serial.print(F("LED_GREEN ON"));
      Control(LED_GREEN);

      Serial.println("Making POST request...");

      String contentType = "application/json";
      String postData = GenerateJsonData();

      int err = http.post("/api/bus/", contentType, postData);

      if (err != 0) {
        Serial.println(F("failed to connect"));
        delay(10000);
        return;
      }

      int status = http.responseStatusCode();
      Serial.print(F("Response status code: "));
      Serial.println(status);

      String response = http.responseBody();
      Serial.println(F("Response:"));
      Serial.println(response);

      if (status == 200) {
        Serial.println(F("Setting Buzzer High"));
        Control(BUZZER);
      }

      http.stop();
      Serial.println(F("Server disconnected"));
    }
  }
}

String GenerateJsonData() {
  String fare = "500";
  String aux = "{\n\t\"userId\":\"";
  aux.concat(String(userId));
  aux.concat("\",\n\t\"fare\":\"");
  aux.concat(String(fare));
  aux.concat("\n}");
  return aux;
}

void Control(int comp) {
  digitalWrite(comp, HIGH);
  delay(2000);
  Serial.print(F("LED_RED OFF"));
  digitalWrite(comp, LOW);
  delay(2000);
}

boolean MFRC522_Reader() {
  {
    if (!reader.PICC_IsNewCardPresent()) {
      return false;
    }
    if (!reader.PICC_ReadCardSerial()) {
      return false;
    }
    Serial.println(userId);
    for (uint8_t i = 0; i < 4; i++) {
      userId.concat(String(reader.uid.uidByte[i], DEC));
    }
    userId.toUpperCase();
    reader.PICC_HaltA();
    return true;
  }
}
