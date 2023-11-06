#include <SPI.h>
#include <MFRC522.h>

#include <SoftwareSerial.h>
SoftwareSerial sim800(2, 4);

#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
TinyGsm modem(sim800);
TinyGsmClient client(modem);
HttpClient http = HttpClient(client, "bunihub.herokuapp.com", 80);

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 6
#define LED_RED 8
#define LED_GREEN 7
MFRC522 reader(SS_PIN, RST_PIN);

String tagID = "";

void setup() {
  Serial.begin(9600);

  SPI.begin();
  reader.PCD_Init();

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);


  digitalWrite(LED_GREEN, HIGH);
  delay(2000);


  Serial.print("HOTEL CASHLESS SYSTEM");
}

void loop() {
  unsigned long savedtime1 = 0;
  unsigned long savedtime2 = 0;
  const long delay1 = 1000;
  const long delay2 = 5000;

  unsigned long currenttime = millis();
  if (currenttime - savedtime1 >= delay1) {
    savedtime1 = delay1;
    int success_read = getID();
    Serial.println("Waiting for scan");
    if (success_read == 1) {
      digitalWrite(BUZZER, HIGH);
      delay(1000);
      digitalWrite(BUZZER, LOW);
      delay(1000);
    }

    if (currenttime - savedtime2 >= delay2) {
      savedtime2 = delay2;
      Serial.println("Connecting to the Internet");
      modem.gprsConnect("", "", "");

      if (modem.isGprsConnected()) {
        Serial.println("Making POST request...");

        String contentType = "application/json";
        String postData = GenerateJsonData();
        Serial.println(postData);
        int err = http.post("/bus", contentType, postData);

        if (err != 0) {
          Serial.println(F("failed to connect"));
          tone(BUZZER, 10000, 500);
          delay(1000);
          noTone(BUZZER);
          delay(1000);
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
          digitalWrite(LED_RED, HIGH);
          delay(3000);
          digitalWrite(LED_RED, LOW);
          delay(3000);
        }
      } else {
        tone(BUZZER, 10000, 500);
        delay(1000);
        noTone(BUZZER);
        delay(1000);
      }
    }
  }
}


int getID() {
  if (!reader.PICC_IsNewCardPresent()) {
    return 0;
  }

  if (!reader.PICC_ReadCardSerial()) {
    return 0;
  }

  for (byte i = 0; i < reader.uid.size; i++) {
    tagID += String(reader.uid.uidByte[i], DEC);
  }
  tagID = tagID.substring(0, 8);
  Serial.println("Tag ID: " + tagID);

  Serial.println("");
  reader.PICC_HaltA();
  return 1;
}

String GenerateJsonData() {
  String type = "processPayment";

  String aux = "{\n\t\"type\":\"";
  aux.concat(String(type));
  aux.concat("\",\n\t\"card_number\":\"");
  aux.concat(String(tagID));
  aux.concat("\n}");
  return aux;
}