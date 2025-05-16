#include <LiquidCrystal.h>
#include <DHT.h>

// Definições dos pinos
const int sensorLuz = A0;       // LDR no pino A0
const int pinoDHT = A1;         // DHT11 no pino A1
const int ledVerde = 12;        // LED verde no pino 12
const int ledAmarelo = 11;      // LED amarelo no pino 11
const int ledVermelho = 10;     // LED vermelho no pino 10
const int buzzer = 9;           // Buzzer no pino 9

// Configuração do DHT11
#define DHTTYPE DHT11
DHT dht(pinoDHT, DHTTYPE);

// Limiares de luminosidade
const int limiarBaixaLuz = 400;   // Abaixo disso = baixa luz
const int limiarMediaLuz = 600;   // Entre 400-600 = média luz

// Limiares de temperatura para vinhedo
const float tempMinimaIdeal = 10.0;
const float tempMaximaIdeal = 15.0;
const float tempMinimaCritica = 5.0;
const float tempMaximaCritica = 20.0;

// Limiares de umidade
const int umidMinimaCritica = 45;
const int umidMinimaAlerta = 50;
const int umidMaximaAlerta = 70;
const int umidMaximaCritica = 75;

// Configuração do LCD
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// Variáveis de controle
unsigned long tempoAnterior = 0;
const int intervaloDisplay = 2000;
int telaAtual = 0;

void setup() {
  // Configura os pinos
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Inicializa o DHT11
  dht.begin();
  
  // Inicializa o LCD
  lcd.begin(16, 2);
  lcd.print("Monitor Vinhedo");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  
  analogReference(DEFAULT);
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  // Leitura dos sensores
  int valorLuz = analogRead(sensorLuz);
  
  // Leitura do DHT11 (temperatura e umidade)
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  
  // Verifica se a leitura do DHT11 falhou
  if (isnan(temperatura) {
    temperatura = 25.0; // Valor padrão se falhar
    Serial.println("Erro ao ler temperatura!");
  }
  if (isnan(umidade)) {
    umidade = 60.0; // Valor padrão se falhar
    Serial.println("Erro ao ler umidade!");
  }
  
  // Classificação dos estados
  int estadoLuz = classificarLuminosidade(valorLuz);
  int estadoTemp = classificarTemperatura(temperatura);
  int estadoUmid = classificarUmidade(umidade);

  // Controle dos LEDs e buzzer
  controlarSinalizadores(estadoLuz, estadoTemp, estadoUmid);
  
  // Exibição no LCD
  mostrarNoLCD(valorLuz, temperatura, umidade, estadoLuz, estadoTemp, estadoUmid);
  
  delay(2000); // Intervalo maior para o DHT11
}

int classificarTemperatura(float temp) {
  if (temp >= tempMinimaIdeal && temp <= tempMaximaIdeal) return 0;
  if ((temp >= tempMinimaCritica && temp < tempMinimaIdeal) || 
      (temp > tempMaximaIdeal && temp <= tempMaximaCritica)) return 1;
  return 2;
}

int classificarLuminosidade(int valorLuz) {
  if (valorLuz < limiarBaixaLuz) return 0;
  if (valorLuz < limiarMediaLuz) return 1;
  return 2;
}

int classificarUmidade(int umid) {
  if (umid >= umidMinimaAlerta && umid <= umidMaximaAlerta) return 0;
  if ((umid >= umidMinimaCritica && umid < umidMinimaAlerta) || 
      (umid > umidMaximaAlerta && umid <= umidMaximaCritica)) return 1;
  return 2;
}

void controlarSinalizadores(int estadoLuz, int estadoTemp, int estadoUmid) {
  // Determinar o pior estado entre todos os sensores
  int piorEstado = max(max(estadoLuz, estadoTemp), estadoUmid);
  
  // Primeiro apaga todos os LEDs
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVermelho, LOW);
  
  // Acende apenas o LED correspondente ao pior estado
  switch(piorEstado) {
    case 0: // Tudo OK - verde
      digitalWrite(ledVerde, HIGH);
      break;
    case 1: // Alerta em algum sensor - amarelo
      digitalWrite(ledAmarelo, HIGH);
      break;
    case 2: // Crítico em algum sensor - vermelho
      digitalWrite(ledVermelho, HIGH);
      break;
  }
  
  // Buzzer só toca se algum estado for crítico
  if (piorEstado == 2) {
    tone(buzzer, 1000);
  } else {
    noTone(buzzer);
  }
}

void mostrarNoLCD(int luz, float temp, float umid, int estadoLuz, int estadoTemp, int estadoUmid) {
  if (millis() - tempoAnterior >= intervaloDisplay) {
    tempoAnterior = millis();
    telaAtual = (telaAtual + 1) % 3;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    
    switch (telaAtual) {
      case 0: // Tela de luminosidade
        switch (estadoLuz) {
          case 0:
            lcd.print("Luz: Ideal");
            break;
          case 1:
            lcd.print("Luz: Alerta");
            break;
          case 2:
            lcd.print("Luz: Critica");
            break;
        }
        lcd.setCursor(0, 1);
        lcd.print("Valor: ");
        lcd.print(luz);
        break;
        
      case 1: // Tela de temperatura
        switch (estadoTemp) {
          case 0:
            lcd.print("Temp: Ideal");
            break;
          case 1:
            lcd.print("Temp: Alerta");
            break;
          case 2:
            lcd.print("Temp: Critica");
            break;
        }
        lcd.setCursor(0, 1);
        lcd.print(temp, 1);
        lcd.print("C");
        break;
        
      case 2: // Tela de umidade
        switch (estadoUmid) {
          case 0:
            lcd.print("Umidade: OK");
            break;
          case 1:
            lcd.print("Umidade: Alerta");
            break;
          case 2:
            lcd.print("Umidade: Critica");
            break;
        }
        lcd.setCursor(0, 1);
        lcd.print(umid, 1);
        lcd.print("%");
        break;
    }
  }
}