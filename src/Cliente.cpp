#include "Client.hpp"

Client::Client()
    : _socket(-1), _buffer(""), _nickname(""), _username(""), _authenticated(false) {}

Client::Client(int socket)
    : _socket(socket), _buffer(""), _nickname(""), _username(""), _authenticated(false) {}

Client::~Client() {}

int Client::getSocket() const {
    return _socket;
}

std::string &Client::getBuffer() {
    return _buffer;
}

void Client::appendToBuffer(const std::string &data) {
    _buffer += data;
}

void Client::clearBuffer() {
    _buffer.clear();
}

void Client::setNickname(const std::string &nickname) {
    _nickname = nickname;
}

void Client::setUsername(const std::string &username) {
    _username = username;
}

void Client::authenticate() {
    _authenticated = true;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

const std::string &Client::getNickname() const {
    return _nickname;
}

void Client::addChannel(const std::string& channel) {
    _channels.insert(channel); // Um std::set<std::string> _channels;
}

const std::string &Client::getUsername() const {
    return _username;
}

bool Client::hasUsername() const {
    return !_username.empty();
}

void Client::setRegistered(bool registered) {
    _registered = registered;
}

std::string Client::getPrefix() const {
    return _nickname + "!" + _username + "@localhost";
}


