#include <SPI.h>
#include <MFRC522.h>


#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
TinyGsm modem(Serial);
TinyGsmClient client(modem);
HttpClient http = HttpClient(client, "missing url", 80); // Replace missing url with actual url

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 6
#define LED_GREEN 8
#define LED_BLUE 7

MFRC522 reader(SS_PIN, RST_PIN);
String card = "";

void setup() {
  // We are using the hardware serial 
  Serial.begin(9600);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.println(F("HOTEL CASHLESS PAYMENT"));
  delay(2000);

  Serial.println(F("Connecting to Internet ..."));
  if (modem.gprsConnect("", "", "")) {
    Serial.print(" success");
    delay(1000);
  }
}

void loop() {
   if (!reader.PICC_IsNewCardPresent()) {
      return false;
    }
    if (!reader.PICC_ReadCardSerial()) {
      return false;
    }

    for (byte i = 0; i < reader.uid.size; i++) {
    card += String(reader.uid.uidByte[i], DEC);
  }

    card = card.substring(0, 8);
    Serial.println("Tag ID: " + card);

    reader.PICC_HaltA();
  

    Serial.print(F("Setting buzzer High to confirm read"));
    digitalWrite(BUZZER, HIGH);
    delay(3000);
    digitalWrite(BUZZER, LOW);

    if (modem.isGprsConnected()) {
      Serial.print(F("Setting Up blue green led to indicate connection"));
      digitalWrite(LED_GREEN, HIGH);

      Serial.println("Making POST request...");

      String contentType = "application/json";
      String postData = GenerateJsonData();

      int err = http.post("/", contentType, postData);

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
        Serial.println(F("Successful"));
        digitalWrite(LED_GREEN, HIGH);
      }

      http.stop();
      Serial.println(F("Server disconnected"));
    }
    else {
      Serial.println("Failed to connect to the network ");

      digitalWrite(LED_BLUE, HIGH),
      delay(1000);
      digitalWrite(LED_BLUE, LOW);
      delay(1000);
      digitalWrite(LED_BLUE, HIGH),
      delay(1000);
      digitalWrite(LED_BLUE, LOW);
      delay(1000);
    }
  }


String GenerateJsonData() {
  String type = "processPayment";
  String aux = "{\n\t\"type\":\"";
  aux.concat(String(type));
  aux.concat("\",\n\t\"card_number\":\"");
  aux.concat(String(card));
  aux.concat("\n}");
  return aux;
}


