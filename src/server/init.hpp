#pragma once

#include <Arduino.h>
#include "../libs/GitHubClient.h"

GitHubClient initGH(const String &GITHUB_TOKEN = "")
{
    GitHubClient gh;

    if (!gh.begin(GITHUB_TOKEN))
    {
        Serial.println("❌ GitHub token invalid or network error.");
        while (true)
        {
            delay(1000);
        }
    }

    Serial.printf("✅ Logged in als: %s\n", gh.getUsername().c_str());

    if (gh.createRepoIfNotExists("web-msg-data", true))
    {
        Serial.println("✅ Repo web-msg-data ready.");
    }
    else
    {
        Serial.println("❌ Failed to create/access web-msg-data.");
        while (true)
        {
            delay(1000);
        }
    }

    return gh; // Rückgabe per Wert (copy oder move)
}
