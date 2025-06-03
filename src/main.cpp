#include <WiFi.h>
#include "./libs/GitHubClient.h"

#include "./secrets.h"

GitHubClient gh;

void setup()
{
  Serial.begin(115200);
  delay(500);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");

  if (!gh.begin(GITHUB_TOKEN))
  {
    Serial.println("❌ GitHub token invalid or network error.");
    while (true)
    {
      delay(1000);
    }
  }
  Serial.printf("✅ Logged in as: %s\n", gh.getUsername().c_str());

  // Ensure “web-msg-data” repo exists (public)
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

  // Create a folder “myfolder”
  if (gh.createDir("/myfolder"))
  {
    Serial.println("✅ Created directory /myfolder");
  }

  // Write a file “/myfolder/hello.txt”
  if (gh.writeFile("/myfolder/hello.txt", "Hello, ESP32 + GitHub!"))
  {
    Serial.println("✅ Wrote /myfolder/hello.txt");
  }

  // List entries in “/myfolder”
  auto entries = gh.readDir("/myfolder");
  for (auto &e : entries)
  {
    Serial.printf(" • %s [%s]\n", e.name.c_str(), e.type.c_str());
  }

  // Read the file
  String txt = gh.readFile("/myfolder/hello.txt");
  Serial.println("📄 File contents: " + txt);

  // Delete the file
  if (gh.deleteFile("/myfolder/hello.txt"))
  {
    Serial.println("✅ Deleted /myfolder/hello.txt");
  }
}

void loop()
{
  // Nothing to do
}
