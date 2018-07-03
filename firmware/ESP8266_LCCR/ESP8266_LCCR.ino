#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <ArduinoJson.h>


#define USE_SERIAL Serial


//ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
StaticJsonDocument<200> doc;

void executaAcaoLed(String led, String estado);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        //webSocket.sendTXT(num, "{Connected");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
      //Inicia parse do json
      DeserializationError error = deserializeJson(doc, payload);
      //se houver erro, sai do laço
      if (error) {
        USE_SERIAL.print(F("deserializeJson() failed: "));
        USE_SERIAL.println(error.c_str());
        break;
      }
      JsonObject root = doc.as<JsonObject>();
      //Analisa o tipo e chama a funcao com os parametros
      if ((root["tipo"].as<String>()).equals("led")) {

       
        executaAcaoLed(root["led"].as<String>(), root["estado"].as<String>());
      }
      break;
  }

}

void setup() {
 
  USE_SERIAL.begin(115200);
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("Inicializando %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  pinMode(LED_BUILTIN, OUTPUT);
  //WiFiMulti.addAP("SSID", "passpasspass");
  USE_SERIAL.println("Iniciando Wifi");
  //while(WiFiMulti.run() != WL_CONNECTED) {
  // USE_SERIAL.println(".");
  //   delay(100);
  //}
  WiFi.softAP("Elimu_LCCR", "elimu123");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Ip da placa: ");
  Serial.println(myIP);
  USE_SERIAL.println("Iniciando Webcsocket");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  //Inicializa serviço mdns
  if (MDNS.begin("esp8266")) {
    USE_SERIAL.println("MDNS iniciou");
  }

  // trata a request, pagina principal redireciona para o scratchx
  server.on("/", []() {
    server.send(200, "text/html", "<html><head><script>document.location.href=\"http://scratchx.org/#scratch\" </script></head><body></body></html>");
  });

  server.begin();

  // define os tipos de servico mDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}
void executaAcaoLed(String led, String estado) {
  boolean estado_b = estado.equals("On");
  if (led.equals("Led 1")) {
     USE_SERIAL.print("eh led interno \n");
     
    digitalWrite(LED_BUILTIN, estado_b);
  }
}

void loop() {
  webSocket.loop();
  server.handleClient();
}
