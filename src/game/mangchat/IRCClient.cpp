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
#include "../ObjectMgr.h"
#include "../World.h"
#include "../ChannelMgr.h"
#include "../Channel.h"
#include "../Chat.h"

IRCClient::IRCClient(string sHost, int iPort, string sUser, string sPass, string sNick, ChannelLinkMap WoWChannelLinks, ChannelLinkMap IRCChannelLinks, uint8 uiAuth) :
m_bActive(true),
m_sHost(sHost),
m_iPort(iPort),
m_sUser(sUser),
m_sPass(sPass),
m_sNick(sNick),
m_mWoWLinks(WoWChannelLinks),
m_mIRCLinks(IRCChannelLinks),
m_uiAuth(uiAuth)
{
}

IRCClient::~IRCClient()
{
    m_bActive = false;
}

// ZThread Entry This function is called when the thread is created in Master.cpp (mangosd)
void IRCClient::run()
{
    // Before we begin we wait a while MaNGOS is still starting up.
    ACE_Based::Thread::Sleep(500);

    /* Connection count
     */
    int cCount = 0;

    // Create a loop to keep the thread running untill active is set to false
    while (m_bActive && !World::IsStopped())
    {
        // Initialize socket library
        if (InitSock())
        {
            // Connect To The IRC Server
            sLog.outString("MangChat: Connecting to %s Try # %d", m_sHost.c_str(), cCount);

            if (Connect(m_sHost.c_str(), m_iPort))
            {
                sLog.outString("MangChat: Connected And Logging In");

                // On connection success reset the connection counter
                cCount = 0;

                // Login to the IRC server
                if (Login(m_sNick, m_sUser, m_sPass))
                {
                    sLog.outString("MangChat: Logged In And Running!!");

                    // While we are connected to the irc server keep listening for data on the socket
                    while (m_bConnected && !World::IsStopped())
                    {
                        SockRecv();
                    }
                }

                sLog.outString("MangChat: Connection To IRC Server Lost!");
            }

            // When an error occures or connection lost cleanup
            Disconnect();

            // Increase the connection counter
            ++cCount;

            // if MAX_CONNECT_ATTEMPT is reached stop trying
            if (cCount == MAX_CONNECT_ATTEMPT)
                m_bActive = false;

            // If we need to reattempt a connection wait WAIT_CONNECT_TIME milli seconds before we try again
            if (m_bActive)
                ACE_Based::Thread::Sleep(WAIT_CONNECT_TIME);
        }
        else
        {
            // Socket could not initialize cancel
            m_bActive = false;
            sLog.outError("MangChat: Could not initialize socket");
        }
    }
    // thread stays alive for calls from other threads
}

bool IRCClient::SendToIRC(string sData)
{
    if (m_bConnected)
    {
        if (send(SOCKET, sData.c_str(), sData.length(), 0) == -1)
        {
            sLog.outError("IRC Error: Socket Receive ** \n");
            //Disconnect();
            return false;
        }
    }

    return true;
}

bool IRCClient::SendToIRCChannel(std::string sChannel, std::string sMessage)
{
    if (m_bConnected)
        return SendToIRC("PRIVMSG #" + sChannel + " :" + sMessage + "\n");

    return false;
}

void IRCClient::SendToWoWChannel(string sChannel, std::string sMessage)
{
    if (sChannel.length() < 1)
        return;

    // IRC-Protocol does not use a specific character encoding.
    // TODO: Autoencode to UTF8 (as used in the wow client)

    HashMapHolder<Player>::MapType& m = ObjectAccessor::Instance().GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (itr->second && itr->second->GetSession()->GetPlayer() && itr->second->GetSession()->GetPlayer()->IsInWorld())
        {
            if (ChannelMgr* cMgr = channelMgr(itr->second->GetSession()->GetPlayer()->GetTeam()))
            {
                if (Channel* chn = cMgr->GetChannel(sChannel, itr->second->GetSession()->GetPlayer()))
                {
                    WorldPacket data;
                    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, sChannel.c_str(), ObjectGuid(), sMessage.c_str(), NULL);
                    itr->second->GetSession()->SendPacket(&data);
                }
            }
        }
    }
}

string IRCClient::GetSubstring(string sContent, int iStartIdx, int iEndIdx)
{
    if (iStartIdx < 0 || iEndIdx < 0 ||
        iEndIdx <= iStartIdx ||
        sContent.length() < (uint32)iEndIdx)
        return "";

    return sContent.substr(iStartIdx, std::max(1, iEndIdx - iStartIdx));
}

int IRCClient::FindSubstring(string sContent, string sFind, int iOffset, int iIdxPlus)
{
    if (iOffset < 0 || sContent.length() <= (uint32)iOffset)
        return -1;

    int iIndex = sContent.find(sFind.c_str(), iOffset);

    if (iIndex < 0)
    {
        return -1;
    }
    else
        return iIndex + iIdxPlus;
}
