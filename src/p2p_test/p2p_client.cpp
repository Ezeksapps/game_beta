#include "p2p_client.hpp"
#include "librats/subsystems/file_transfer.h"
#include <assert.h>
#include <stdexcept>

P2PClient::P2PClient() {
    NodeConfig cfg;
    cfg.listen_port = 8080;
    cfg.bind_address = "::";

    m_pNode = std::make_unique<Node>(cfg);
    m_pNode->add_subsystem(std::make_unique<FileTransfer>()); // will be used later for save file transfers

    m_pNode->on_peer_connected([](const Peer& peer) {

    });
    m_pNode->on("rcvData", [](const Peer& peer, ByteView data) {

    });

    if (!m_pNode->start()) throw std::runtime_error("Could not start a node");
}

P2PClient::~P2PClient() {}

void P2PClient::establishConnection(const std::string& addr, const NetworkType& networkType) {
    m_pNode->connect(addr, 8081);
}

template<typename T> void P2PClient::sendData(T data) {
  //  m_pNode->send(, , )
}

