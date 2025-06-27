#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

// Pines: [MSB, ..., LSB]
const int switches[4] = {33, 25, 26, 27};
const int sendButton = 35;

int numCount = 0;
int num1 = -1;
int num2 = -1;
bool prevSendState = false;

uint8_t serverMAC[6] = { 0xD4, 0x8C, 0x49, 0xE3, 0x92, 0xC6 }; // tu MAC

void setup() {
  Serial.begin(115200);

  // Activar modo cliente (true)
  SerialBT.begin("ESP32_Client", true); 
  Serial.println("Conectando al servidor...");

  if (SerialBT.connect(serverMAC, 1)) { // ← Asegúrate que esta sea la MAC real del servidor
    Serial.println("Conectado al servidor.");
  } else {
    Serial.println("No se pudo conectar al servidor.");
  }

  for (int i = 0; i <= 3; i++) {
    pinMode(switches[i], INPUT_PULLDOWN);
  }
  pinMode(sendButton, INPUT_PULLDOWN);
}

void loop() {
  if (!SerialBT.connected()) {
    Serial.println("No conectado. Reintenta manualmente o reinicia.");
    delay(1000);
    return;
  }

  bool currentSend = digitalRead(sendButton);

  // Detectar flanco de subida (LOW → HIGH)
  if (currentSend && !prevSendState) {
    int val = 0;

    // Leer switches (MSB a LSB)
    for (int i = 0; i < 4; i++) {
      int bit = digitalRead(switches[i]); // Negamos porque INPUT_PULLUP
      val |= (bit << (3 - i));
    }

    Serial.print("Número leído del DIP: ");
    Serial.println(val);

    if (numCount == 0) {
      num1 = val;
      numCount = 1;
      Serial.print("Primer número guardado: ");
      Serial.println(num1);
    } else if (numCount == 1) {
      num2 = val;
      numCount = 0; // Listos para reiniciar

      Serial.print("Segundo número guardado: ");
      Serial.println(num2);

      // Enviar al servidor
      Serial.print("Enviando: ");
      Serial.print(num1);
      Serial.print(" ");
      Serial.println(num2);

      SerialBT.print(num1);
      SerialBT.print(" ");
      SerialBT.println(num2);
    }
  }

  // Leer mensaje del servidor (opcional)
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    Serial.println("Servidor dice: " + msg);
  }

  prevSendState = currentSend;
  delay(50); // Pausa para evitar rebotes y lecturas excesivas
}

