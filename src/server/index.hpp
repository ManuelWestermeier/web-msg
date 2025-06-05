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
    String ip = "";

    WebMsgServer() : WebMsgServer(80) {}
    WebMsgServer(int port) : server(port), serverPort(port) {}

    static void updateIpTask(void *param)
    {
        WebMsgServer *self = static_cast<WebMsgServer *>(param);
        if (!self)
            vTaskDelete(NULL); // safety

        while (1)
        {
            String newIp = getPublicIP();

            if (newIp.startsWith("Error:"))
                continue;

            if (newIp.length() > 0 && newIp != self->ip)
            {
                Serial.println("[updateIpTask] New IP: " + newIp);
                self->gh.writeFile("ip.txt", newIp);
                self->ip = newIp;
            }

            vTaskDelay(pdMS_TO_TICKS(60000)); // 60 Sekunden
        }
    }

    void init(const String &GITHUB_TOKEN = "")
    {
        Serial.println("http://" + WiFi.localIP().toString() + (serverPort == 80 ? "" : ":" + String(serverPort)));

        gh = initGH(GITHUB_TOKEN);

        server.on("/send", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { request->send(200, "text/plain", ip); });

        BaseType_t result = xTaskCreate(
            updateIpTask,
            "UpdateIPTask",
            8192,
            this,
            1,
            NULL);
        if (result != pdPASS)
        {
            Serial.println("❌ Failed to create updateIpTask");
        }

        server.begin();
    }
};
