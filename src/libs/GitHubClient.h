#ifndef GITHUB_CLIENT_H
#define GITHUB_CLIENT_H

#include <Arduino.h>
#include <vector>

struct GH_Entry {
  String name;   // file or directory name
  String path;   // full path in repo
  String sha;    // blob/tree SHA
  String type;   // "file" or "dir"
};

class GitHubClient {
public:
  GitHubClient();

  /**
   * @brief Initialize GitHubClient with a personal access token (PAT).
   *        Verifies the token by fetching the authenticated username.
   *
   * @param token GitHub PAT with at least “repo” scope.
   * @return true if token is valid (username fetched), false otherwise.
   */
  bool begin(const String &token);

  /**
   * @brief Returns the GitHub username extracted at begin().
   */
  String getUsername() const;

  /**
   * @brief Checks if the repository exists. If not, attempts to create it.
   *        Repo is created as public if publicFlag=true.
   *
   * @param repoName Name of the repository to ensure.
   * @param publicFlag true → public; false → private.
   * @return true if repo already existed or was created successfully.
   */
  bool createRepoIfNotExists(const String &repoName, bool publicFlag = true);

  /**
   * @brief List contents of a “directory” in the repo. If path is “/” or empty,
   *        lists top-level.
   *
   * @param path Leading slash is optional. e.g. "/folder", "folder/sub"
   * @return Vector of GH_Entry, each with name/type/sha.
   */
  std::vector<GH_Entry> readDir(const String &path);

  /**
   * @brief Creates a “directory” by committing a “.gitkeep” placeholder in it.
   *
   * @param path Path to directory (e.g. "/logs/subdir").
   * @return true if success, false otherwise.
   */
  bool createDir(const String &path);

  /**
   * @brief Write (create or update) a file at given path with content.
   *
   * If the file already exists, this fetches its current SHA and updates it.
   * Otherwise, it creates it afresh.
   *
   * @param path Full path including filename. e.g. "/logs/hello.txt"
   * @param content Plain‐text content to store (will be base64‐encoded internally).
   * @return true if write was successful.
   */
  bool writeFile(const String &path, const String &content);

  /**
   * @brief Read a file’s contents as a String.
   *
   * @param path Full path including filename. e.g. "/logs/hello.txt"
   * @return Plain‐text contents if success; empty String on failure.
   */
  String readFile(const String &path);

  /**
   * @brief Delete a file at given path.
   *
   * @param path Full path including filename. e.g. "/logs/hello.txt"
   * @return true if delete was successful.
   */
  bool deleteFile(const String &path);

private:
  String _token;
  String _username;
  String _repo;      // set once createRepoIfNotExists() succeeds

  static const char *GITHUB_HOST;  // "api.github.com"

  /**
   * @brief Make an HTTPS request to GitHub.
   *
   * @param method "GET", "POST", "PUT", "DELETE"
   * @param url API URL path (e.g. "/user/repos")
   * @param requestBody JSON‐encoded body (empty if none)
   * @param responseCode (output) HTTP status code
   * @param responseBody (output) response body as string
   * @return true if an HTTPS connection was made and a response received.
   */
  bool _request(const String &method,
                const String &url,
                const String &requestBody,
                int &responseCode,
                String &responseBody);

  /**
   * @brief URL‐encode a path segment (percent‐encode).
   */
  String _urlEncode(const String &str);

  /**
   * @brief Helper: fetch SHA of existing file or directory, or return false if not found.
   *
   * @param path e.g. "/logs/hello.txt"
   * @param shaOut (output) the blob/tree SHA if found.
   * @return true if found (shaOut set), false otherwise.
   */
  bool _getSha(const String &path, String &shaOut);

  /**
   * @brief Base64 encode a UTF‐8 string using mbedTLS. GitHub expects content in base64 for file uploads.
   */
  String _base64Encode(const String &plain);

  /**
   * @brief Base64 decode a string (mbedTLS) → returns decoded UTF‐8.
   */
  String _base64Decode(const String &b64);
};

#endif // GITHUB_CLIENT_H
