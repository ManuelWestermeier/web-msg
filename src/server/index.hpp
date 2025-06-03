#pragma once

#include "./init.hpp"

struct Server
{
    GitHubClient gh;

    void init(const String &GITHUB_TOKEN = "", int port = 0)
    {
        gh = initGH(GITHUB_TOKEN);
    }
};