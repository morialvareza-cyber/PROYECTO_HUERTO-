#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuración LCD I2C (dirección 0x27 para la mayoría de displays)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pines
int bomba = 8;
int pinPO = A1;    // Sensor PH
int pinDO = 6;     // Sensor Oxígeno Disuelto
int pinDH = 7;     // Sensor Temperatura DHT11
int potenciometro = A4; // Potenciómetro para cambiar parámetros en A4

// Variables
int humedad = 0;
int modoDisplay = 0; // 0:Humedad, 1:Temperatura, 2:PH, 3:Estado
int ultimoModo = -1; // Para detectar cambios

void setup() {
  Serial.begin(9600); 
  
  // Configurar pines
  pinMode(bomba, OUTPUT); 
  pinMode(pinDO, INPUT);
  pinMode(pinDH, INPUT);
  pinMode(potenciometro, INPUT);
  
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SISTEMA RIEGO");
  lcd.setCursor(0, 1);
  lcd.print("INICIANDO...");
  
  delay(2000);
  lcd.clear();
}

void loop() {
  // Leer sensores
  humedad = analogRead(A0); 
  int valorPot = analogRead(potenciometro);
  
  // Determinar modo basado en el potenciómetro (4 zonas)
  if (valorPot < 256) {
    modoDisplay = 0; // Humedad
  } else if (valorPot < 512) {
    modoDisplay = 1; // Temperatura
  } else if (valorPot < 768) {
    modoDisplay = 2; // PH
  } else {
    modoDisplay = 3; // Estado
  }
  
  // Control de la bomba
  if(humedad >= 721 && humedad <= 1024) {
    digitalWrite(bomba, LOW); // Apagar bomba (suelo húmedo)
  } else {
    digitalWrite(bomba, HIGH); // Encender bomba (suelo seco)
  }
  
  // Conversión a porcentaje
  int humedadPct = map(humedad, 0, 1023, 100, 0); // Invertido para lógica correcta
  int phVal = analogRead(pinPO);
  int phPct = map(phVal, 0, 1023, 0, 100);
  int doVal = digitalRead(pinDO) * 100; // 0% o 100%
  int temperatura = digitalRead(pinDH) * 100; // 0% o 100%
  
  // Determinar estado de la planta
  bool estadoEstable = (humedadPct >= 30 && humedadPct <= 70) && 
                       (phPct >= 40 && phPct <= 80) && 
                       (temperatura >= 20 && temperatura <= 80);

  // Solo actualizar LCD si cambió el modo
  if (modoDisplay != ultimoModo) {
    lcd.clear();
    ultimoModo = modoDisplay;
  }
  
  // Mostrar en LCD según el modo
  switch(modoDisplay) {
    case 0: // Humedad de suelo
      lcd.setCursor(0, 0);
      lcd.print("HUMEDAD SUELO:");
      lcd.setCursor(0, 1);
      lcd.print(humedadPct);
      lcd.print("%   BOMBA:");
      lcd.print(digitalRead(bomba) ? "OFF " : "ON");
      break;
      
    case 1: // Temperatura
      lcd.setCursor(0, 0);
      lcd.print("TEMPERATURA:");
      lcd.setCursor(0, 1);
      lcd.print(temperatura);
      lcd.print("%");
      break;
      
    case 2: // PH y Oxígeno
      lcd.setCursor(0, 0);
      lcd.print("PH:");
      lcd.print(phPct);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("OXIGENO:");
      lcd.print(doVal);
      lcd.print("%");
      break;
      
    case 3: // Estado general
      lcd.setCursor(0, 0);
      lcd.print("ESTADO PLANTA:");
      lcd.setCursor(0, 1);
      if (estadoEstable) {
        lcd.print("ESTABLE      ");
      } else {
        // Detallar qué falta
        lcd.print("FALTA: ");
        if (humedadPct < 30) lcd.print("AGUA ");
        if (humedadPct > 70) lcd.print("DRENAR ");
        if (phPct < 40 || phPct > 80) lcd.print("PH ");
        if (temperatura < 20 || temperatura > 80) lcd.print("TEMP");
      }
      break;
  }
  
  // Mostrar en Serial
  Serial.print("Modo:");
  Serial.print(modoDisplay);
  Serial.print(" | H:");
  Serial.print(humedadPct);
  Serial.print("% | PH:");
  Serial.print(phPct);
  Serial.print("% | TEMP:");
  Serial.print(temperatura);
  Serial.print("% | BOMBA:");
  Serial.print(digitalRead(bomba) ? "OFF" : "ON");
  Serial.print(" | POT:");
  Serial.print(valorPot);
  Serial.print(" | ESTADO:");
  Serial.println(estadoEstable ? "ESTABLE" : "ATENCION");
  
  delay(500);
}