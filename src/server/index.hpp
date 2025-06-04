#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "./init.hpp"

struct WebMsgServer
{
    GitHubClient gh;
    AsyncWebServer server;
    int serverPort;

    WebMsgServer() : WebMsgServer(80) {}
    WebMsgServer(int port) : server(port), serverPort(port) {}

    void init(const String &GITHUB_TOKEN = "")
    {
        gh = initGH(GITHUB_TOKEN); // KEIN `*` mehr – sicheres Copy/Move

        // WiFi muss bereits verbunden sein!
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(200, "text/plain", "Hello from ESP32 HTTP Server!"); });

        server.begin();
        Serial.println("http://" + WiFi.localIP().toString() + ":" + String(serverPort));
    }
};
