import http from "http";
import fetch from "node-fetch";

async function getPublicUserData(domain) {
  try {
    const encodeDomain = hash256(domain);
    const url = `https://raw.githubusercontent.com/ManuelWestermeier/web-msg-public-data/refs/heads/main/domains/${encodeDomain}.dat`;
    const res = await fetch(url);
    if (!res.ok) return new Error("not found");
    const [ip, publicKey] = (await res.text()).split("\n");
    return { ip, publicKey };
  } catch (error) {
    return new Error("fetch error");
  }
}

async function getPublicUserData(domain) {
  try {
    const encodeDomain = hash256(domain);
    const url = `https://raw.githubusercontent.com/ManuelWestermeier/web-msg-public-data/refs/heads/main/domains/${encodeDomain}.dat`;
    const res = await fetch(url);
    if (!res.ok) return new Error("not found");
    const [ip, publicKey] = (await res.text()).split("\n");
    return { ip, publicKey };
  } catch (error) {
    return new Error("fetch error");
  }
}

http
  .createServer(async (req, res) => {
    const url = new URL(`http:localhost${req.url}`);
    if (url.pathname == "/register") {
    } else if (url.pathname == "/change-ip") {
    } else if (url.pathname == "/delete") {
    }
  })
  .listen(process.env.PORT || 27954);
