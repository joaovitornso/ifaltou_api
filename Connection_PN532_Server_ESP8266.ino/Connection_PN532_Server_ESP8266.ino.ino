#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>
#include <ArduinoHttpClient.h>

#define SDA_PIN   19  // número do pino correspondente a D2
#define SCL_PIN   20  // número do pino correspondente a D1

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

//char serverAddress[] = "192.168.0.127";  // server address
//char serverAddress[] = "192.168.137.1";  // server address
//char serverAddress[] = "10.70.197.193";  // server address
char serverAddress[] = "192.168.232.168";  // server address

int port = 8000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

const char *ssid = "Joao";
const char *password = "123123123";

String payload_base = "/?tag=";
String payload = "";
String buffer = "";

void connectToNetwork();
void handleNetworkConnected(String tag);
void handleNoNetwork(String tag);
void trySendBuffer();
void readTag();

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("\nOlá Mundo!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("\nPlaca PN53x não encontrada");
    while (1);
  }
  connectToNetwork();
  delay(2000);

  Serial.print("Encontrou chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Serial.println("Esperando por um cartão ISO14443A...");
}

bool tagProcessed = true; 



void loop(void) {
  
  // uint8_t success;
  // uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  // uint8_t uidLength;

  // success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  // if (success) {
  //   Serial.println("Cartão ISO14443A encontrado!");
    // delay(1000);
    // Serial.print(" Comprimento do UID: ");
    // Serial.print(uidLength, DEC);
    // Serial.println(" bytes");
    // Serial.print(" Valor UID: ");
    // nfc.PrintHex(uid, uidLength);
    // Serial.println("");

    // String result = "";

    // for (int i = 0; i < uidLength; i++) {
    //   result += String(uid[i], HEX);
    // }

    // Serial.print("result: ");
    // Serial.println(result);

    // if (WiFi.status() == WL_CONNECTED) {
    //   handleInternetConnected(result);
    // } else {
    //   handleNoInternet(result);
    // }

    // tagProcessed = true;

    // delay(1000);

    if (tagProcessed) {
      readTag();
    } else {

      // A cada 30 segundos, tenta enviar dados do buffer para o servidor
      static unsigned long lastBufferSendTime = 0;
      if (millis() - lastBufferSendTime > 30000) {
        trySendBuffer();
        lastBufferSendTime = millis();
      }
      
      // Inicia uma nova leitura
      readTag();
    }
}
  
  // A cada 30 segundos, tenta enviar dados do buffer para o servidor
//     static unsigned long lastBufferSendTime = 0;
//     if (!tagProcessed && millis() - lastBufferSendTime > 30000) {
//       trySendBuffer();
//       lastBufferSendTime = millis();
//     }
// }


void readTag() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  Serial.println("Esperando por um cartão ISO14443A...");
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Cartão ISO14443A encontrado!");
    
    delay(1000);
    Serial.print(" Comprimento do UID: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print(" Valor UID: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    String result = "";

    for (int i = 0; i < uidLength; i++) {
      result += String(uid[i], HEX);
    }

    Serial.print("result: ");
    Serial.println(result);

    if (WiFi.status() == WL_CONNECTED) {
      handleNetworkConnected(result);
    } else {
      handleNoNetwork(result);
    }

    tagProcessed = false;  // Marca a tag como processada
  }
}
void handleNetworkConnected(String tag) {
  payload = payload_base + tag;

  String endpoint = "/api/receber-json/";
  String url = endpoint + "?tag=" + tag;

  configTime(0, 0, "pool.ntp.org");
  time_t t = now();
  struct tm *tm_info;
  tm_info = localtime(&t);

  StaticJsonDocument<200> doc;
  doc["tag_value"] = tag;
  // doc["year"] = tm_info->tm_year + 1970;
  // doc["month"] = tm_info->tm_mon + 1;
  // doc["day"] = tm_info->tm_mday;
  // doc["hour"] = tm_info->tm_hour;
  // doc["minute"] = tm_info->tm_min;
  // doc["second"] = tm_info->tm_sec;
  String jsonBody;
  serializeJson(doc, jsonBody);

  Serial.println("Fazendo Requsisão POST");
  Serial.print("Server address: ");
  Serial.println(serverAddress);
  Serial.print("Port: ");
  Serial.println(port);
  Serial.print("URL: ");
  Serial.println(url);
  Serial.print("jsonBody: ");
  Serial.println(jsonBody);

  if (client.connect(serverAddress, port)) {
    Serial.println("Conectado ao servidor.");

    client.beginRequest();
    client.post(url);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", jsonBody.length());
    client.beginBody();
    client.print(jsonBody);
    client.endRequest();

    delay(5000);

    Serial.println("Requisição enviada. Aguardando resposta...");

    if (client.available()) {
      Serial.println("Dados disponíveis para leitura. Iniciando leitura...");

      while (client.available()) {
        char c = client.read();
        Serial.print(c);
      }

      Serial.println("Resposta recebida. Fechando conexão...");
    } else {
      Serial.println("Nenhum dado disponível para leitura.");

    }

    client.flush();
    client.stop();
    Serial.println("\nConexão encerrada");
  } else {
    Serial.println("Conexão com o servidor falhou!");
    //handleNoInternet(tag);
    noConnectionWithServer(tag);
  }
   tagProcessed = false;
}

void noConnectionWithServer(String tag){
  if (buffer.length() > 0) {
    buffer += ",";
  }
  buffer += tag;

  Serial.println("Sem conexão com o servidor. Adicionando tag ao buffer.");
  Serial.print("Conteúdo do buffer: ");
  Serial.println(buffer);

  trySendBuffer();
  delay(1000);
}
void handleNoNetwork(String tag) {
  if (buffer.length() > 0) {
    buffer += ",";
  }
  buffer += tag;

  Serial.println("Sem conexão com a rede. Adicionando tag ao buffer.");
  Serial.print("Conteúdo do buffer: ");
  Serial.println(buffer);

  tagProcessed = true;

  readTag();
  delay(1000);


}

void trySendBuffer() {
  if (buffer.length() > 0 && WiFi.status() == WL_CONNECTED) {
    Serial.println("Tentando enviar buffer para o servidor...");

    // Adapte este trecho de acordo com o formato que deseja para enviar múltiplas tags
    String bufferData = "{\"tags\":[" + buffer + "]}";

    if (client.connect(serverAddress, port)) {
      Serial.println("Conectado ao servidor para upload de buffer.");

      String endpoint = "/api/send-buffer/";
      String url = endpoint;

      client.beginRequest();
      client.post(url);
      client.sendHeader("Content-Type", "application/json");
      client.sendHeader("Content-Length", bufferData.length());
      client.beginBody();
      client.print(bufferData);
      client.endRequest();
      
      delay(5000);

      Serial.println("Solicitacao de upload de buffer enviada. Aguardando resposta...");

      if (client.available()) {
        Serial.println("Dados disponniveis para leitura. Iniciando leitura...");

        while (client.available()) {
          char c = client.read();
          Serial.print(c);
        }

        Serial.println("Response recebido. Fechando Conexando conexao...");
      } else {
        Serial.println("Sem dados disponiveis para leitura.");
      }

      client.flush();
      client.stop();
      buffer = "";  // Limpa o buffer após o envio bem-sucedido
      Serial.println("\nConexao fechada");
    } else {
      Serial.println("Falha na conexao com o servidor para upload do buffer!");
      setup();
    }
    tagProcessed = true;
  }
}

void connectToNetwork() {
  Serial.println("\nConectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    //Serial.println(".");
    attempts++;
    Serial.print("Tentativa: ");
    Serial.println(attempts);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFalhou ao conectar ao Wi-Fi. Por favor check suas credenciais.");
    //while (1);
  } else {
    Serial.println('\n');
    Serial.println("Conexão estabelecida!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
  }
}
