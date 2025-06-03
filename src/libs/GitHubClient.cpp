#include "GitHubClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/base64.h> // for mbedtls_base64_encode / decode

// Host for all GitHub API calls
const char *GitHubClient::GITHUB_HOST = "api.github.com";

GitHubClient::GitHubClient()
    : _token(""),
      _username(""),
      _repo("")
{
}

bool GitHubClient::begin(const String &token)
{
    _token = token;

    // 1) GET /user → verify token and fetch username
    int code;
    String body;
    if (!_request("GET", "/user", "", code, body))
    {
        return false;
    }
    if (code != 200)
    {
        // invalid token or network error
        return false;
    }

    // Parse JSON: { "login": "username", … }
    StaticJsonDocument<512> doc;
    auto err = deserializeJson(doc, body);
    if (err)
    {
        return false;
    }
    _username = doc["login"].as<String>();
    return (_username.length() > 0);
}

String GitHubClient::getUsername() const
{
    return _username;
}

bool GitHubClient::createRepoIfNotExists(const String &repoName, bool publicFlag)
{
    _repo = repoName;

    // 1) Check GET /repos/:owner/:repo
    int code;
    String body;
    String urlCheck = "/repos/" + _username + "/" + _repo;
    if (_request("GET", urlCheck, "", code, body) && code == 200)
    {
        // Repo already exists
        return true;
    }

    // 2) Create it via POST /user/repos
    StaticJsonDocument<512> doc;
    doc["name"] = _repo;
    doc["description"] = "Repository created by ESP32";
    doc["private"] = (publicFlag ? false : true);
    String payload;
    serializeJson(doc, payload);

    if (!_request("POST", "/user/repos", payload, code, body))
    {
        return false;
    }
    return (code == 201);
}

std::vector<GH_Entry> GitHubClient::readDir(const String &path)
{
    std::vector<GH_Entry> result;
    // Normalize path: remove leading slash (GitHub API expects no leading slash)
    String p = path;
    if (p.startsWith("/"))
        p = p.substring(1);

    String url = "/repos/" + _username + "/" + _repo + "/contents";
    if (p.length())
        url += "/" + p;

    int code;
    String body;
    if (!_request("GET", url, "", code, body))
    {
        return result;
    }
    if (code != 200)
    {
        return result;
    }

    // Parse JSON array of entries
    DynamicJsonDocument doc(2048);
    auto err = deserializeJson(doc, body);
    if (err || !doc.is<JsonArray>())
    {
        return result;
    }

    for (JsonObject entry : doc.as<JsonArray>())
    {
        GH_Entry e;
        e.name = entry["name"].as<String>();
        e.path = entry["path"].as<String>();
        e.sha = entry["sha"].as<String>();
        e.type = entry["type"].as<String>(); // "file" or "dir"
        result.push_back(e);
    }
    return result;
}

bool GitHubClient::createDir(const String &path)
{
    // To create a "directory" /foo/bar, write a ".gitkeep" inside it
    String p = path;
    if (p.endsWith("/"))
        p = p.substring(0, p.length() - 1);
    if (p.startsWith("/"))
        p = p.substring(1);

    String placeholderPath = "/" + p + "/.gitkeep";
    return writeFile(placeholderPath, String());
}

bool GitHubClient::writeFile(const String &path, const String &content)
{
    // 1) Check if file exists → fetch SHA
    String sha;
    bool exists = _getSha(path, sha);

    // 2) Build JSON payload
    StaticJsonDocument<2048> doc;
    String cleanPath = path;
    if (cleanPath.startsWith("/"))
        cleanPath = cleanPath.substring(1);

    doc["message"] = exists ? ("Update " + cleanPath) : ("Create " + cleanPath);

    // Base64‐encode the content
    String b64 = _base64Encode(content);
    doc["content"] = b64;

    if (exists)
    {
        doc["sha"] = sha;
    }

    String payload;
    serializeJson(doc, payload);

    // 3) PUT /repos/:owner/:repo/contents/:path
    String url = "/repos/" + _username + "/" + _repo + "/contents/" + _urlEncode(cleanPath);
    int code;
    String body;
    if (!_request("PUT", url, payload, code, body))
    {
        return false;
    }
    return (code == 201 || code == 200);
}

String GitHubClient::readFile(const String &path)
{
    String cleanPath = path;
    if (cleanPath.startsWith("/"))
        cleanPath = cleanPath.substring(1);
    String url = "/repos/" + _username + "/" + _repo + "/contents/" + _urlEncode(cleanPath);

    int code;
    String body;
    if (!_request("GET", url, "", code, body) || code != 200)
    {
        return String();
    }

    // Parse JSON: { "content": "BASE64ENCODED...", "encoding": "base64", ... }
    StaticJsonDocument<2048> doc;
    auto err = deserializeJson(doc, body);
    if (err)
    {
        return String();
    }
    String encoded = doc["content"].as<String>();

    // Remove any newline characters
    encoded.replace("\n", "");

    // Base64‐decode via mbedTLS
    return _base64Decode(encoded);
}

bool GitHubClient::deleteFile(const String &path)
{
    // Need to fetch the SHA first
    String sha;
    if (!_getSha(path, sha))
    {
        return false; // file doesn’t exist
    }

    // Build JSON: { "message": "Delete <path>", "sha": "<sha>" }
    StaticJsonDocument<512> doc;
    String cleanPath = path;
    if (cleanPath.startsWith("/"))
        cleanPath = cleanPath.substring(1);
    doc["message"] = "Delete " + cleanPath;
    doc["sha"] = sha;
    String payload;
    serializeJson(doc, payload);

    String url = "/repos/" + _username + "/" + _repo + "/contents/" + _urlEncode(cleanPath);
    int code;
    String body;
    if (!_request("DELETE", url, payload, code, body))
    {
        return false;
    }
    return (code == 200);
}

// ————— PRIVATE HELPERS ——————————————————————————

bool GitHubClient::_request(const String &method,
                            const String &url,
                            const String &requestBody,
                            int &responseCode,
                            String &responseBody)
{
    WiFiClientSecure client;
    // In production, load the root CA here. For simplicity, we skip ownership of CA validation:
    client.setCACert(nullptr);

    HTTPClient https;
    String fullURL = String("https://") + GITHUB_HOST + url;
    https.begin(client, fullURL);
    https.addHeader("User-Agent", "ESP32-GitHubClient");
    https.addHeader("Authorization", "token " + _token);
    https.addHeader("Accept", "application/vnd.github.v3+json");

    if (method == "GET")
    {
        responseCode = https.GET();
    }
    else if (method == "POST")
    {
        https.addHeader("Content-Type", "application/json");
        responseCode = https.POST(requestBody);
    }
    else if (method == "PUT")
    {
        https.addHeader("Content-Type", "application/json");
        responseCode = https.PUT(requestBody);
    }
    else if (method == "DELETE")
    {
        https.addHeader("Content-Type", "application/json");
        responseCode = https.sendRequest("DELETE", requestBody);
    }
    else
    {
        https.end();
        return false;
    }

    if (responseCode > 0)
    {
        responseBody = https.getString();
    }
    else
    {
        responseBody = String();
    }
    https.end();
    return true;
}

String GitHubClient::_urlEncode(const String &str)
{
    String encoded;
    encoded.reserve(str.length() * 3); // worst‐case each char → %XX
    for (size_t i = 0; i < str.length(); i++)
    {
        char c = str[i];
        if (('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~')
        {
            encoded += c;
        }
        else
        {
            char buf[4];
            sprintf(buf, "%%%02X", (uint8_t)c);
            encoded += buf;
        }
    }
    return encoded;
}

bool GitHubClient::_getSha(const String &path, String &shaOut)
{
    String cleanPath = path;
    if (cleanPath.startsWith("/"))
        cleanPath = cleanPath.substring(1);
    String url = "/repos/" + _username + "/" + _repo + "/contents/" + _urlEncode(cleanPath);

    int code;
    String body;
    if (!_request("GET", url, "", code, body) || code != 200)
    {
        return false;
    }

    StaticJsonDocument<512> doc;
    auto err = deserializeJson(doc, body);
    if (err)
        return false;

    shaOut = doc["sha"].as<String>();
    return (shaOut.length() > 0);
}

String GitHubClient::_base64Encode(const String &plain)
{
    size_t ilen = plain.length();
    // Calculate required output buffer size: 4 * ((ilen+2)/3)
    size_t buf_len = 4 * ((ilen + 2) / 3) + 1;
    unsigned char *outBuf = (unsigned char *)malloc(buf_len);
    if (!outBuf)
    {
        return String();
    }

    size_t olen = 0;
    int ret = mbedtls_base64_encode(
        outBuf,
        buf_len,
        &olen,
        (const unsigned char *)plain.c_str(),
        ilen);
    if (ret != 0)
    {
        free(outBuf);
        return String();
    }
    outBuf[olen] = '\0';
    String encoded = String((char *)outBuf);
    free(outBuf);
    return encoded;
}

String GitHubClient::_base64Decode(const String &b64)
{
    size_t ilen = b64.length();
    // Rough output buffer size: (ilen * 3) / 4
    size_t buf_len = (ilen * 3) / 4 + 1;
    unsigned char *outBuf = (unsigned char *)malloc(buf_len);
    if (!outBuf)
    {
        return String();
    }

    size_t olen = 0;
    int ret = mbedtls_base64_decode(
        outBuf,
        buf_len,
        &olen,
        (const unsigned char *)b64.c_str(),
        ilen);
    if (ret != 0)
    {
        free(outBuf);
        return String();
    }
    outBuf[olen] = '\0';
    String decoded = String((char *)outBuf);
    free(outBuf);
    return decoded;
}
