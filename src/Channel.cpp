#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _inviteOnly(false) {}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

void Channel::addClient(Client* client) {
    if (!hasClient(client)) {
        _clients.push_back(client);
    }
}

void Channel::removeClient(Client* client) {
    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) {
        _clients.erase(it);
    }
    _operators.erase(client);
    _invited.erase(client);
}

const std::vector<Client*>& Channel::getClients() const {
    return _clients;
}

bool Channel::hasClient(Client* client) const {
    return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

// ---------- Operadores ----------

// void Channel::setOperator(Client* client) {
//     _operators.insert(client);
// }

void Channel::addOperator(Client* client) {
    _operators.insert(client);
}

void Channel::removeOperator(Client* client) {
    _operators.erase(client);
}


bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

// ---------- Invite-only mode (+i) ----------

void Channel::setInviteOnly(bool value) {
    _inviteOnly = value;
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

void Channel::inviteClient(Client* client) {
    _invited.insert(client);
}

bool Channel::isInvited(Client* client) const {
    return _invited.find(client) != _invited.end();
}

// ---------- Modo atual como string (ex: "+i") ----------

std::string Channel::getModeString() const {
    std::string modes = "+";
    if (_inviteOnly) modes += "i";
    if (_topicRestricted) modes += "t";
    if (_hasKey) modes += "k";
    if (_hasUserLimit) modes += "l";
    if (modes == "+") return "";
    return modes;
}

// +t
void Channel::setTopicRestricted(bool value) {
    _topicRestricted = value;
}

bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

// +k
void Channel::setKey(const std::string& key) {
    _key = key;
    _hasKey = true;
}

void Channel::removeKey() {
    _key = "";
    _hasKey = false;
}

bool Channel::checkKey(const std::string& key) const {
    return _hasKey && _key == key;
}

bool Channel::hasKey() const {
    return _hasKey;
}

// +l
void Channel::setUserLimit(int limit) {
    _userLimit = limit;
    _hasUserLimit = true;
}

void Channel::removeUserLimit() {
    _userLimit = 0;
    _hasUserLimit = false;
}

bool Channel::isFull() const {
    return _hasUserLimit && static_cast<int>(_clients.size()) >= _userLimit;
}
