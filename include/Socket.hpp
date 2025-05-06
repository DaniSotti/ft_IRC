#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "Client.hpp"
#include "Parser.hpp"
#include <map>
#include <string>
#include <vector>
#include <iostream>

class Socket {
public:
    Socket(const std::string &port, const std::string &password);
    ~Socket();

    void initSocket();
    void run();
    void handleNewConnection();
    void handleClientData(int clientSocket);
    void disconnectClient(int clientSocket);

    // Métodos que vamos implementar para corrigir os erros
    Client &getClient(int clientSocket);
    const std::string &getPassword() const;
    void sendToClient(const Client &client, const std::string &message);
    void addClientToChannel(Client &client, const std::string &channel);
    void sendMessageToTarget(const Client &client, const std::string &target, const std::string &message);
    void kickUserFromChannel(Client &client, const std::string &channel, const std::string &user);
    void inviteUserToChannel(Client &client, const std::string &channel, const std::string &user);
    void setChannelTopic(Client &client, const std::string &channel, const std::string &topic);
    void setInviteOnlyMode(Client &client, const std::string &channel);
    void removeInviteOnlyMode(Client &client, const std::string &channel);
    void setModeratedMode(Client &client, const std::string &channel);
    void removeModeratedMode(Client &client, const std::string &channel);
    Client& getClientByNickname(const std::string &nickname);
    

private:
    std::string _port;
    std::string _password;
    int _serverSocket;
    std::map<int, Client> _clients;  // Armazenando clientes com seu socket
    std::map<std::string, std::vector<Client*> > _channels;
    std::vector<struct pollfd> _pollfds;
    Parser _parser;
};

#endif
