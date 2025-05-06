#include "Parser.hpp"
#include "Socket.hpp"
#include <sstream>
#include <iostream>

Parser::Parser(Socket &socket) : _socket(socket) {}

Parser::~Parser() {
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
}

std::vector<std::string> Parser::split(const std::string &input) {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    while (stream >> token) {
        if (token[0] == ':') {
            std::string rest;
            getline(stream, rest);
            token += rest; // adiciona o restante da linha
            tokens.push_back(token);
            break;
        }
        tokens.push_back(token);
    }
    return tokens;
}


void Parser::parseCommand(int clientSocket, const std::string &input) {
    Client &client = _socket.getClient(clientSocket);
    std::vector<std::string> args = split(input);
    if (args.empty())
        return;
   
    const std::string &command = args[0];
    // if (!client.isAuthenticated() && command != "PASS" && command != "NICK" && command != "USER") {
    //     _socket.sendToClient(client.getSocket(), "451 :You have not registered\r\n");
    //     return;
    // }
    if (command == "PASS") handlePASS(client, args);
    else if (command == "NICK") handleNICK(client, args);
    else if (command == "USER") handleUSER(client, args);
    else if (command == "JOIN"){
        std::cout << "JOIN recebido de " << client.getNickname() << " para o canal " << args[1] << std::endl;
        handleJOIN(client, args);
    } 
    else if (command == "PRIVMSG") handlePRIVMSG(client, args);
    else if (command == "KICK") handleKICK(client, args);
    else if (command == "INVITE") handleINVITE(client, args);
    else if (command == "TOPIC") handleTOPIC(client, args);
    else if (command == "MODE") handleMODE(client, args);
}

void Parser::handlePASS(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 2) return;
    if (args[1] == _socket.getPassword()) client.authenticate();
}

void Parser::handleNICK(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 2) {
        _socket.sendToClient(client, ":ircserv 431 * :No nickname given\r\n");
        return;
    }

    std::string newNick = args[1];

    // Verifica se já existe alguém com esse nickname
    try {
        _socket.getClientByNickname(newNick);
        _socket.sendToClient(client, ":ircserv 433 * " + newNick + " :Nickname is already in use\r\n");
        return;
    } catch (const std::exception&) {
        // OK, não encontrado, podemos usar
    }

    client.setNickname(newNick);
    if (client.isAuthenticated() && client.hasUsername()) {
        client.setRegistered(true);
        _socket.sendToClient(client, ":ircserv 001 " + newNick + " :Welcome to the IRC server!\r\n");
    }
}


void Parser::handleUSER(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 2) return;
    client.setUsername(args[1]);
}

void Parser::handleJOIN(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 2) {
        _socket.sendToClient(client, "461 JOIN :Not enough parameters\r\n");
        return;
    }

    std::string channelName = args[1];
    std::string key = (args.size() >= 3 ? args[2] : "");

    Channel* channel = _channels[channelName];

    // Se o canal ainda não existir, crie e torne o usuário operador
    if (!channel) {
        channel = new Channel(channelName);
        _channels[channelName] = channel;
        channel->addClient(&client);
        channel->addOperator(&client);

        _socket.sendToClient(client, ":" + client.getPrefix() + " JOIN :" + channelName + "\r\n");
        return;
    }

    // Canal já existe: checar modos
    if (channel->isInviteOnly() && !channel->isInvited(&client)) {
        _socket.sendToClient(client, "473 " + channelName + " :Cannot join channel (+i)\r\n");
        return;
    }

    if (channel->hasKey() && !channel->checkKey(key)) {
        _socket.sendToClient(client, "475 " + channelName + " :Cannot join channel (+k)\r\n");
        return;
    }

    if (channel->isFull()) {
        _socket.sendToClient(client, "471 " + channelName + " :Cannot join channel (+l)\r\n");
        return;
    }

    if (channel->hasClient(&client)) {
        return; // já está no canal
    }

    channel->addClient(&client);

    _socket.sendToClient(client, ":" + client.getPrefix() + " JOIN :" + channelName + "\r\n");
}


// void Parser::handleJOIN(Client &client, const std::vector<std::string> &args) {
//     if (args.size() < 2) {
//         _socket.sendToClient(client.getSocket(), "461 JOIN :Not enough parameters\r\n");
//         return;
//     }

//     std::string channelName = args[1];

//     // Verifica se o canal começa com '#' (requisito padrão do IRC)
//     if (channelName[0] != '#') {
//         _socket.sendToClient(client.getSocket(), "403 " + channelName + " :No such channel\r\n");
//         return;
//     }

//     Channel *channel;
//     std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
//     if (it == _channels.end()) {
//         // Cria o canal se ele não existir
//         channel = new Channel(channelName);
//         _channels[channelName] = channel;
//     } else {
//         channel = it->second;
//     }

//     if (channel->hasClient(&client)) {
//         _socket.sendToClient(client.getSocket(), "443 " + channelName + " :You're already on that channel\r\n");
//         return;
//     }

//     channel->addClient(&client);

//     // Mensagem de confirmação JOIN
//     std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " JOIN :" + channelName + "\r\n";
//     const std::vector<Client*>& clients = channel->getClients();
//     for (size_t i = 0; i < clients.size(); ++i) {
//         _socket.sendToClient(clients[i]->getSocket(), joinMsg);
//     }

//     // Enviar o tópico (se houver)
//     if (channel->getTopic() != "")
//         _socket.sendToClient(client.getSocket(), "332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");

//     // Lista de nomes no canal
//     std::string names = "=" + channelName + " :";
//     for (size_t i = 0; i < clients.size(); ++i) {
//         names += clients[i]->getNickname() + " ";
//     }
//     _socket.sendToClient(client.getSocket(), "353 " + client.getNickname() + " " + names + "\r\n");
//     _socket.sendToClient(client.getSocket(), "366 " + client.getNickname() + " " + channelName + " :End of /NAMES list\r\n");
// }


void Parser::handlePRIVMSG(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 3) return;

    std::string message;
    for (size_t i = 2; i < args.size(); ++i) {
        if (i > 2)
            message += " ";
        message += args[i];
    }
    if (message[0] == ':')
        message = message.substr(1); // remove o ':' inicial

    _socket.sendMessageToTarget(client, args[1], message);
}

void Parser::handleKICK(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 3) return;
    _socket.kickUserFromChannel(client, args[1], args[2]);
}

void Parser::handleINVITE(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 3) return;
    _socket.inviteUserToChannel(client, args[1], args[2]);
}

void Parser::handleTOPIC(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 3) return;

    std::string topic;
    for (size_t i = 2; i < args.size(); ++i) {
        if (i > 2)
            topic += " ";
        topic += args[i];
    }
    if (topic[0] == ':')
        topic = topic.substr(1);

    _socket.setChannelTopic(client, args[1], topic);
}

void Parser::handleMODE(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 3) return;

    std::string channelName = args[1];
    std::string mode = args[2];
    Channel *channel = _channels[channelName];

    if (!channel) {
        _socket.sendToClient(client, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!channel->isOperator(&client)) {
        _socket.sendToClient(client, "482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    bool adding = (mode[0] == '+');
    char flag = mode[1];

    if (flag == 'i') channel->setInviteOnly(adding);
    else if (flag == 't') channel->setTopicRestricted(adding);
    else if (flag == 'k') {
        if (args.size() < 4) return;
        if (adding) channel->setKey(args[3]);
        else channel->removeKey();
    }
    else if (flag == 'l') {
        if (adding) {
            if (args.size() < 4) return;
            int limit = atoi(args[3].c_str());
            channel->setUserLimit(limit);
        } else {
            channel->removeUserLimit();
        }
    }
    else if (flag == 'o') {
        if (args.size() < 4) return;
        try {
            Client &target = _socket.getClientByNickname(args[3]);
            if (adding)
                channel->addOperator(&target);
            else
                channel->removeOperator(&target);
        } catch (...) {
            _socket.sendToClient(client, "401 " + args[3] + " :No such nick\r\n");
        }
    }
}

