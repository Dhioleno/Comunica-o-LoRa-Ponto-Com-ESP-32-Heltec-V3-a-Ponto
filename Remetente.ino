#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include "DHT.h"

// Definições de parâmetros LoRa
#define RF_FREQUENCY        927000000  // Frequência de operação (Hz)
#define TX_OUTPUT_POWER     10         // Potência de saída (dBm)
#define LORA_BANDWIDTH      0          // Largura de banda (125 kHz)
#define LORA_SPREADING_FACTOR 11       // Fator de espalhamento (SF11)
#define LORA_CODINGRATE     1          // Taxa de codificação (4/5)
#define LORA_PREAMBLE_LENGTH 8         // Comprimento do preâmbulo
#define LORA_SYMBOL_TIMEOUT 0          // Timeout do símbolo
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define BUFFER_SIZE         50         // Tamanho do buffer de transmissão

// Definições de pinos para sensores
#define HW_390PIN 1    // Pino do sensor de umidade do solo HW-390
#define DHTPIN 2       // Pino do sensor DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT

// Buffer de transmissão
char txpacket[BUFFER_SIZE];
bool lora_idle = true;  // Flag para indicar estado ocioso do LoRa
int soilMoisture = 0;   // Variável para armazenar a umidade do solo
static RadioEvents_t RadioEvents;  // Estrutura de eventos do rádio

// Leituras mínima e máxima do sensor de umidade do solo (calibradas)
const int leituraMinima = 800;  // Valor mínimo calibrado
const int leituraMaxima = 1900; // Valor máximo calibrado
extern SSD1306Wire display;     // Objeto do display SSD1306

DHT dht(DHTPIN, DHTTYPE);  // Objeto do sensor DHT

// Funções de callback para eventos de transmissão LoRa
void OnTxDone(void);
void OnTxTimeout(void);

// Configuração inicial do sistema
void setup() {
    Serial.begin(9600);  // Inicializar comunicação serial
    Mcu.begin();         // Inicializar MCU

    // Inicializar o display OLED
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.drawString(0, 0, "Heltec.LoRa Sender");
    display.display();

    // Configurar eventos de transmissão LoRa
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;

    // Inicializar e configurar o rádio LoRa
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    // Inicializar o sensor DHT
    dht.begin();
    pinMode(HW_390PIN, INPUT);  // Configurar pino do sensor de umidade do solo como entrada
}

// Loop principal
void loop() {
    if (lora_idle) {  // Verificar se o rádio LoRa está ocioso
        delay(1000);  // Aguardar 1 segundo

        // Ler e mapear o valor da umidade do solo
        soilMoisture = analogRead(HW_390PIN);
        float umidadeDoSolo = map(soilMoisture, leituraMaxima, leituraMinima, 0, 100);

        // Ler temperatura e umidade do ar do sensor DHT
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        // Verificar se a leitura do sensor DHT foi bem-sucedida
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Erro ao ler dados do sensor DHT11!");
            return;
        }

        // Formatar a mensagem de transmissão com os dados dos sensores
        sprintf(txpacket, "Temp:%.1fC, UmiAr:%.2f, UmiSolo:%.2f%%", temperature, humidity, umidadeDoSolo);

        // Exibir a mensagem de transmissão no console serial
        Serial.printf("\r\nEnviando pacote \"%s\", comprimento %d\r\n", txpacket, strlen(txpacket));

        // Enviar a mensagem via LoRa
        Radio.Send((uint8_t *)txpacket, strlen(txpacket));
        lora_idle = false;  // Marcar o rádio LoRa como ocupado

        // Atualizar o display OLED com os dados dos sensores
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 15, "Temp: " + String(temperature) + "C");
        display.drawString(0, 30, "UmiAr: " + String(humidity) + "%");
        display.drawString(0, 45, "UmiSolo: " + String(umidadeDoSolo));
        display.display();
    }

    // Processar interrupções do rádio LoRa
    Radio.IrqProcess();
}

// Função de callback chamada quando a transmissão é concluída
void OnTxDone(void) {
    lora_idle = true;  // Marcar o rádio LoRa como ocioso
}

// Função de callback chamada quando ocorre um timeout na transmissão
void OnTxTimeout(void) {
    Radio.Sleep();  // Colocar o rádio LoRa em modo de espera
    Serial.println("Timeout de TX......");
    lora_idle = true;  // Marcar o rádio LoRa como ocioso
}
