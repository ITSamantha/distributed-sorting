#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <sstream>
#include <chrono>
#include <cstring>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

constexpr int BUFFER_SIZE = 4096;
int PORT_BASE = 55990;

void merge(int a, int b) {

    std::cout << "Merging " << a << " and " << b << std::endl; 
    int sock = 0;
    int opt = 1;
    struct sockaddr_in serv_addr, client_addr;

    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_BASE + a);

    std::memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(56990 + 100);

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
        perror("Manager connection Failed");
        close(sock);
        return;
    }

    char s[BUFFER_SIZE] = {0};
    s[0] = 'z';
    s[1] = a;
    s[2] = b; 

    send(sock, s, BUFFER_SIZE, 0);
    close(sock);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

}

void merge(int a, int b, int c, int d) {
    merge(a, c);
    merge(b, d);
    merge(b, c);
}

void merge(int a, int b, int c, int d, int e, int f, int g, int h) {
    merge(a, b, e, f);
    merge(c, d, g, h);
    merge(c, d, e, f);
}


int main() {

    merge(1, 2);
    merge(3, 4);
    merge(5, 6);
    merge(7, 8);
    
    merge(1, 2, 3, 4);
    merge(5, 6, 7, 8);

    merge(1, 2, 3, 4, 5, 6, 7, 8);

    return 0;
}
