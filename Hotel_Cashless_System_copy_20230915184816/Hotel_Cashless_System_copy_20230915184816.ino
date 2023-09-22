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
    digitalWrite(BUZZER, HIGH);
    delay(2000);
    Serial.print(F("Buzzer OFF"));
    digitalWrite(BUZZER, LOW);
    delay(2000);

    String jsonData = GenerateJsonData();
    String apn = "";
    String apn_u = "";
    String apn_p = "";
    String url = "http://unieats.000webhostapp.com/api/index.php";

sim800.write("AT\r");
  printresponse(4000);
  sim800.write("AT+SAPBR=3,1,Contype,GPRS");
  printresponse(100);
  sim800.print("AT+SAPBR=3,1,APN," + apn);
  printresponse(100);
  sim800.print("AT+SAPBR=3,1,USER," + apn_u);
  printresponse(100);
  sim800.print("AT+SAPBR=3,1,PWD," + apn_p);
  printresponse(100);
  sim800.print("AT+SAPBR=1,1");
  printresponse(100);
  sim800.print("AT+SAPBR=2,1");
  printresponse(2000);
  sim800.print("AT+HTTPINIT");
  printresponse(100);
  sim800.print("AT+HTTPPARA=CID,1");
  printresponse(100);
  sim800.print("AT+HTTPPARA=URL," + url);
  printresponse(100);
  sim800.print("AT+HTTPPARA=CONTENT-application/json," + url);
  printresponse(500);
  sim800.print("params=" + jsonData);
  printresponse(10000);
  sim800.print("AT+HTTPACTION=1");
  printresponse(5000);
  sim800.print("AT+HTTPREAD");
  printresponse(100);
  sim800.print("AT+HTTPTERM");
  printresponse(100);

  // Sending sms as the risiti

}

String GenerateJsonData() {
  String type = "processPayment";

  String aux = "{\n\t\"type\":\"";
  aux.concat(String(type));
  aux.concat("\",\n\t\"card_number\":\"");
  aux.concat(String(uid));
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

    Serial.println(uid);
    for (uint8_t i = 0; i < 4; i++) {
      uid.concat(String(reader.uid.uidByte[i], DEC));
    }
    uid.toUpperCase();
    reader.PICC_HaltA();
    return true;
  }
}

void printresponse(int del) {
  int status_code;
  if (sim800.available()>0) {
    Serial.println(sim800.readString());
    char c = sim800.read();
    status_code += c
  }

  delay(del);

  if (response == 200){
    digitalWrite(LED_GREEN HIGH);
    delay(5000);
  }
}
