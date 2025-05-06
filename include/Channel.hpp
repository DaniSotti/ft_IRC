#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <set>

class Client; // Declaração antecipada para evitar dependência circular

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::vector<Client*> _clients;
    std::set<Client*> _operators;
    std::set<Client*> _invited;

    bool _inviteOnly;  // Modo +i (somente convidados podem entrar)
    std::string _key;
    bool _hasKey;
    int _userLimit;
    bool _hasUserLimit;
    bool _topicRestricted;

public:
    Channel(const std::string& name);
    ~Channel();

    const std::string& getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);

    void addClient(Client* client);
    void removeClient(Client* client);
    const std::vector<Client*>& getClients() const;

    bool hasClient(Client* client) const;

    // Operadores
    void setOperator(Client* client);
    bool isOperator(Client* client) const;

    // Invite-only (+i)
    void setInviteOnly(bool value);
    bool isInviteOnly() const;

    void inviteClient(Client* client);
    bool isInvited(Client* client) const;

    // Retorna os modos como uma string, ex: "+i"
    std::string getModeString() const;
    void setTopicRestricted(bool value);
    bool isTopicRestricted() const;
    
    // +k
    void setKey(const std::string& key);
    
    void removeKey();
    
    bool checkKey(const std::string& key) const;
    bool hasKey() const;
    
    // +l
    void setUserLimit(int limit);
    
    void removeUserLimit();
    
    bool isFull() const;

    void addOperator(Client* client);
    void removeOperator(Client* client);
   
    
};

#endif
