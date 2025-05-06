#include "Socket.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    std::string password = argv[2];

    try {
        Socket server(port, password);
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
