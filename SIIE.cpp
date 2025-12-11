#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10     // SDA no MFRC522
#define RST_PIN 9     // RST no MFRC522
#define LED_VERDE 7   // LED para entrada
#define LED_VERMELHO 6 // LED para erro
#define BUZZER 5      // Buzzer para feedback

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Cria instância MFRC522

void setup() {
  Serial.begin(9600);     // Inicia comunicação serial
  SPI.begin();            // Inicia SPI bus
  mfrc522.PCD_Init();     // Inicia MFRC522
  
  // Configurar pinos dos LEDs e buzzer
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Mostra informações do leitor
  mfrc522.PCD_DumpVersionToSerial();
  
  Serial.println("SIIE RFID Reader Ready");
  Serial.println("Scan RFID Card...");
  
  // Feedback de inicialização
  digitalWrite(LED_VERDE, HIGH);
  tone(BUZZER, 1000, 100);
  delay(200);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  tone(BUZZER, 1500, 100);
  delay(200);
  digitalWrite(LED_VERMELHO, LOW);
}

void loop() {
  // Verifica se há novo cartão
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Verifica se o cartão pode ser lido
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  // Formata o UID para envio
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    // Formato hexadecimal com 2 dígitos
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(mfrc522.uid.uidByte[i], HEX);
    
    if (i < mfrc522.uid.size - 1) {
      uidString += " ";
    }
  }
  uidString.toUpperCase();
  
  // Envia UID via Serial
  Serial.println(uidString);
  
  // Feedback de leitura
  tone(BUZZER, 800, 50);
  digitalWrite(LED_VERDE, HIGH);
  delay(0001);
  digitalWrite(LED_VERDE, LOW);
  
  // Espera por feedback do Python
  unsigned long startTime = millis();
  bool feedbackRecebido = false;
  
  while (millis() - startTime < 3000) {  // Espera até 3 segundos
    if (Serial.available() > 0) {
      String feedback = Serial.readStringUntil('\n');
      feedback.trim();
      
      if (feedback == "ENTRADA_OK") {
        // Feedback para entrada registrada
        digitalWrite(LED_VERDE, HIGH);
        tone(BUZZER, 1000, 200);
        delay(500);
        digitalWrite(LED_VERDE, LOW);
      } 
      else if (feedback == "SAIDA_OK") {
        // Feedback para saída registrada
        digitalWrite(LED_VERDE, HIGH);
        tone(BUZZER, 1500, 200);
        delay(500);
        digitalWrite(LED_VERDE, LOW);
      }
      else if (feedback == "NAO_CADASTRADO" || feedback == "SEM_DONO" || feedback == "INATIVO") {
        // Feedback para erro
        digitalWrite(LED_VERMELHO, HIGH);
        tone(BUZZER, 500, 500);
        delay(1000);
        digitalWrite(LED_VERMELHO, LOW);
      }
      else if (feedback == "ERRO_HORA" || feedback == "ERRO_CALCULO" || feedback == "ERRO_GERAL") {
        // Feedback para erro genérico
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_VERMELHO, HIGH);
          tone(BUZZER, 300, 100);
          delay(150);
          digitalWrite(LED_VERMELHO, LOW);
          delay(100);
        }
      }
      else if (feedback == "TEST") {
        // Resposta para teste de conexão
        Serial.println("ARDUINO_OK");
      }
      
      feedbackRecebido = true;
      break;
    }
    delay(10);
  }
  
  // Se não recebeu feedback, sinaliza timeout
  if (!feedbackRecebido) {
    digitalWrite(LED_VERMELHO, HIGH);
    tone(BUZZER, 200, 300);
    delay(500);
    digitalWrite(LED_VERMELHO, LOW);
  }
  
  // Aguarda antes de ler próximo cartão (evita múltiplas leituras)
  delay(2000);
  
  // Finaliza leitura
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
