#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <sys/stat.h>

#include "utils.h"

int HOST_NUMBER = std::getenv("HOST_NUMBER") ? atoi(std::getenv("HOST_NUMBER")) : 0;
int NEIGHBOUR_HOST_NUMBER = std::getenv("NEIGHBOUR_HOST") ? atoi(std::getenv("NEIGHBOUR_HOST")) : 0;
int PORT_BASE = 55990;
int PORT = PORT_BASE + HOST_NUMBER;
int CLIENT_PORT_BASE = 56990;
int CLIENT_PORT = CLIENT_PORT_BASE + HOST_NUMBER;

constexpr int BUFFER_SIZE = 4096;

void sendDataToServer(uint32_t port, std::string src_filename, std::string dst_filename) {

    int sock = 0;
    int opt = 1;
    struct sockaddr_in serv_addr, client_addr;

    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    std::memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(CLIENT_PORT);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(sock);
        return;
    }

    if (bind(sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Client bind failed");
        close(sock);
        return;
    }

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Client connection Failed");
        close(sock);
        return;
    }

    std::ifstream input(src_filename);
    while (!input.eof()) {

        std::vector<int> chunk;
        int num;
        while (chunk.size() < BUFFER_SIZE / 4 - 2 && input >> num) {
            chunk.push_back(num);
        }

        if (!chunk.empty()) {
            char s[BUFFER_SIZE] = {0};
            s[0] = 'y'; 
            int32_t dataSize = static_cast<int32_t>(chunk.size() * sizeof(int32_t));
            std::memcpy(&s[4], &dataSize, sizeof(dataSize));
            std::memcpy(&s[8], chunk.data(), chunk.size() * sizeof(int32_t));
            send(sock, s, BUFFER_SIZE, 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    dst_filename = "x" + dst_filename;

    send(sock, dst_filename.c_str(), dst_filename.length(), 0);
    close(sock);
    
}

inline bool file_exists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

std::string get_file_prefix(uint32_t host_number, std::string path) {
    std::string prefix = "../data/local" + std::to_string(host_number);
    return prefix + path;
}

void clientThread() {
    std::string file_prefix = get_file_prefix(HOST_NUMBER, "/file");
    std::string file_to_merge_prefix = get_file_prefix(HOST_NUMBER, "/file_to_merge");
    std::string file_merged_prefix = get_file_prefix(HOST_NUMBER, "/merged_file");
    std::string my_filename = file_prefix + std::to_string(HOST_NUMBER);
    externalMergeSort(my_filename);
    std::string file_prefix_neighbour = get_file_prefix(NEIGHBOUR_HOST_NUMBER, "/file_to_merge");
    sendDataToServer(PORT_BASE + NEIGHBOUR_HOST_NUMBER, my_filename, file_prefix_neighbour); 
}

int main(int argc, char* argv[]) {

    if (argc >= 3) {
        HOST_NUMBER = std::atoi(argv[1]);
        NEIGHBOUR_HOST_NUMBER = std::atoi(argv[2]);

        PORT_BASE = 55990;
        PORT = PORT_BASE + HOST_NUMBER;
        CLIENT_PORT_BASE = 56990;
        CLIENT_PORT = CLIENT_PORT_BASE + HOST_NUMBER;

    }

    std::thread client(clientThread);

    client.join();

    return 0;
}
