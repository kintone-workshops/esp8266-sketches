#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Your WIFI information. You must be connected to the same WIFI network to connect via localhost.
const char* ssid = "MY_WIFI_SSID";
const char* password = "MY_WIFI_PASSWORD";
// A JSON Class to handle JSON data.
DynamicJsonDocument doc(1024);
// The ESP8266 server class.
ESP8266WebServer server(80);
// A variable for tracking a valid HTTP response or not.
bool result = false;

// The setup function is special, and runs only once.
void setup() {
  // The BAUD rate for the serial monitor.
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

  // The HTTP server class let's developers create routes, that run specific functions on access.
  // Here, we designate the root route, the /post route, and a 404 route.
  server.on("/", handle_OnConnect);
  server.on("/post", handlePost);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// The loop function. This will run continuously, and let our server class handle incoming requests.
void loop() {
  server.handleClient();
}

// This function runs when users access the root route.
void handle_OnConnect() {
  Serial.println("Welcome. Enter your App ID, API Key, and Subdomain to post some data.");
  // This function sends a 200 on successful connection, and then runs the SendHTML() function.
  server.send(200, "text/html", SendHTML());
}

// This function runs after clicking the Post to Kintone button, and the users accesses the /post route.
void handlePost() {
  // A new HTTP Client.
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  Serial.println("Attempting to Save to Kintone...");
  // The variables passed to this route via Form Action. The user's subdomain, appID, API Token, and favorite color.
  String subdomain = server.arg("subdomain");
  String appID = server.arg("appID");
  String token = server.arg("API");
  String color = server.arg("color");
  // The URL for our HTTP Request.
  String URL = "https://" + subdomain + ".kintone.com/k/v1/record.json?app=" + appID;
  Serial.print("info:");
  Serial.println(URL);
  http.begin(client, URL);
  // Formatting the POST Request JSON Object.
  doc["app"] = appID;
  JsonObject recordObject = doc.createNestedObject("record");
  recordObject["color"]["value"] = color;
  String json;
  serializeJson(doc, json);
  // End of JSON formatting. The JSON string is saved to the json variable.

  // Response Code variable for our HTTP request.
  int responseCode = 0;
  // Adding request headers via the http class.
  http.addHeader("X-Cybozu-API-Token", token);
  http.addHeader("Content-type", "application/json");
  // Making the POST request, and saving the result to responseCode
  responseCode = http.POST(json);
  Serial.printf("http Response Code = %d \n", responseCode);
  String payload = http.getString();
  Serial.println(payload);

  // Using an IF statement we can choose which HTML to send for each response code. Details are logged to the serial monitor.
  if (responseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(responseCode);
    String payload = http.getString();
    Serial.println(payload);
    result = true;
  } else if (responseCode == 400) {
    Serial.print("Error code: ");
    Serial.println(responseCode);
    result = false;
  }
  else {
    Serial.print("Error code: ");
    Serial.println(responseCode);
    result = false;
  }
  // Ending the HTTP request, and passing the result variable to the PostResult function.
  http.end();
  server.send(200, "text/html", PostResult(result));
}

// A function to handle all 404 routes.
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

// A Function which returns an html string. This is the HTML which gets displayed on each route. SendHTML for the root route.
// Note that quotation marks (" ") within HTML must be escaped with a forward slash ( \ ).
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

// This function displays the HTML for the /post route. It takes a boolean variable, which can be used to alter the HTML displayed.
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
    ptr += "<p>Error Posting Check your JSON, API Token, and field codes!</p>";
  }
  ptr += "<a href='/'> Go Back </a>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}