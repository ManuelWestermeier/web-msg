import crypto from "crypto";

export function generateKey() {
  return crypto.randomBytes(32).toString("hex"); // 256-bit
}

export function hash256(input) {
  return crypto.createHash("sha256").update(input).digest("hex");
}
