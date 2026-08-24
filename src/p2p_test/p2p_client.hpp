#include "librats/node/node.h"

using namespace librats;

/* Peer-to-peer connection handler */

enum NetworkType : uint8_t {
    CONNECTION_LAN,
    CONNECTION_INTERNET
};

class P2PClient {

public:

    P2PClient();
    ~P2PClient();

    /* Establish a connection with another client at the specified IP address (which can either be IPv4 or IPV6)
     * that is in a network of the specified type (LAN or global internet)
     */
    void establishConnection(const std::string& addr, const NetworkType& networkType);
    void disconnect(const std::string& addr);

private:

    std::unique_ptr<Node> m_pNode;
};
