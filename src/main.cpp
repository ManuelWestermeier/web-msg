#include <WiFi.h>

#include "./secrets.h"

#include "./server/index.hpp"

WebMsgServer app(80);

void setup()
{
  Serial.begin(115200);

  // connect to wifi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");

  app.init(GITHUB_TOKEN);
}

void loop()
{
}
