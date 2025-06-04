import net from "net";
import http from "http";
import { performance } from "perf_hooks";

const mode = process.argv[2] || "tcp"; // 'http' or 'tcp'
const targetHost = "192.168.178.133";
const targetPort = 80;
const maxClients = 4;
const timeoutMs = 0;

let successCount = 0;
let failCount = 0;
let totalResponseTime = 0;

const testTcpConnection = (i) => {
  return new Promise((resolve) => {
    const start = performance.now();
    const client = new net.Socket();

    let finished = false;
    client.setTimeout(timeoutMs);

    client.connect(targetPort, targetHost, () => {
      const duration = performance.now() - start;
      successCount++;
      totalResponseTime += duration;
      finished = true;
      client.destroy();
      resolve();
    });

    client.on("error", () => {
      if (!finished) {
        failCount++;
        finished = true;
        resolve();
      }
    });

    client.on("timeout", () => {
      if (!finished) {
        failCount++;
        finished = true;
        client.destroy();
        resolve();
      }
    });
  });
};

const testHttpConnection = (i) => {
  return new Promise((resolve) => {
    const start = performance.now();
    const req = http.get(
      { hostname: targetHost, port: targetPort, path: "/", timeout: timeoutMs },
      (res) => {
        res.on("data", () => {});
        res.on("end", () => {
          const duration = performance.now() - start;
          successCount++;
          totalResponseTime += duration;
          resolve();
        });
      }
    );

    req.on("error", () => {
      failCount++;
      resolve();
    });

    req.on("timeout", () => {
      failCount++;
      req.destroy();
      resolve();
    });
  });
};

(async () => {
  console.log(
    `Running ${mode.toUpperCase()} benchmark on ${targetHost}:${targetPort}`
  );

  const testFn = mode === "http" ? testHttpConnection : testTcpConnection;
  const clients = Array.from({ length: maxClients }, (_, i) => testFn(i));
  await Promise.all(clients);

  const avgTime =
    successCount > 0 ? (totalResponseTime / successCount).toFixed(2) : "N/A";

  console.log(`\n--- Benchmark Result ---`);
  console.log(`Total Attempts:   ${maxClients}`);
  console.log(`Success:          ${successCount}`);
  console.log(`Failed:           ${failCount}`);
  console.log(`Avg Response Time: ${avgTime} ms`);
})();
