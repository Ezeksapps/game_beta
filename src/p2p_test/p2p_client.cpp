#include "p2p_client.hpp"
#include <assert.h>

P2PClient::P2PClient() {
    NodeConfig cfg;
    cfg.listen_port = 8080;
    cfg.bind_address = "::";

    m_pNode = std::make_unique<Node>(cfg);
}

P2PClient::~P2PClient() {}

bool g_bQuit = false;

//void P2PClient::establishConnection(const & serverAddr, const NetworkType& networkType) {
  //  m_listenerSocket = CreateListenSocketP2P()
//}

