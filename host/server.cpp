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
        perror("Server connection Failed");
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

bool file_exists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

std::string get_file_prefix(uint32_t host_number, std::string path) {
    std::string prefix = "../data/local" + std::to_string(host_number);
    return prefix + path;
}

void handleClient(int client_socket, int host) {
    
    if (host <= 8) {
        std::cout << "Got client from host #" << host << std::endl; 
    } else {
        std::cout << "Got client from management host #" << host << std::endl; 
    }
    
    char buffer[BUFFER_SIZE] = {0};

    std::string temp_file("file.tmp");
    std::ofstream out(temp_file);

    while (true) {
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            std::cerr << "[Server] Connection closed by client or read error.\n";
            break;
        }
        
        if (buffer[0] == 'y') { // continue sending data
            int32_t dataSize;
            std::memcpy(&dataSize, &buffer[4], sizeof(dataSize));
            size_t numIntegers = dataSize / sizeof(int32_t);
            int32_t received;
            for (size_t i = 0; i < numIntegers; i++) {
                std::memcpy(&received, &buffer[8 + i * sizeof(int32_t)], sizeof(received));
                out << received << " ";
            }
        } else if (buffer[0] == 'x') { // finished sending data
            std::string buffer_as_str(buffer);
            std::string filename = buffer_as_str.substr(1, BUFFER_SIZE);
            std::rename(temp_file.c_str(), filename.c_str());

            std::string file_prefix = get_file_prefix(HOST_NUMBER, "/file");
            std::string file_to_merge_prefix = get_file_prefix(HOST_NUMBER, "/file_to_merge");
            std::string file_merged_prefix = get_file_prefix(HOST_NUMBER, "/merged_file");
            std::string my_filename = file_prefix + std::to_string(HOST_NUMBER);

            externalMergeSort(my_filename);

            std::string merge_filename(file_to_merge_prefix);
            std::string merge_result_filename(file_merged_prefix);

            if (file_exists(merge_filename)) {

                mergeFiles(merge_filename, my_filename, merge_result_filename);
                std::remove(merge_filename.c_str());

                std::string greater_part = splitFile(merge_result_filename);
                sendDataToServer(PORT_BASE + host, greater_part, get_file_prefix(host, "/file" + std::to_string(host)));    
                std::rename(merge_result_filename.c_str(), my_filename.c_str());
                std::remove(greater_part.c_str());

                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            break;
        } else if (buffer[0] == 'z') {

            pid_t pid = fork();
            int a = buffer[1];
            int b = buffer[2];
            if (pid < 0) {
                std::cerr << "Fork failed" << std::endl;
            }
            else if (pid == 0) {
                const char* args[] = { "./client", std::to_string(b).c_str(), std::to_string(a).c_str(), nullptr };
                if (execvp(args[0], const_cast<char* const*>(args)) < 0) {
                    std::cerr << "Exec failed" << std::endl;
                    exit(1);
                }
            }
            
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    out.close();
    close(client_socket);

}

void serverThread() {

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        return;
    }

    std::cout << "Server started at :" << PORT << std::endl;
    while (1) {
        int addrlen = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        struct sockaddr_in addr;
        int host = -1;
        socklen_t addr_len = sizeof(addr);
        if (getpeername(new_socket, (struct sockaddr*)&addr, &addr_len) == -1) {
            perror("getsockname");
        } else {
            int local_port = ntohs(addr.sin_port);
            host = local_port - CLIENT_PORT_BASE;
        }

        handleClient(new_socket, host);

        close(new_socket);
    }

    close(server_fd);
}



int main(int argc, char* argv[]) {

    if (argc >= 2) {
        HOST_NUMBER = std::atoi(argv[1]);

        PORT_BASE = 55990;
        PORT = PORT_BASE + HOST_NUMBER;
        CLIENT_PORT_BASE = 56990;
        CLIENT_PORT = CLIENT_PORT_BASE + HOST_NUMBER;

    }

    std::thread server(serverThread);

    server.join();

    return 0;
}
