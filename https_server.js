const https = require('https');
const http = require('http');
const fs = require('fs');
const path = require('path');

const certPath = path.join(__dirname, 'cert.pem');
const keyPath = path.join(__dirname, 'key.pem');

if (!fs.existsSync(certPath) || !fs.existsSync(keyPath)) {
  console.error('[ERROR] cert.pem or key.pem not found!');
  process.exit(1);
}

const options = {
  key: fs.readFileSync(keyPath),
  cert: fs.readFileSync(certPath)
};

function handleProxy(req, res) {
  const proxy = http.request({
    hostname: '127.0.0.1',
    port: 8080,
    path: req.url,
    method: req.method,
    headers: req.headers
  }, (targetRes) => {
    res.writeHead(targetRes.statusCode, targetRes.headers);
    targetRes.pipe(res, { end: true });
  });

  proxy.on('error', (err) => {
    res.writeHead(502, { 'Content-Type': 'text/plain' });
    res.end('Barcode server not running on port 8080.');
  });

  req.pipe(proxy, { end: true });
}

// 1. Primary HTTPS proxy on 8443
const httpsServer8443 = https.createServer(options, handleProxy);
httpsServer8443.listen(8443, '0.0.0.0', () => {
  console.log('[HTTPS READY] Secure Live Stream active on https://0.0.0.0:8443');
});

// 2. Default HTTPS on 443 (so typing https://<ip> without port works)
try {
  const httpsServer443 = https.createServer(options, handleProxy);
  httpsServer443.on('error', (err) => {
    console.log('[INFO] Port 443 unavailable, standard 8443 active.');
  });
  httpsServer443.listen(443, '0.0.0.0', () => {
    console.log('[HTTPS READY] Default HTTPS active on https://0.0.0.0:443');
  });
} catch (e) {}

// 3. Port 80 HTTP auto-upgrade redirect
// When a user just types "192.168.10.244" in Chrome or Safari, it connects to port 80 first.
// This automatically redirects the mobile browser to https://192.168.10.244:8443/ so live camera works!
try {
  const httpServer80 = http.createServer((req, res) => {
    const host = (req.headers.host || '').split(':')[0];
    const targetUrl = `https://${host}:8443${req.url}`;
    res.writeHead(302, { 'Location': targetUrl });
    res.end();
  });
  httpServer80.on('error', (err) => {
    console.log('[INFO] Port 80 unavailable, use port 8443 or scan QR code.');
  });
  httpServer80.listen(80, '0.0.0.0', () => {
    console.log('[HTTP READY] Port 80 redirect active (typing plain IP will auto-upgrade to HTTPS)');
  });
} catch (e) {}
