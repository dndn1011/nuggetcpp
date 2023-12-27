#include <iostream>
#include <stdexcept>
#include <system_error>
#include <format>
#include <functional>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <unistd.h>
#endif
#include "debug.h"
#include <assert.h>

class TCPListener {
public:
    TCPListener(u_short port) : port(port) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to initialize Winsock");
        }
#endif
    }

    std::function<void(const std::string& str)> receiver;

    void SetReceiver(const std::function<void(const std::string& str)> receiverIn) {
        receiver = receiverIn;
    }

    // Initialize and start listening for incoming connections
    void startListening() {
        createSocket();
        bindSocket();
        listenForConnections();
    }

    // Accept incoming connections and handle data
    void acceptConnections() {
        while (true) {
            SOCKET clientSocket = acceptConnection();
            if (clientSocket != INVALID_SOCKET) {
                handleClient(clientSocket);
            }
        }
    }

    // Close the listener socket
    void stopListening() {
        closeSocket();
    }

    ~TCPListener() {
        stopListening();
#ifdef _WIN32
        WSACleanup();
#endif
    }

private:
    u_short port;
    SOCKET listenerSocket;

    // Helper function to create a socket
    void createSocket() {
        listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenerSocket == INVALID_SOCKET) {
            throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to create socket");
        }
    }

    // Helper function to bind the socket to a specific port
    void bindSocket() {
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port); // Use htons for u_short
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(listenerSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
            throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to bind socket");
        }
    }

    // Helper function to listen for incoming connections
    void listenForConnections() {
        if (listen(listenerSocket, SOMAXCONN) == SOCKET_ERROR) {
            throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to listen for connections");
        }
    }

    // Helper function to accept incoming connections
    SOCKET acceptConnection() {
        sockaddr_in clientAddress{};
        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(listenerSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept connection" << std::endl;
            return INVALID_SOCKET;
        }

        // Use InetNtop to convert the client's IP address to a string
        WCHAR clientIP[INET6_ADDRSTRLEN];
        if (InetNtopW(AF_INET, &(clientAddress.sin_addr), clientIP, sizeof(clientIP) / sizeof(clientIP[0])) == NULL) {
            std::cerr << "Failed to convert IP address to string" << std::endl;
            closesocket(clientSocket);
            return INVALID_SOCKET;
        }

        std::wcout << L"Accepted connection from " << clientIP << L":" << ntohs(clientAddress.sin_port) << std::endl;
        return clientSocket;
    }

    // Placeholder function to handle client data (override this for your application)
    void handleClient(SOCKET clientSocket) {
        char buffer[4];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            assert(bytesRead == sizeof(buffer));
//            std::cout << std::format("Received data from client: {} bytes",bytesRead);
            uint32_t len = *reinterpret_cast<uint32_t*>(buffer);
            std::string str;
            str.reserve(len);
            str.resize(len);
            bytesRead = recv(clientSocket, str.data(), len, 0);
            receiver(str);
            // Example: Send a response
            const char* response = "Hello, client!";
            send(clientSocket, response, static_cast<int>(strlen(response)), 0); // Cast to int
        } else if (bytesRead == 0) {
            // Connection closed by the client
            std::cout << "Client disconnected." << std::endl;
        } else {
            // Error during recv
            std::cerr << "Error receiving data from client" << std::endl;
        }
    }

    // Helper function to close the socket
    void closeSocket() {
        closesocket(listenerSocket);
    }
};

int main() {
    try {
        TCPListener tcpListener(8080);
        tcpListener.startListening();
        tcpListener.SetReceiver([](const std::string& str) {
            output("{}\n", str);
            });
        tcpListener.acceptConnections();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
