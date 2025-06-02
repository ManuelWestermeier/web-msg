import http from "http";
import fs from "fs";
import crypto from "crypto";
import { Buffer } from "buffer";

// === Konfiguration ===
const GITHUB_USERNAME = "ManuelWestermeier";
const GITHUB_REPO = "web-msg-public-data";
const BRANCH = "main";
const GITHUB_TOKEN =
  process.env.GH_TOKEN ||
  fs.readFileSync("./gh.token").toString("utf-8") ||
  "set your token"; // <--- Sicher speichern, z. B. via ENV

// === Hilfsfunktionen ===
function hash256(input) {
  return crypto.createHash("sha256").update(input).digest("hex");
}

function isValidDomain(domain) {
  return typeof domain === "string" && /^[a-zA-Z0-9.-]{1,253}$/.test(domain);
}

function isValidIP(ip) {
  return typeof ip === "string" && /^(\d{1,3}\.){3}\d{1,3}$/.test(ip);
}

function isValidKey(key) {
  return typeof key === "string" && /^[a-zA-Z0-9_-]{16,128}$/.test(key);
}

// === GitHub Zugriff ===
async function fetchFile(domain) {
  const hashed = hash256(domain);
  const url = `https://raw.githubusercontent.com/${GITHUB_USERNAME}/${GITHUB_REPO}/${BRANCH}/domains/${hashed}.dat`;

  try {
    const res = await fetch(url);
    if (!res.ok) return null;
    const content = await res.text();
    return content;
  } catch {
    return null;
  }
}

async function getGitHubFileSha(filePath) {
  const apiUrl = `https://api.github.com/repos/${GITHUB_USERNAME}/${GITHUB_REPO}/contents/${filePath}`;
  const res = await fetch(apiUrl, {
    headers: {
      Authorization: `Bearer ${GITHUB_TOKEN}`,
      "User-Agent": "web-msg-server",
    },
  });
  if (!res.ok) return null;
  const json = await res.json();
  return json.sha;
}

async function writeFileToGitHub(domain, newIp, publicKey, secretKey) {
  const hashed = hash256(domain);
  const path = `domains/${hashed}.dat`;
  const fullContent = `${newIp}\n${publicKey}\n${secretKey}`;
  const encodedContent = Buffer.from(fullContent).toString("base64");

  const existingSha = await getGitHubFileSha(path);

  const res = await fetch(
    `https://api.github.com/repos/${GITHUB_USERNAME}/${GITHUB_REPO}/contents/${path}`,
    {
      method: "PUT",
      headers: {
        Authorization: `Bearer ${GITHUB_TOKEN}`,
        "User-Agent": "web-msg-server",
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        message: `Change IP of ${domain}`,
        content: encodedContent,
        sha: existingSha,
        branch: BRANCH,
        committer: {
          name: "web-msg-bot",
          email: "bot@example.com",
        },
      }),
    }
  );

  return await res.json();
}

// === Server Start ===
http
  .createServer(async (req, res) => {
    try {
      const url = new URL(req.url, `http://${req.headers.host}`);

      if (req.method !== "GET") {
        res.writeHead(405, { "Content-Type": "text/plain" });
        return res.end("Method Not Allowed");
      }

      if (url.pathname === "/change-ip") {
        const domain = url.searchParams.get("domain")?.trim();
        const ip = url.searchParams.get("ip")?.trim();
        const key = url.searchParams.get("key")?.trim();

        // Validierungschecks
        if (!domain || !ip || !key) {
          res.writeHead(400, { "Content-Type": "text/plain" });
          return res.end("Missing parameters.");
        }

        if (!isValidDomain(domain) || !isValidIP(ip) || !isValidKey(key)) {
          res.writeHead(400, { "Content-Type": "text/plain" });
          return res.end("Invalid parameter format.");
        }

        const fileContent = await fetchFile(domain);
        if (!fileContent) {
          res.writeHead(404, { "Content-Type": "text/plain" });
          return res.end("Domain not found.");
        }

        const [oldIp, publicKey, storedKey] = fileContent.trim().split("\n");

        if (!publicKey || !storedKey || key !== storedKey) {
          res.writeHead(403, { "Content-Type": "text/plain" });
          return res.end("Access denied.");
        }

        const result = await writeFileToGitHub(
          domain,
          ip,
          publicKey,
          storedKey
        );

        if (result?.content) {
          res.writeHead(200, { "Content-Type": "text/plain" });
          return res.end("IP updated successfully.");
        } else {
          res.writeHead(500, { "Content-Type": "text/plain" });
          return res.end("Failed to update file.");
        }
      }

      // 404 für andere Pfade
      res.writeHead(404, { "Content-Type": "text/plain" });
      res.end("Not Found");
    } catch (err) {
      // Unerwarteter Fehler
      console.error("Internal error:", err);
      res.writeHead(500, { "Content-Type": "text/plain" });
      res.end("Internal Server Error");
    }
  })
  .listen(process.env.PORT || 27954, () => {
    console.log("Server läuft auf Port 27954");
  });
