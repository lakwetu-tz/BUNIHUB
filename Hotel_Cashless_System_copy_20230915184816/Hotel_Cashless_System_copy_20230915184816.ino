#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
SoftwareSerial sim800(12, 13);

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 6
#define LED_RED 8
#define LED_GREEN 7

MFRC522 reader(SS_PIN, RST_PIN);

bool success_read = false;
String userId = "";

void setup() {
  SPI.begin();
  reader.PCD_Init();

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);
  sim800.begin(9600);

  Serial.println(F("HOTEL CASHLESS PAYMENT SYSTEM"));
  delay(2000);
}

void loop() {
  bool success_read = false;
  success_read = MFRC522_Reader();
  if (success_read == true) {
    Serial.print(F("Buzzer ON"));
    digitalWrite(12, HIGH);
    delay(2000);
    Serial.print(F("Buzzer OFF"));
    digitalWrite(12, LOW);
    delay(2000);

    String jsonData = GenerateJsonData();
    String apn = "";

    sim800.println("AT");
    delay(100);
    sim800.println("AT+CREG?");
    delay(500);
    sim800.println("AT+SAPBR=3,1,Contype,GPRS");
    delay(800);
    sim800.println("AT+SAPBR=3,1,APN," + apn);
    delay(800);
    sim800.println("AT+SAPBR =1,1");
    delay(800);
    sim800.println("AT+SAPBR=2,1");
    delay(800);
    sim800.println("AT+HTTPINIT");
    delay(800);
    sim800.println("AT+HTTPPARA=?");
    sim800.println("AT+HTTPPARA=CID,1");
    delay(800);
    sim800.println("AT+HTTPPARA=URL, http://server-bunihub.heroku.com/api/hotel");
    delay(2000);
    sim800.println("AT+HTTPPARA=CONTENT-TYPE, application/json");
    sim800.print("AT+HTTPDATA=");
    sim800.println(jsonData.length() + 2);  // Add 2 for extra bytes
    delay(100);
    sim800.println(jsonData);
    delay(100);
    sim800.println("AT+HTTPACTION=1");  // Send HTTP POST request
    delay(5000);

    // Sending sms as the risiti

    while (sim800.available() != 0) {
      Serial.write(sim800.read());
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
