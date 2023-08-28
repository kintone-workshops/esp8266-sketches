#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "MY_WIFI_SSID";
const char* password = "MY_WIFI_PASSWORD";
DynamicJsonDocument doc(1024);
ESP8266WebServer server(80);
bool result = false;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/post", handlePost);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handle_OnConnect() {
  Serial.println("Welcome. Enter your App ID, API Key, and Subdomain to post some data.");
  server.send(200, "text/html", SendHTML());
}

void handlePost() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  Serial.println("Attempting to Save to Kintone...");
  String subdomain = server.arg("subdomain");
  String appID = server.arg("appID");
  String token = server.arg("API");
  String color = server.arg("color");
  String URL = "https://" + subdomain + ".kintone.com/k/v1/record.json?app=" + appID;
  Serial.print("info:");
  Serial.println(URL);
  http.begin(client, URL);
  doc["app"] = appID;
  JsonObject recordObject = doc.createNestedObject("record");
  recordObject["color"]["value"] = color;
  serializeJsonPretty(doc, Serial);
  String json;
  serializeJson(doc, json);
  int responseCode = 0;
  http.addHeader("X-Cybozu-API-Token", token);
  http.addHeader("Content-type", "application/json");
  responseCode = http.POST(json);
  Serial.printf("http Response Code = %d \n", responseCode);
  String payload = http.getString();
  Serial.println(payload);
  if (responseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(responseCode);
    String payload = http.getString();
    Serial.println(payload);
    result = true;
  } else {
    Serial.print("Error code: ");
    Serial.println(responseCode);
    result = false;
  }
  http.end();
  server.send(200, "text/html", PostResult(result));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Kintone and IOT!</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP8266 Web Server</h1>\n";
  ptr += "<p>Enter your subdomain, appID, and API Token!</p>";
  ptr += "<form action=\"/post\">";
  ptr += "<label for=\"subdomain\">Subdomain:</label><input type=\"text\" id=\"subdomain\" name=\"subdomain\" value=\"example\">";
  ptr += "<label for=\"appID\">App ID:</label><input type=\"text\" id=\"appID\" name=\"appID\" value=\"1\"/>";
  ptr += "<label for=\"API\">API Token:</label><input type=\"text\" id=\"API\" name=\"API\" value=\"1J22qNAR54I4eiMcd0JmfDAavJNfNJDVaqt34X9A\"/><br>";
  ptr += "<label for=\"color\">Pick your favorite Color:</label><br>";
  ptr += "<label for=\"red\">Red</label>";
  ptr += "<input type=\"radio\" id=\"red\" name=\"color\" value=\"Red\" checked /><br>";
  ptr += "<label for=\"blue\">Blue</label>";
  ptr += "<input type=\"radio\" id=\"blue\" name=\"color\" value=\"Blue\" /><br>";
  ptr += "<label for=\"green\">Green</label>";
  ptr += "<input type=\"radio\" id=\"green\" name=\"color\" value=\"Green\" /><br>";
  ptr += "<label for=\"yellow\">Yellow</label>";
  ptr += "<input type=\"radio\" id=\"yellow\" name=\"color\" value=\"Yellow\" /><br>";
  ptr += "<input type=\"submit\" value=\"Post to Kintone!\">";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

String PostResult(uint8_t result) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Kintone and IOT!</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP8266 Web Server</h1>\n";
  ptr += "<p>Post page</p>";
  if (result) {
    ptr += "<p>Success!</p>";
  } else {
    ptr += "<p>Error Posting</p>";
  }
  ptr += "<a href='/'> Go Back </a>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}