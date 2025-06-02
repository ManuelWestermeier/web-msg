import { generateKey } from "./crypto.js";
import { fetchPublicRecord, uploadPublicRecord, updateIP } from "./github.js";

const domain = "testdomain123.example.com";
const ip = "123.45.67.89";
const publicKey = "dummy-public-key";
const secretKey = generateKey();

console.log("Generated key:", secretKey);

(async () => {
  try {
    console.log("📝 Upload...");
    await uploadPublicRecord(domain, ip, publicKey, secretKey);

    console.log("📥 Fetch...");
    const data = await fetchPublicRecord(domain);
    console.log(data);

    console.log("♻️ Update IP...");
    await updateIP(domain, "88.77.66.55", secretKey);

    console.log("✅ Done.");
  } catch (err) {
    console.error("❌ Error:", err.message);
  }
})();
