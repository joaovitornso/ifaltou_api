#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>

#define SDA_PIN   19  //número do pino correspondente a D2
#define SCL_PIN   20  // número do pino correspondente a D1
//#define IRQ_PIN   -1  // número do pino IRQ 
//#define RESET_PIN -1  // número do pino Reset 

#include <ArduinoHttpClient.h>

char serverAddress[] = "192.168.0.127";  // server address
int port = 8000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

const char *ssid = "MariaNoelia";
const char *password = "02030911";

String payload_base = "/?tag=";
String payload = "";

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// Buffer local para armazenar valores da tag
#define BUFFER_SIZE 50
String tagBuffer[BUFFER_SIZE];
int bufferIndex = 0;

void addToBuffer(String tagValue) {
  if (bufferIndex < BUFFER_SIZE) {
    tagBuffer[bufferIndex++] = tagValue;
  } else {
    Serial.println("Buffer cheio. Não é possível adicionar mais leituras.");
  }
}

void clearBuffer() {
  bufferIndex = 0;
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  Serial.println("\nHello! \n");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.println(".");
    attempts++;
    Serial.print("Attempt: ");
    Serial.println(attempts);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to Wi-Fi. Please check your credentials.");
    //while (1);
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  delay(2000);

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    String result = "";

    // Iterar pelos bytes e adicioná-los à string
    for (int i = 0; i < uidLength; i++) {
      // Adicionar cada byte à string como um caractere
      //result += (char)uid[i];
      result += String(uid[i], HEX);
    }
    Serial.print("result: ");
    Serial.println(result);

    payload = payload_base + result;
    //String payload_2 = "{\"tag\":\"" + result + "\"}";

    String endpoint = "/receber-json/";
    String url = endpoint + "?tag=" + result;

    configTime(0, 0, "pool.ntp.org");
    // Adicione este trecho de código para imprimir a hora obtida
    Serial.print("Current time: ");
    Serial.println(now());

    // Obtenha a data e hora atual usando TimeLib
    //tmElements_t tm;
    //breakTime(now(), tm);

    time_t t = now();
    struct tm *tm_info;
    tm_info = localtime(&t);

    // Adicione a data e hora ao objeto JSON
    StaticJsonDocument<200> doc;
    doc["tag_value"] = result;
    doc["year"] = tm_info->tm_year + 1970;
    doc["month"] = tm_info->tm_mon + 1;
    doc["day"] = tm_info->tm_mday;
    doc["hour"] = tm_info->tm_hour;
    doc["minute"] = tm_info->tm_min;
    doc["second"] = tm_info->tm_sec;
    // Converter o objeto JSON em uma string
    String jsonBody;
    serializeJson(doc, jsonBody);

    Serial.println("making GET request");
    Serial.print("Server address: ");
    Serial.println(serverAddress);
    Serial.print("Port: ");
    Serial.println(port);
    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("jsonBody: ");
    Serial.println(jsonBody);
    Serial.print("jsonBody: ");
    Serial.println(jsonBody);
    
    if (client.connect(serverAddress, port)) {
      Serial.println("Connected to server");
     // String request = "GET " + payload + " HTTP/1.1\r\n" +
                 // "Host: " + serverAddress + "\r\n" +
                  //"Connection: close\r\n";


      

      // Iniciar a solicitação POST com o caminho do endpoint
      client.beginRequest();
      client.post(url);

      // Adicionar cabeçalhos adicionais
      client.sendHeader("Content-Type", "application/json");
      client.sendHeader("Content-Length", jsonBody.length());
      //client.sendHeader("Authorization", "Bearer your_access_token");

      // Enviar o corpo da solicitação, se houver
      client.beginBody();
      //client.print("{'tag_value': result }");
      //client.print("{'tag_value':'");
      client.print(jsonBody);
      //client.print("'}");
      client.endRequest();

      //Serial.println("Sending request:\n" + request);
      //Serial.println("Enviando requisição GET para o servidor...");
      //client.print(request);
      //client.post(request);
      // Adicionar uma pausa para garantir que a solicitação seja enviada completamente
      delay(5000);

      Serial.println("Requisição enviada. Aguardando resposta...");
      // Ler e imprimir a resposta do servidor, se houver
      if (client.available()) {
        Serial.println("Dados disponíveis para leitura. Iniciando leitura...");

        // Ler e imprimir a resposta do servidor
        while (client.available()) {
          char c = client.read();
          Serial.print(c);
        }

        Serial.println("Resposta recebida. Fechando conexão...");
        // Limpar o buffer após o envio bem-sucedido
        clearBuffer();
      }  else {
        Serial.println("Nenhum dado disponível para leitura.");
        // Se não houver dados disponíveis, adicione os valores da tag ao buffer
        for (int i = 0; i < uidLength; i++) {
          tagBuffer[i] = String(uid[i], HEX);
        }
        Serial.println("Valores da tag armazenados no buffer para envio posterior.");
        
      }

      
      // Serial.println("Request sent!");

      client.flush();  // Certifique-se de que todos os dados são enviados antes de fechar a conexão
      client.stop(); // Fecha a conexão
      Serial.println("\nConexão encerrada");

    } else {
      Serial.println("Connection to server failed!");
      // Se não houver conexão com o servidor, adicione os valores da tag ao buffer
      for (int i = 0; i < uidLength; i++) {
        tagBuffer[i] = String(uid[i], HEX);
      }
      Serial.println("Valores da tag armazenados no buffer para envio posterior.");
    
    }

    //client.get(payload);
    delay(1000);
 
    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (uidLength == 7)
    {
      uint8_t data[32];

      // We probably have an NTAG2xx card (though it could be Ultralight as well)
      Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");

      // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
      // starting at page 4 ... larger cards just add pages to the end of
      // this range:

      // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf

      // TAG Type       PAGES   USER START    USER STOP
      // --------       -----   ----------    ---------
      // NTAG 203       42      4             39
      // NTAG 213       45      4             39
      // NTAG 215       135     4             129
      // NTAG 216       231     4             225

      for (uint8_t i = 0; i < 42; i++)
      {
        success = nfc.ntag2xx_ReadPage(i, data);

        // Display the current page number
        Serial.print("PAGE ");
        if (i < 10)
        {
          Serial.print("0");
          Serial.print(i);
        }
        else
        {
          Serial.print(i);
        }
        Serial.print(": ");

        // Display the results, depending on 'success'
        if (success)
        {
          // Dump the page data
          nfc.PrintHexChar(data, 4);
        }
        else
        {
          Serial.println("Unable to read the requested page!");
        }
      }
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }

    // Wait a bit before trying again
    Serial.println("\n\nSend a character to scan another tag!");
    Serial.flush();
    while (!Serial.available());
    while (Serial.available()) {
    Serial.read();
    }
    Serial.flush();
  }
}