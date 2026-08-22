#include "p2p/isteamnetworkingsockets.h"
#include "p2p/isteamnetworkingutils.h"

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
    void establishConnection(const SteamNetworkingIPAddr& addr, const NetworkType& networkType);


private:

    void pollIncomingMessages();
    //void pollLocalUserInput();
    void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
    static void connectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
    void pollConnectionStateChanges();

    HSteamNetConnection m_connection;
    HSteamListenSocket m_listenerSocket;
    // ISteamNetworkingSockets* m_pInterface;
    static P2PClient* m_pCallbackInstance;
};
