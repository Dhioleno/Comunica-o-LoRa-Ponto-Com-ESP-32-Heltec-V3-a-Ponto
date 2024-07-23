#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "string.h"
#include "stdio.h"
#include "HT_SSD1306Wire.h"
#include "images.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Configurações do display SSD1306
extern SSD1306Wire display;

// Configurações de rede Wi-Fi
const char* SSID = "Dhionatan-NetOnda-308275-2G";  // Nome da rede Wi-Fi
const char* PASSWORD = "HB209056";  // Senha da rede Wi-Fi

// Configurações do servidor MQTT
const char* MQTT_SERVER = "01b82b48968f435cbedad63e9d31480c.s1.eu.hivemq.cloud";  // Endereço do servidor MQTT
const int MQTT_PORT = 8883;  // Porta segura MQTT (TLS)
const char* MQTT_USERNAME = "hivemq.webclient.1699300830577";  // Nome de usuário MQTT
const char* MQTT_PASSWORD = "G3iv2dHD&#O1?6,McefR";  // Senha MQTT

// Configurações do LoRa
#define RF_FREQUENCY 927000000  // Frequência de operação do LoRa
#define LORA_BANDWIDTH 0  // Largura de banda do LoRa
#define LORA_SPREADING_FACTOR 11  // Fator de espalhamento do LoRa
#define LORA_CODINGRATE 1  // Taxa de codificação do LoRa
#define LORA_PREAMBLE_LENGTH 8  // Comprimento do preâmbulo
#define LORA_SYMBOL_TIMEOUT 0  // Timeout de símbolo
#define LORA_FIX_LENGTH_PAYLOAD_ON false  // Payload de comprimento fixo
#define LORA_IQ_INVERSION_ON false  // Inversão de IQ
#define RX_TIMEOUT_VALUE 1000  // Tempo de timeout para RX
#define BUFFER_SIZE 256  // Tamanho do buffer para dados recebidos

// Certificado raiz para conexão segura
const char ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// Variáveis globais
char rxpacket[BUFFER_SIZE];  // Buffer para armazenar dados recebidos via LoRa
bool lora_idle = true;  // Flag para indicar se o LoRa está ocioso
bool isWifiConnected = false;  // Flag para verificar se a conexão Wi-Fi está ativa
int rssiValue = 0;  // Valor do RSSI (Indicador de Intensidade do Sinal Recebido)
int rxSize = 0;  // Tamanho dos dados recebidos
unsigned long tempo1 = 0;  // Tempo inicial para cálculo do tempo de envio
unsigned long tempo2 = 0;  // Tempo final para cálculo do tempo de envio
unsigned long Tenvio;  // Tempo total de envio
int chave = 0;  // Controle para alternar entre dois estados de recebimento

WiFiClientSecure espClient;  // Cliente Wi-Fi seguro para conexão MQTT
PubSubClient client(espClient);  // Cliente MQTT
RadioEvents_t RadioEvents;  // Estrutura de eventos do rádio LoRa

// Função para exibir o logotipo no display
void logo()
{
    display.clear();  // Limpar o display
    display.drawXbm(0, 5, logo_width, logo_height, logo_bits);  // Desenhar o logotipo
    display.display();  // Atualizar o display
    delay(500);
}

// Função de inicialização
void setup()
{
    logo();  // Mostrar o logotipo
    display.drawString(0, 0, "Heltec.LoRa Initial success!");  // Mensagem inicial
    display.drawString(0, 10, "Iniciando conexão LoRa...");  // Mensagem de início da conexão LoRa
    display.drawString(0, 20, "Iniciando conexão WiFi...");  // Mensagem de início da conexão Wi-Fi
    display.drawString(0, 30, "Iniciando conexão MQTT...");  // Mensagem de início da conexão MQTT
    display.display();  // Atualizar o display
    delay(1000);  // Aguardar 1 segundo

    Serial.begin(9600);  // Inicializar a comunicação serial
    Serial.println("Conectando à rede WiFi...");  // Mensagem de início da conexão Wi-Fi
    WiFi.begin(SSID, PASSWORD);  // Iniciar a conexão com a rede Wi-Fi
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);  // Aguardar 1 segundo
        Serial.println("Tentando se conectar à rede WiFi...");  // Mensagem de tentativa de conexão
    }
    Serial.println("Conectado à rede WiFi");  // Mensagem de sucesso na conexão Wi-Fi

    Serial.println("Carregando certificado raiz...");  // Mensagem de carregamento do certificado
    espClient.setCACert(ROOT_CA);  // Configurar o certificado raiz para a conexão segura
    client.setServer(MQTT_SERVER, MQTT_PORT);  // Configurar o servidor MQTT e a porta
    client.setCallback(callback);  // Definir a função de callback para mensagens MQTT

    while (!client.connected())
    {
        Serial.println("Conectando ao servidor MQTT...");  // Mensagem de tentativa de conexão MQTT
        if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD))
        {
            Serial.println("Conectado ao servidor MQTT");  // Mensagem de sucesso na conexão MQTT
        }
        else
        {
            Serial.print("Falha na conexão ao servidor MQTT, rc=");
            Serial.print(client.state());  // Mensagem de erro na conexão MQTT
            Serial.println(" Tentando novamente em 5 segundos...");
            delay(5000);  // Aguardar 5 segundos antes de tentar novamente
        }
    }

    RadioEvents.RxDone = OnRxDone;  // Definir a função a ser chamada quando um pacote for recebido
    Radio.Init(&RadioEvents);  // Inicializar o rádio LoRa com os eventos configurados
    Radio.SetChannel(RF_FREQUENCY);  // Definir a frequência do canal LoRa
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);  // Configurar parâmetros de recepção do LoRa

    isWifiConnected = false;  // Inicializar a flag de conexão Wi-Fi como falsa
}

// Função principal de loop
void loop()
{
    if (lora_idle)
    {
        delay(1000);  // Aguardar 1 segundo
        Serial.println("Aguardando dados LoRa...");  // Mensagem de espera por dados LoRa
        lora_idle = false;  // Definir a flag para indicar que o LoRa não está ocioso
        Serial.println("Entrando no modo RX LoRa...");  // Mensagem de início do modo RX
        Radio.Rx(0);  // Iniciar o modo de recepção LoRa
    }

    Radio.IrqProcess();  // Processar interrupções do rádio LoRa
    client.loop();  // Manter a conexão e processar mensagens MQTT

    // Verificar se a conexão MQTT está ativa
    if (client.connected())
    {
        display.setTextAlignment(TEXT_ALIGN_LEFT);  // Definir o alinhamento do texto
        display.setFont(ArialMT_Plain_10);  // Definir a fonte do texto
        display.drawString(0, 42, "Conexão MQTT: Ativa");  // Mensagem de status da conexão MQTT
        display.display();  // Atualizar o display
    }
    else
    {
        // Tentar reconectar ao servidor MQTT se a conexão estiver inativa
        if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD))
        {
            Serial.println("Conectado ao servidor MQTT");  // Mensagem de sucesso na conexão MQTT
        }
        else
        {
            Serial.print("Falha na conexão ao servidor MQTT, rc=");
            Serial.print(client.state());  // Mensagem de erro na conexão MQTT
            Serial.println(" Tentando novamente em 5 segundos...");
            delay(5000);  // Aguardar 5 segundos antes de tentar novamente
        }
    }

    // Verificar e manter a conexão Wi-Fi
    if (WiFi.status() == WL_CONNECTED)
    {
        display.setTextAlignment(TEXT_ALIGN_LEFT);  // Definir o alinhamento do texto
        display.setFont(ArialMT_Plain_10);  // Definir a fonte do texto
        display.drawString(0, 51, "Conexão WiFi: Ativa");  // Mensagem de status da conexão Wi-Fi
        display.display();  // Atualizar o display
        isWifiConnected = true;  // Definir a flag de conexão Wi-Fi como verdadeira
    }
    else
    {
        Serial.println("Conectando à rede WiFi...");  // Mensagem de tentativa de conexão Wi-Fi
        WiFi.begin(SSID, PASSWORD);  // Recomeçar a conexão com a rede Wi-Fi
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(1000);  // Aguardar 1 segundo
            Serial.println("Tentando se conectar à rede WiFi...");  // Mensagem de tentativa de conexão
            Serial.println(WiFi.SSID());  // Exibir o SSID da rede Wi-Fi
        }
        Serial.println("Conectado à rede WiFi");  // Mensagem de sucesso na conexão Wi-Fi
    }
}

// Função chamada quando um pacote LoRa é recebido
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Tenvio = 0;  // Inicializar o tempo de envio
    String tempStr;  // String para armazenar a temperatura recebida
    String humStr;  // String para armazenar a umidade recebida
    String soilStr;  // String para armazenar a umidade do solo recebida

    if (chave == 0)
    {
        tempo1 = millis();  // Registrar o tempo inicial
        rxSize = size;  // Armazenar o tamanho dos dados recebidos
        memcpy(rxpacket, payload, size);  // Copiar os dados recebidos para o buffer
        rxpacket[size] = '\0';  // Adicionar o terminador nulo
        rssiValue = rssi;  // Armazenar o valor do RSSI

        // Extrair os valores de temperatura, umidade e umidade do solo dos dados recebidos
        tempStr = getValue(rxpacket, ',', 0);
        humStr = getValue(rxpacket, ',', 1);
        soilStr = getValue(rxpacket, ',', 2);

        Tenvio = tempo1 - tempo2;  // Calcular o tempo total de envio
        Serial.printf("Tempo de envio: %lu ms\n", Tenvio);  // Exibir o tempo de envio
        chave++;
    }
    else
    {
        tempo2 = millis();  // Registrar o tempo final
        rxSize = size;  // Armazenar o tamanho dos dados recebidos
        memcpy(rxpacket, payload, size);  // Copiar os dados recebidos para o buffer
        rxpacket[size] = '\0';  // Adicionar o terminador nulo
        rssiValue = rssi;  // Armazenar o valor do RSSI

        // Extrair os valores de temperatura, umidade e umidade do solo dos dados recebidos
        tempStr = getValue(rxpacket, ',', 0);
        humStr = getValue(rxpacket, ',', 1);
        soilStr = getValue(rxpacket, ',', 2);

        Serial.printf("Tempo inicial2: %lu ms\n", tempo1);  // Exibir o tempo inicial
        Serial.printf("Tempo final2: %lu ms\n", tempo2);  // Exibir o tempo final
        Tenvio = tempo2 - tempo1;  // Calcular o tempo individual de envio
        Serial.printf("Tempo individual: %lu ms\n", Tenvio);  // Exibir o tempo individual de envio
        chave--;
    }

    float temperature = tempStr.toFloat();  // Converter a temperatura para float
    float humidity = humStr.toFloat();  // Converter a umidade para float
    int soilMoisture = soilStr.toInt();  // Converter a umidade do solo para inteiro

    // Publicar os dados recebidos nos tópicos MQTT correspondentes
    if (client.connected())
    {
        client.publish("TEMPERATURA", tempStr.c_str());  // Publicar temperatura
        client.publish("UMIDADE", humStr.c_str());  // Publicar umidade
        client.publish("UMIDADE_SOLO", String(soilMoisture).c_str());  // Publicar umidade do solo
        Serial.printf("Dados publicados: Temperatura=%f, Umidade=%f, Umidade do Solo=%d\n", temperature, humidity, soilMoisture);  // Exibir os dados publicados
    }

    lora_idle = true;  // Definir a flag para indicar que o LoRa está ocioso
}

// Função auxiliar para obter um valor de uma string separada por vírgulas
String getValue(String data, char separator, int index)
{
    int startIndex = 0;
    int endIndex = data.indexOf(separator);
    for (int i = 0; i < index; i++)
    {
        startIndex = endIndex + 1;
        endIndex = data.indexOf(separator, startIndex);
    }
    if (endIndex == -1)
    {
        endIndex = data.length();
    }
    return data.substring(startIndex, endIndex);
}

// Função de callback para mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Mensagem recebida no tópico: ");
    Serial.println(topic);  // Exibir o tópico da mensagem
    Serial.print("Mensagem: ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);  // Exibir o conteúdo da mensagem
    }
    Serial.println();  // Nova linha após a mensagem
}
