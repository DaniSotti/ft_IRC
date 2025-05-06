#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
//#include "Socket.hpp"
#include <set>

class Client {
private:
    int _socket;
    std::string _buffer;
    std::string _nickname;
    std::string _username;
    bool _authenticated;
    std::set<std::string> _channels;
    bool _registered;

public:
    Client();
    Client(int socket);
    ~Client();

    int getSocket() const;
    std::string &getBuffer();
    void appendToBuffer(const std::string &data);
    void clearBuffer();

    void setNickname(const std::string &nickname);
    void setUsername(const std::string &username);
    void authenticate();
    bool isAuthenticated() const;

    const std::string &getNickname() const;
    void addChannel(const std::string& channel);
    const std::string &getUsername() const;
    bool hasUsername() const;
    void setRegistered(bool registered);
    std::string getPrefix() const;
    

};

#endif
