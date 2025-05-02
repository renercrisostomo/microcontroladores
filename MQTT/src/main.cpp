#include <WiFi.h>
#include <PubSubClient.h>

// Configuracao Wi-Fi
const char* ssid        = "xxx";
const char* password    = "xxx";

// Broker MQTT publico
const char* mqtt_broker   = "broker.mqtt.cool";
const int   mqtt_port     = 1883;
const char* mqtt_user     = "emqx";
const char* mqtt_pass     = "public";

// Topicos MQTT
const char* topicPub = "esp32/pub";
const char* topicSub = "esp32/sub";

// Pino do LED integrado (GPIO 2) e pino de toque (GPIO 4)
const int ledPin      = 2;
const int touchPin    = 4;     
const int touchThresh = 40;    // limiar para deteccao de toque

WiFiClient     espClient;
PubSubClient   client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Mensagem recebida em [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);
  Serial.println("-----------------------");

  // Se receber "LED ON", pisca o LED integrado
  if (msg == "LED ON") {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    Serial.println("LED piscar executado");
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "esp32-";
    clientId += String(WiFi.macAddress());
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado ao broker MQTT");
      client.subscribe(topicSub);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado");
  
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publica mensagem apenas ao detectar toque
  int touchValue = touchRead(touchPin);
  if (touchValue < touchThresh) {  // valor baixo indica toque
    String msg = "Ola do ESP32!";
    client.publish(topicPub, msg.c_str());
    Serial.print("Publicado por toque em [");
    Serial.print(topicPub);
    Serial.print("]: ");
    Serial.println(msg);
    delay(1000); // debounce para evitar multiplas publicacoes
  }
}