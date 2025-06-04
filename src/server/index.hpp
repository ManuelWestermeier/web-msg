#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "./init.hpp"

#include "../get-public-ip.hpp"

struct WebMsgServer
{
    GitHubClient gh;
    AsyncWebServer server;
    int serverPort;

    WebMsgServer() : WebMsgServer(80) {}
    WebMsgServer(int port) : server(port), serverPort(port) {}

    void init(const String &GITHUB_TOKEN = "")
    {
        Serial.println("http://" + WiFi.localIP().toString() + (serverPort == 80 ? "" : +":" + String(serverPort)));
        gh = initGH(GITHUB_TOKEN);

        server.on("/send", HTTP_GET, [](AsyncWebServerRequest *request) { //
            request->send(200, "text/plain", getPublicIP());
        });

        server.begin();
    }
};
