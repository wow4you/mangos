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

#ifndef _IRC_CLIENT_H
#define _IRC_CLIENT_H

#include "Common.h"
#include "../Player.h"

using namespace std;

enum HardcodedOptions
{
    MAX_CONNECT_ATTEMPT = 20,
    WAIT_CONNECT_TIME = 30000
};

enum MangChatAuthMethods
{
    AUTH_NO_AUTH = 0,
    AUTH_NICKSERV_PW = 1,
    AUTH_NICKSERV_USER_AND_PW = 2,
    MAX_AUTH_METHODS = 3
};

enum MangChatChannelOptions
{
    OPTION_LINK_WITH_OTHER_CHANNEL = 1,
    OPTION_DISPLAY_JOINS = 2,
    OPTION_DISPLAY_LEAVES = 4,
    OPTION_DISPLAY_GM_TAG = 8,
    OPTION_DISPLAY_KICKS = 16
};

struct MangChatChannel
{
    public:
        MangChatChannel(string _sChannel, uint16 _uiOptions) :
            sChannel(_sChannel),
            uiOptions(_uiOptions)
            { }

        string sChannel;
        uint16 uiOptions;
};

typedef list<MangChatChannel> ChannelList;
typedef map<string, ChannelList> ChannelLinkMap;

// IRCClient main class
class IRCClient : public ACE_Based::Runnable
{
    public:
        IRCClient(string sHost, int iPort, string sUser, string sPass, string sNick, ChannelLinkMap WoWChannelLinks, ChannelLinkMap IRCChannelLinks, uint8 uiAuth);
        ~IRCClient();

        // ZThread Entry
        void run();

        // This function is called in Channel.cpp
        void HandleJoinWoWChannel(Player* pPlayer, string sChannel);
        // This function is called in Channel.cpp
        void HandleLeaveWoWChannel(Player* pPlayer, string sChannel);
        // This function is called in ChatHandler.cpp
        void HandleSendMessage(Player* pPlayer, string sMessage, string sChannel);

    private:
        // Initialize socket library
        bool InitSock();
        // Connect to IRC Server
        bool Connect(const char* cHost, int nPort);
        // Login to IRC Server
        bool Login(std::string sNick, std::string sUser, std::string sPass);
        // Receives data from the socket.
        void SockRecv();
        // Processes the data received from IRC
        void HandleReceivedData(std::string sData);

        // Send raw data to IRC
        bool SendToIRC(string sData);
        // Send a message to the specified IRC channel
        bool SendToIRCChannel(std::string sChannel, std::string sMessage);
        // Sends a message to all players on the specified channel
        void SendToWoWChannel(string sChannel, std::string sMessage);

        /* Cut a piece out of a string
         */
        string GetSubstring(string sContent, int iStartIdx, int iEndIdx);

        /* Searches the given string content
           Returns -1 or the found position plus iIdxPlus
         */
        int FindSubstring(string sContent, string sFind, int iOffset, int iIdxPlus = 0);

        // Disconnect from IRC and cleanup socket
        void Disconnect();

        /* IRCClient active
         */
        bool m_bActive;

        /* Connected to IRC
         */
        bool m_bConnected;

        /* IRC Server Host
         */
        string m_sHost;

        /* IRC Server Port
         */
        int m_iPort;

        /* IRC Username
         */
        string m_sUser;

        /* IRC Password
         */
        string m_sPass;

        /* IRC Nickname
         */
        string m_sNick;

        /* WoW Channel : IRC Channel
         */
        ChannelLinkMap m_mWoWLinks;

        /* IRC Channel : WoW Channel
         */
        ChannelLinkMap m_mIRCLinks;

        /* Authenticationmethod
         */
        uint8 m_uiAuth;

        // Socket indentifier
        int SOCKET;
};
#endif
