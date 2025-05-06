#include "Socket.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h> // Para o poll()
#include <cstdlib>

Socket::Socket(const std::string &port, const std::string &password)
    : _port(port), _password(password), _parser(*this) {
    initSocket();
}

Socket::~Socket() {
    close(_serverSocket);
    std::map<int, Client>::iterator it = _clients.begin();
    while (it != _clients.end()) {
        close(it->first);
        ++it;
    }
}

void Socket::initSocket() {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(_port.c_str()));

    if (bind(_serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(_serverSocket, 10) < 0) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    struct pollfd pfd;
    pfd.fd = _serverSocket;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);
}

void Socket::run() {
    while (true) {
        int ret = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ret < 0) {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            continue;
        }

        size_t i = 0;
        while (i < _pollfds.size()) {
            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _serverSocket)
                    handleNewConnection();
                else
                    handleClientData(_pollfds[i].fd);
            }
            ++i;
        }
    }
}

void Socket::handleNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int clientSocket = accept(_serverSocket, (struct sockaddr *)&clientAddr, &addrlen);
    if (clientSocket < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }

    fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    struct pollfd pfd;
    pfd.fd = clientSocket;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);

    _clients[clientSocket] = Client(clientSocket);

    std::cout << "New connection accepted: " << clientSocket << std::endl;
}

void Socket::handleClientData(int clientSocket) {
    char buffer[512];
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        disconnectClient(clientSocket);
        return;
    }

    buffer[bytes] = '\0';
    _clients[clientSocket].appendToBuffer(buffer);

    // Agora chamamos o Parser para processar o comando do cliente
    _parser.parseCommand(clientSocket, _clients[clientSocket].getBuffer());
    _clients[clientSocket].clearBuffer();
}

// void Socket::disconnectClient(int clientSocket) {
//     std::map<std::string, std::vector<Client*> >::iterator chIt = _channels.begin();
//     while (chIt != _channels.end()) {
//         std::vector<Client*>& clients = chIt->second;
//         for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ) {
//             if ((*it)->getSocket() == clientSocket)
//                 it = clients.erase(it);
//             else
//                 ++it;
//         }
//         ++chIt;
//     }

//     close(clientSocket);
//     _clients.erase(clientSocket);
//     std::cout << "Client disconnected: " << clientSocket << std::endl;
// }

void Socket::disconnectClient(int clientSocket) {
    std::map<std::string, std::vector<Client*> >::iterator chIt = _channels.begin();
    while (chIt != _channels.end()) {
        std::vector<Client*>& clients = chIt->second;
        for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ) {
            if ((*it)->getSocket() == clientSocket)
                it = clients.erase(it);
            else
                ++it;
        }
        ++chIt;
    }

    close(clientSocket);
    _clients.erase(clientSocket);
    std::cout << "Client disconnected: " << clientSocket << std::endl;
}


// Métodos necessários para o IRC

Client &Socket::getClient(int clientSocket) {
    return _clients.at(clientSocket);
}

const std::string &Socket::getPassword() const {
    return _password;
}

void Socket::sendToClient(const Client &client, const std::string &message) {
    send(client.getSocket(), message.c_str(), message.length(), 0);
}

void Socket::addClientToChannel(Client &client, const std::string &channel) {
    // Cria canal se não existir
    if (_channels.find(channel) == _channels.end()) {
        _channels[channel] = std::vector<Client*>();
    }

    // Adiciona o cliente à lista
    _channels[channel].push_back(&client);
    client.addChannel(channel); // IMPORTANTE: você precisa guardar no Client os canais aos quais ele pertence

    std::string nick = client.getNickname();
    std::string user = client.getUsername(); // Se tiver implementado isso
    std::string host = "localhost"; // Pode customizar

    // JOIN
    std::string joinMsg = ":" + nick + "!" + user + "@" + host + " JOIN :" + channel + "\r\n";
    sendToClient(client, joinMsg);

    // TOPIC (332) — você pode ter um map para armazenar tópicos por canal
    std::string topic = "No topic is set"; // ou algo armazenado
    std::string topicMsg = ":ircserv 332 " + nick + " " + channel + " :" + topic + "\r\n";
    sendToClient(client, topicMsg);

    // NAMES (353)
    std::string namesList;
    for (std::vector<Client*>::iterator it = _channels[channel].begin(); it != _channels[channel].end(); ++it) {
        namesList += (*it)->getNickname() + " ";
    }
    std::string namesMsg = ":ircserv 353 " + nick + " = " + channel + " :" + namesList + "\r\n";
    sendToClient(client, namesMsg);

    // End of NAMES (366)
    std::string endNamesMsg = ":ircserv 366 " + nick + " " + channel + " :End of /NAMES list.\r\n";
    sendToClient(client, endNamesMsg);
}


void Socket::sendMessageToTarget(const Client &client, const std::string &target, const std::string &message) {
    if (_channels.find(target) != _channels.end()) {
        std::vector<Client*>::iterator it = _channels[target].begin();
        while (it != _channels[target].end()) {
            if ((*it)->getSocket() != client.getSocket()) // evita enviar para si mesmo
                sendToClient(**it, client.getNickname() + ": " + message);
            ++it;
        }
    } else {
        try {
            sendToClient(getClientByNickname(target), client.getNickname() + ": " + message);
        } catch (const std::exception &e) {
            std::cerr << "Target client not found: " << target << std::endl;
        }
    }
}


void Socket::kickUserFromChannel(Client &client, const std::string &channel, const std::string &user) {
    // Remove o usuário do canal
    std::vector<Client*>& channelClients = _channels[channel];
    std::vector<Client*>::iterator it = channelClients.begin();
    while (it != channelClients.end()) {
        if ((*it)->getNickname() == user) {
            channelClients.erase(it);
            std::string kickMessage = user + " has been kicked from the channel.\n";
            sendToClient(client, kickMessage);
            break;
        }
        ++it;
    }
}

void Socket::inviteUserToChannel(Client &client, const std::string &channel, const std::string &user) {
    (void)client; // Se ainda não for usado

    std::string inviteMessage = ":" + client.getNickname() + " INVITE " + user + " :" + channel + "\r\n";

    try {
        Client &targetClient = getClientByNickname(user);
        sendToClient(targetClient, inviteMessage);
    } catch (const std::exception &e) {
        // Trate erro, se necessário
    }
}

void Socket::setChannelTopic(Client &client, const std::string &channel, const std::string &topic) {
    // Atualiza o tópico do canal (no futuro, deve-se armazenar o tópico de cada canal)
    std::string topicMessage = "Topic for channel " + channel + " has been set to: " + topic + "\n";
    sendMessageToTarget(client, channel, topicMessage);
}

void Socket::setInviteOnlyMode(Client &client, const std::string &channel) {
    // Marca o canal como "invite-only"
    std::string inviteOnlyMessage = "Channel " + channel + " is now invite-only.\n";
    sendMessageToTarget(client, channel, inviteOnlyMessage);
}

void Socket::removeInviteOnlyMode(Client &client, const std::string &channel) {
    // Remove o modo "invite-only"
    std::string removeInviteMessage = "Channel " + channel + " is no longer invite-only.\n";
    sendMessageToTarget(client, channel, removeInviteMessage);
}

void Socket::setModeratedMode(Client &client, const std::string &channel) {
    // Marca o canal como "moderated"
    std::string moderatedMessage = "Channel " + channel + " is now moderated.\n";
    sendMessageToTarget(client, channel, moderatedMessage);
}

void Socket::removeModeratedMode(Client &client, const std::string &channel) {
    // Remove o modo "moderated"
    std::string removeModeratedMessage = "Channel " + channel + " is no longer moderated.\n";
    sendMessageToTarget(client, channel, removeModeratedMessage);
}

Client& Socket::getClientByNickname(const std::string &nickname) {
    std::map<int, Client>::iterator it = _clients.begin();
    while (it != _clients.end()) {
        if (it->second.getNickname() == nickname) {
            return it->second;
        }
        ++it;
    }
    throw std::runtime_error("Client not found");
}

