#include <WiFi.h>
#include <HTTPClient.h>

String getPublicIP()
{
    HTTPClient http;
    http.begin("http://api.ipify.org");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String ip = http.getString();
        http.end();
        return ip;
    }
    else
    {
        http.end();
        return "Error: " + String(httpCode);
    }
}
