#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

struct Message {
    char sender[20];
    char content[256];
};

void ReceiveMessages(SOCKET server_socket) {
    Message msg;
    while (true) {
        int bytes_received = recv(server_socket, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
        if (bytes_received > 0) {
            std::cout << "\n[" << msg.sender << "]: " << msg.content << std::endl << "Enter message: ";
        }
        else {
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    wchar_t wide_ip[16];
    MultiByteToWideChar(CP_UTF8, 0, SERVER_IP, -1, wide_ip, 16);
    InetPton(AF_INET, wide_ip, &server_addr.sin_addr);

    connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr));

    Message msg;
    std::cout << "Enter your name: ";
    std::cin.getline(msg.sender, 20);

    send(client_socket, reinterpret_cast<char*>(&msg), sizeof(msg), 0);

    std::thread receive_thread(ReceiveMessages, client_socket);
    receive_thread.detach();

    while (true)
    {
        std::cout << "Enter message: ";
        std::cin.getline(msg.content, 256);

        send(client_socket, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
