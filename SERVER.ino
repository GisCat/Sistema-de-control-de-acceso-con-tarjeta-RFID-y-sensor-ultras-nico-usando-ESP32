#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

// Pines de los LEDs (MSB a LSB)
const int L[4] = {13, 14, 26, 33};

// Pin de selección de operación con DIP switch
const int OP_PIN = 22; // LOW = suma, HIGH = resta

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Server"); // Nombre Bluetooth del servidor
  Serial.println("Servidor Bluetooth esperando cliente...");

  pinMode(OP_PIN, INPUT_PULLUP);

  for (int i = 0; i < 4; i++) {
    pinMode(L[i], OUTPUT);
    digitalWrite(L[i], LOW);
  }

  SerialBT.begin("ESP32_MAC_CHECK");
}

void loop() {

    uint8_t mac[6];
  SerialBT.getBtAddress(mac);

  Serial.print("MAC Bluetooth: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");  // Formato 2 dígitos
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  if(!SerialBT.hasClient()) Serial.println("ESPERANDO...");
  
  // Si hay un cliente conectado
  if (SerialBT.hasClient()) {
    if (SerialBT.available()) {
      int num1 = SerialBT.parseInt();
      int num2 = SerialBT.parseInt();

      bool op = digitalRead(OP_PIN); // 0 = suma, 1 = resta
      int res = op ? (num1 - num2) : (num1 + num2);

      Serial.print("Operación: ");
      Serial.println(op ? "Resta" : "Suma");
      Serial.print("Recibido: ");
      Serial.print(num1);
      Serial.print(op ? " - " : " + ");
      Serial.print(num2);
      Serial.print(" = ");
      Serial.println(res);

      if (res < 0 || res > 15) {
        Serial.println("⚠️ Resultado fuera de rango (0–15). No se mostrará.");
        return;
      }

      // Mostrar resultado en LEDs
      for (int i = 3; i >= 0; i--) {
        int bit = (res >> i) & 1;
        digitalWrite(L[3 - i], bit);
      }

      SerialBT.println("Resultado mostrado.");
    }
  }
}
