#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// Pines del RFID
#define SS_PIN 5
#define RST_PIN 22

// Pines del sensor ultrasónico
#define TRIG_PIN 26
#define ECHO_PIN 27

// Pin del servo conectado al módulo HW-482
#define SERVO_PIN 21

// Umbral para detectar un vehículo (en cm)
#define VEHICLE_THRESHOLD 20

// Instancias
MFRC522 rfid(SS_PIN, RST_PIN);
Servo pluma;

// Lista de UIDs autorizados (puedes agregar más)
const byte tarjetasAutorizadas[][4] = {
  {89, 184, 182, 22},
  {216, 16, 236, 158}
};
const int totalAutorizadas = sizeof(tarjetasAutorizadas) / sizeof(tarjetasAutorizadas[0]);

void setup() {
  Serial.begin(115200);
  Serial.println("Sistema de control de acceso iniciado.");

  // Inicialización de RFID
  SPI.begin(); // SCK=18, MISO=19, MOSI=23
  rfid.PCD_Init();
  Serial.println("Lector RFID listo.");

  // Pines ultrasónico
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Inicialización del servo
  pluma.attach(SERVO_PIN);
  pluma.write(0); // Pluma abajo
  Serial.println("Pluma bajada.");
}

void loop() {
  // Medir distancia
  long distancia = medirDistancia();
  Serial.print("Distancia actual: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Verifica si hay vehículo y tarjeta RFID
  if (distancia < VEHICLE_THRESHOLD) {
    Serial.println("Vehículo detectado. Esperando tarjeta...");

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.print("UID detectado: ");
      printUID(rfid.uid.uidByte, rfid.uid.size);

      if (esTarjetaAutorizada(rfid.uid.uidByte)) {
        Serial.println("Tarjeta autorizada. Abriendo pluma.");

        // Subir pluma
        pluma.write(90);
        delay(2000); // Tiempo para que el vehículo empiece a pasar

        // Esperar a que el vehículo se retire
        while (medirDistancia() < VEHICLE_THRESHOLD) {
          delay(500);
        }

        // Bajar pluma
        pluma.write(0);
        Serial.println("Vehículo retirado. Pluma bajada.");
      } else {
        Serial.println("Tarjeta NO autorizada. Acceso denegado.");
      }

      // Detener comunicación RFID
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      delay(1000); // Breve pausa antes de volver al ciclo
    }
  }

  delay(300); // Ciclo principal
}

// Función para verificar si la tarjeta está autorizada
bool esTarjetaAutorizada(byte *uid) {
  for (int i = 0; i < totalAutorizadas; i++) {
    bool coincide = true;
    for (int j = 0; j < 4; j++) {
      if (tarjetasAutorizadas[i][j] != uid[j]) {
        coincide = false;
        break;
      }
    }
    if (coincide) return true;
  }
  return false;
}

// Función para medir distancia con HC-SR04
long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms
  if (duracion == 0) return 999; // Nada detectado
  return duracion * 0.034 / 2; // Conversión a cm
}

// Imprime el UID en formato decimal
void printUID(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 10 ? "0" : "");
    Serial.print(buffer[i], DEC);
    if (i < bufferSize - 1) Serial.print("-");
  }
  Serial.println();
}
