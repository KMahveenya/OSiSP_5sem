#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 12345

struct Message {
    char sender[20];
    char content[256];
};

struct ClientInfo {
    SOCKET socket;
    std::string name;
};

std::vector<ClientInfo> clients;
std::mutex clients_mutex;

void BroadcastMessage(const Message& msg, const std::string& exclude_client) {
    std::lock_guard<std::mutex> guard(clients_mutex);
    for (const auto& client : clients) {
        if (client.name != exclude_client) {
            send(client.socket, reinterpret_cast<const char*>(&msg), sizeof(msg), 0);
        }
    }
}

void ClientHandler(SOCKET client_socket) {
    Message msg;
    int bytes_received = recv(client_socket, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return;
    }

    std::string client_name = msg.sender;

    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        clients.push_back({ client_socket, client_name });
        std::cout << client_name << " connected.\n";
    }

    while (true) {
        int bytes_received = recv(client_socket, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
        if (bytes_received <= 0) break;

        std::cout << "[" << msg.sender << "]: " << msg.content << std::endl;
        BroadcastMessage(msg, client_name);
    }

    closesocket(client_socket);

    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(), [&](const ClientInfo& c) {
            return c.socket == client_socket;
            }), clients.end());
        std::cout << client_name << " disconnected.\n";
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    std::cout << "Server started. Waiting for connections...\n";

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        std::thread(ClientHandler, client_socket).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
