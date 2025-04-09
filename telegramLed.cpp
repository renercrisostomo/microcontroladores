#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

/* Credenciais wi-fi */
#define ssid_wifi "ifce-alunos"      /* Coloque aqui o nome da rede wiifi que o ESP32 deve se conectar */
#define password_wifi "ifce-4lun0s"  /* Coloque aqui a senha da rede wiifi que o ESP32 deve se conectar */

/* Definições do Telegram */
#define TEMPO_ENTRE_CHECAGEM_DE_MENSAGENS   250 //ms

/* Token de acesso Telegram */
#define TOKEN_TELEGRAM_BOT ""  /* Coloque aqui o seu token de acesso Telegram (fornecido pelo BotFather) */
#define CHAT_ID ""

/* Definições das mensagens possíveis de serem recebidas */
#define CMD_LIGA_LED               "/liga"
#define CMD_DESLIGA_LED            "/desliga"

/* GPIOs usados */
#define GPIO_LED              4

/* Variáveis e objetos globais */
WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN_TELEGRAM_BOT, client);
unsigned long timestamp_checagem_msg_telegram = 0;
int num_mensagens_recebidas_telegram = 0;
String resposta_msg_recebida;

/* Prototypes */
void init_wifi(void);
void conecta_wifi(void);
void verifica_conexao_wifi(void);
unsigned long diferenca_tempo(unsigned long timestamp_referencia);
String trata_mensagem_recebida(String msg_recebida);

/* 
 *  Implementações
 */
 
/* Função: inicializa wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void init_wifi(void) {
    Serial.println("------WI-FI -----");
    Serial.print("Conectando-se a rede: ");
    Serial.println(ssid_wifi);
    Serial.println("Aguarde...");    
    conecta_wifi();
}

/* Função: conecta-se a rede wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void conecta_wifi(void) {
    /* Se ja estiver conectado, nada é feito. */
    if (WiFi.status() == WL_CONNECTED)
        return;

    /* refaz a conexão */
    WiFi.begin(ssid_wifi, password_wifi);
    
    while (WiFi.status() != WL_CONNECTED) 
    {        
        vTaskDelay( 100 / portTICK_PERIOD_MS );
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Conectado com sucesso a rede wi-fi ");
    Serial.println(ssid_wifi);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void verifica_conexao_wifi(void) {
    conecta_wifi(); 
}

/* Função: calcula a diferença de tempo entre o timestamp
 *         de referência e o timestamp atual
 * Parametros: timestamp de referência
 * Retorno: diferença de tempo 
 */
unsigned long diferenca_tempo(unsigned long timestamp_referencia) {
    return (millis() - timestamp_referencia);
}

String trata_mensagem_recebida(String msg_recebida) {
    String resposta = "";
    bool comando_valido = false;

    if (msg_recebida.equals(CMD_LIGA_LED))
    {
        digitalWrite(GPIO_LED, HIGH);
        
        resposta = "Led ligado";
        comando_valido = true;
    }

    if (msg_recebida.equals(CMD_DESLIGA_LED))
    {
        digitalWrite(GPIO_LED, LOW);
        
        resposta = "Led desligado";
        comando_valido = true;
    }

    if (comando_valido == false)
        resposta = "Comando invalido: " + msg_recebida;      
 
    return resposta;
}

void setup() 
{
    Serial.begin(115200);
    
    pinMode(GPIO_LED, OUTPUT);
    
    init_wifi();

    /* Inicializa timestamp de checagem de mensagens recebidas via Telegram */
    timestamp_checagem_msg_telegram = millis();
}

void loop() 
{
    int i;
    
    verifica_conexao_wifi();

    /* Verifica se é hora de checar por mensagens enviadas ao bot Telegram */
    if ( diferenca_tempo(timestamp_checagem_msg_telegram) >= TEMPO_ENTRE_CHECAGEM_DE_MENSAGENS)
    {
        /* Verifica se há mensagens a serem recebidas */
        num_mensagens_recebidas_telegram = bot.getUpdates(bot.last_message_received + 1);

        if (num_mensagens_recebidas_telegram > 0)
        {
            Serial.print("[BOT] Mensagens recebidas: ");
            Serial.println(num_mensagens_recebidas_telegram);
        }
        
        /* Recebe mensagem a mensagem, faz tratamento e envia resposta */
        while(num_mensagens_recebidas_telegram) 
        {
            for (i=0; i<num_mensagens_recebidas_telegram; i++) 
            {                
                resposta_msg_recebida = "";
                resposta_msg_recebida = trata_mensagem_recebida(bot.messages[i].text);
                bot.sendMessage(bot.messages[i].chat_id, resposta_msg_recebida, "");
            }
            
            num_mensagens_recebidas_telegram = bot.getUpdates(bot.last_message_received + 1);
        }
        
        /* Reinicializa timestamp de checagem de mensagens recebidas via Telegram */
        timestamp_checagem_msg_telegram = millis();
    }
}
