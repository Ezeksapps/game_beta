#include "p2p_client.hpp"
#include <assert.h>

P2PClient::P2PClient() {}
P2PClient::~P2PClient() {}

bool g_bQuit = false;

void P2PClient::establishConnection(const SteamNetworkingIPAddr& serverAddr, const NetworkType& networkType) {
  //  m_listenerSocket = CreateListenSocketP2P()
}

/*
void P2PClient::connectToHost(const SteamNetworkingIPAddr& serverAddr) {

    m_pInterface = SteamNetworkingSockets();



    // start connection, NOTE: k_cchMaxString = 48, which is the size of an IPv6 address
    char addrBuffer[SteamNetworkingIPAddr::k_cchMaxString];
    serverAddr.ToString(addrBuffer, sizeof(addrBuffer), true); // << Use to show address in logs later

    // config options
    SteamNetworkingConfigValue_t opt;
    // set callback to invoke whenever connection status changes
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)connectionStatusChangedCallback);
    // connect to the host
    m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
    // if ( m_hConnection == k_HSteamNetConnection_Invalid) FatalError("Failed to create connection");

    while (!g_bQuit) {
        pollIncomingMessages();
        pollConnectionStateChanges();
        pollLocalUserInput();
    }
}*/
/*
void P2PClient::pollIncomingMessages() {
    while (!g_bQuit) {
        ISteamNetworkingMessage *pIncomingMsg = nullptr;
        int numMsgs = m_pInterface->ReceiveMessagesOnConnection( m_hConnection, &pIncomingMsg, 1 );
        //if ( numMsgs == 0 ) break;
        //if ( numMsgs < 0 ) FatalError( "Error checking for messages" );

        // Just echo anything we get from the server

        // We don't need this anymore.
        pIncomingMsg->Release();
    }
}

void P2PClient::pollLocalUserInput() {
   //m_pInterface->SendMessageToConnection( m_hConnection, cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable, nullptr );
}

void P2PClient::onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

    // What's the state of the connection?
    switch (pInfo->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
            g_bQuit = true;

            // Print an appropriate message
            if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
                // connection attempt failed
            }
            else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                // lost connection w/ host
            }
            else {

                // host terminated connection
            }

            // Clean up the connection.  This is important!
            // The connection is "closed" in the network sense, but
            // it has not been destroyed.  We must close it on our end, too
            // to finish up.  The reason information do not matter in this case,
            // and we cannot linger because it's already closed on the other end,
            // so we just pass 0's.
            m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
            m_hConnection = k_HSteamNetConnection_Invalid;
            break;
        case k_ESteamNetworkingConnectionState_Connecting:
            // We will get this callback when we start connecting.
            // We can ignore this.
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            // connection successful
            break;

        default:
            // Silences -Wswitch
            break;
    }
}

void P2PClient::connectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    m_pCallbackInstance->onConnectionStatusChanged(pInfo);
}

void P2PClient::pollConnectionStateChanges() {
    m_pCallbackInstance = this;
    m_pInterface->RunCallbacks();
}*/
