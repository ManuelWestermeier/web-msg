import fetch from "node-fetch";
import fs from "fs";
import { hash256 } from "./crypto.js";

const GITHUB_TOKEN =
  process.env.GH_TOKEN ||
  fs.readFileSync("./gh.token").toString("utf-8") ||
  "set your token"; // <--- Sicher speichern, z. B. via ENV

const USER = "ManuelWestermeier";
const REPO = "web-msg-public-data";
const BRANCH = "main";
const TOKEN = GITHUB_TOKEN;

function buildHeaders() {
  return {
    Authorization: `Bearer ${TOKEN}`,
    "User-Agent": "github-api-client",
    Accept: "application/vnd.github+json",
  };
}

export async function fetchPublicRecord(domain) {
  const filePath = `domains/${hash256(domain)}.dat`;
  const url = `https://raw.githubusercontent.com/${USER}/${REPO}/${BRANCH}/${filePath}`;
  const res = await fetch(url);
  if (!res.ok) return null;
  const [ip, publicKey, secretKey] = (await res.text()).trim().split("\n");
  return { ip, publicKey, secretKey };
}

export async function uploadPublicRecord(domain, ip, publicKey, secretKey) {
  const filePath = `domains/${hash256(domain)}.dat`;
  const content = Buffer.from(`${ip}\n${publicKey}\n${secretKey}`).toString(
    "base64"
  );

  const url = `https://api.github.com/repos/${USER}/${REPO}/contents/${filePath}`;
  const res = await fetch(url, {
    method: "PUT",
    headers: buildHeaders(),
    body: JSON.stringify({
      message: `Initial upload for ${domain}`,
      content,
      branch: BRANCH,
    }),
  });

  if (!res.ok) {
    const err = await res.text();
    throw new Error(`Upload failed: ${err}`);
  }

  return await res.json();
}

export async function updateIP(domain, newIP, providedKey) {
  const filePath = `domains/${hash256(domain)}.dat`;
  const record = await fetchPublicRecord(domain);
  if (!record) throw new Error("Record not found.");

  if (providedKey !== record.secretKey) throw new Error("Invalid key");

  const newContent = Buffer.from(
    `${newIP}\n${record.publicKey}\n${record.secretKey}`
  ).toString("base64");

  const metaUrl = `https://api.github.com/repos/${USER}/${REPO}/contents/${filePath}`;
  const metaRes = await fetch(metaUrl, { headers: buildHeaders() });
  const meta = await metaRes.json();

  const updateRes = await fetch(metaUrl, {
    method: "PUT",
    headers: buildHeaders(),
    body: JSON.stringify({
      message: `Update IP for ${domain}`,
      content: newContent,
      sha: meta.sha,
      branch: BRANCH,
    }),
  });

  if (!updateRes.ok) {
    const err = await updateRes.text();
    throw new Error(`Update failed: ${err}`);
  }

  return await updateRes.json();
}
