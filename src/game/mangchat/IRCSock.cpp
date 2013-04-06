/*
 * MangChat for MaNGOS, the open source MMORPG-server
 *
 * This Program Is Free Software; You Can Redistribute It And/Or Modify It Under The
 * Terms Of The GNU General Public License
 *
 * Written and Developed by Cybrax <cybraxvd@gmail.com>, |Death| <death@hell360.net>,
 * Lice <lice@yeuxverts.net>, Dj_baby, Sanaell, Tase, Shinzon <shinzon@wowgollum.com>,
 * Xeross, 3raZar3, the orangevirus team <www.orangevir.us>, ...
 *
 * Rewritten by kid10
 *
 * With Help And Support From The MaNGOS Project Community.
 * PLEASE RETAIN THE COPYRIGHT OF THE AUTHORS.
 */

#include "IRCClient.h"
#include "../../shared/Log.h"

bool IRCClient::InitSock()
{
    #ifdef _WIN32
    WSADATA wsaData;                                        //WSAData
    if(WSAStartup(MAKEWORD(2,0),&wsaData) != 0)
    {
        sLog.outError("IRC Error: Winsock Initialization Error");
        return false;
    }
    #endif
    if ((SOCKET = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        sLog.outError("IRC Error: Socket Error");
        return false;
    }
    int on = 1;
    if ( setsockopt ( SOCKET, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    {
        sLog.outError("IRC Error: Invalid Socket");
        return false;
    }
    #ifdef _WIN32
    u_long iMode = 0;
    ioctlsocket(SOCKET, FIONBIO, &iMode);
    #else
    fcntl(SOCKET, F_SETFL, O_NONBLOCK);                // set to non-blocking
    fcntl(SOCKET, F_SETFL, O_ASYNC);                   // set to asynchronous I/O
    #endif
    return true;
}

bool IRCClient::Connect(const char* cHost, int nPort)
{
    m_bConnected = false;

    struct hostent *he;
    if ((he=gethostbyname(cHost)) == NULL)
    {
        sLog.outError("IRCLIENT: Could not resolve host: %s", cHost);
        return false;
    }
    struct sockaddr_in their_addr;
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(nPort);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);
    if (::connect(SOCKET, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        sLog.outError("IRCLIENT: Cannot connect to %s", cHost);
        return false;
    }
    //FD_ZERO(&sfdset);
    //FD_SET(SOCKET,&sfdset);

    m_bConnected = true;

    return true;
}

bool IRCClient::Login(std::string sNick, std::string sUser, std::string sPass)
{
    char hostname[128];
    gethostname(hostname, sizeof(hostname));

    if (SendToIRC("HELLO\n"))
        if (SendToIRC("PASS " + sPass + "\n"))
            if (SendToIRC("NICK " + sNick + "\n"))
                if (SendToIRC("USER " + sUser + " " + hostname + " " + sNick + " :" + sNick + "\n"))
                    return true;

    return false;
}

void IRCClient::SockRecv()
{
    char szBuffer[512]; // Max Data Size

    memset(szBuffer, 0, 512);

    int nBytesRecv = ::recv(SOCKET, szBuffer, 512 - 1, 0 );
    if (nBytesRecv == -1)
    {
        sLog.outError("Connection lost.");
        m_bConnected = false;
    }
    else
    {
        if (-1 == nBytesRecv)
        {
            sLog.outError("Error occurred while receiving from socket.");
        }
        else
        {
            std::string reply;
            std::istringstream iss(szBuffer);
            while (getline(iss, reply))
            {
                HandleReceivedData(reply);
            }
        }
    }
}

void IRCClient::Disconnect()
{
    if (SOCKET)
    {
        #ifdef _WIN32
        closesocket(SOCKET);
        //WSACleanup();
        #else
        close(SOCKET);
        #endif
    }
}
