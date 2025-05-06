#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <cstdlib>
#include "Client.hpp"
#include <map>
#include "Channel.hpp"

class Socket;

class Parser {
public:
    Parser(Socket &socket);
    ~Parser();

    void parseCommand(int clientSocket, const std::string &input);

private:
    Socket &_socket;

    std::map<std::string, Channel*> _channels;
    std::vector<std::string> split(const std::string &input);
    void handlePASS(Client &client, const std::vector<std::string> &args);
    void handleNICK(Client &client, const std::vector<std::string> &args);
    void handleUSER(Client &client, const std::vector<std::string> &args);
    void handleJOIN(Client &client, const std::vector<std::string> &args);
    void handlePRIVMSG(Client &client, const std::vector<std::string> &args);
    void handleKICK(Client &client, const std::vector<std::string> &args);
    void handleINVITE(Client &client, const std::vector<std::string> &args);
    void handleTOPIC(Client &client, const std::vector<std::string> &args);
    void handleMODE(Client &client, const std::vector<std::string> &args);
};

#endif
