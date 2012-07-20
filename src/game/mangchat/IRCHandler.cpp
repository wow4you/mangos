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

#include "IRCMgr.h"
#include "IRCClient.h"
#include "../World.h"
#include "../Player.h"
#include "../../shared/Log.h"

void IRCMgr::HandleWoWChannelAction(string sChannel, string sMessage, Player* pPlayer)
{
    for (ClientList::const_iterator itr = m_lClients.begin(); itr != m_lClients.end(); ++itr)
        (*itr)->HandleSendMessage(pPlayer, sMessage, sChannel);
}

void IRCClient::HandleSendMessage(Player* pPlayer, string sMessage, string sChannel)
{
    if (m_bConnected && pPlayer)
    {
        ChannelLinkMap::const_iterator itr = m_mWoWLinks.find(sChannel);

        if (itr != m_mWoWLinks.end())
        {
            ChannelList channels = itr->second;

            for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
            {
                string sName = pPlayer->GetName();

                if (pPlayer->isGameMaster() && (itr2->uiOptions & OPTION_DISPLAY_GM_TAG))
                    sName = "[GM]" + sName;

                SendToIRCChannel(itr2->sChannel, "<WoW>" + sName + ": " + sMessage);
            }
        }
    }
}

void IRCMgr::HandleWoWChannelAction(string sChannel, ChannelAction action, Player* pPlayer)
{
    if (action == ACTION_JOIN_CHANNEL)
    {
        for (ClientList::const_iterator itr = m_lClients.begin(); itr != m_lClients.end(); ++itr)
            (*itr)->HandleJoinWoWChannel(pPlayer, sChannel);
    }
    else if (action == ACTION_LEAVE_CHANNEL)
    {
        for (ClientList::const_iterator itr = m_lClients.begin(); itr != m_lClients.end(); ++itr)
            (*itr)->HandleLeaveWoWChannel(pPlayer, sChannel);
    }
}

void IRCClient::HandleJoinWoWChannel(Player* pPlayer, string sChannel)
{
    if (m_bConnected && pPlayer)
    {
        ChannelLinkMap::const_iterator itr = m_mWoWLinks.find(sChannel);

        if (itr != m_mWoWLinks.end())
        {
            ChannelList channels = itr->second;

            for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
            {
                if (itr2->uiOptions & OPTION_DISPLAY_JOINS)
                {
                    string sName = pPlayer->GetName();

                    if (pPlayer->isGameMaster() && (itr2->uiOptions & OPTION_DISPLAY_GM_TAG))
                        sName = "[GM]" + sName;

                    SendToIRCChannel(itr2->sChannel, sName + " ist dem Channel " + sChannel + " beigetreten!");
                }
            }
        }
    }
}

void IRCClient::HandleLeaveWoWChannel(Player* pPlayer, string sChannel)
{
    if (m_bConnected && pPlayer)
    {
        ChannelLinkMap::const_iterator itr = m_mWoWLinks.find(sChannel);

        if (itr != m_mWoWLinks.end())
        {
            ChannelList channels = itr->second;

            for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
            {
                if (itr2->uiOptions & OPTION_DISPLAY_LEAVES)
                {
                    string sName = pPlayer->GetName();

                    if (pPlayer->isGameMaster() && (itr2->uiOptions & OPTION_DISPLAY_GM_TAG))
                        sName = "[GM]" + sName;

                    SendToIRCChannel(itr2->sChannel, sName + " hat den Channel " + sChannel + " verlassen.");
                }
            }
        }
    }
}

void IRCClient::HandleReceivedData(std::string sData)
{
    if (GetSubstring(sData, 0, 5) == "ERROR")
    {
        Disconnect();
    }
    else if (GetSubstring(sData, 0, 4) == "PING")
    {
        SendToIRC("PONG " + GetSubstring(sData, 4, sData.size() - 4) + "\n");
    }
    else if (GetSubstring(sData, 0, 1) == ":")
    {
        /* Remove \r and \n
         */
        sData.erase(std::remove(sData.begin(), sData.end(), '\r'), sData.end());
        sData.erase(std::remove(sData.begin(), sData.end(), '\n'), sData.end());

        string sUser = GetSubstring(sData, 1, sData.find("!"));
        string sCommand = GetSubstring(sData, FindSubstring(sData, " ", 0, 1), FindSubstring(sData, " ", FindSubstring(sData, " ", 0, 1)));

        if (sCommand == "PRIVMSG")
        {
            string sChannel = GetSubstring(sData, FindSubstring(sData, "#", 0, 1), FindSubstring(sData, " ", FindSubstring(sData, "#", 0, 1)));

            if (sChannel.length() > 0)
            {
                string sMessage = GetSubstring(sData, FindSubstring(sData, ":", 1, 1), sData.length());

                DEBUG_LOG("IRCHandler: %s sends a message to channel %s", sUser.c_str(), sChannel.c_str());

                if (sMessage.length() > 0 && sMessage.length() < 1024)
                {
                    ChannelLinkMap::const_iterator itr = m_mIRCLinks.find(sChannel);

                    if (itr != m_mIRCLinks.end())
                    {
                        ChannelList channels = itr->second;
                        string sFinalMessage = "<IRC>[" + sUser + "]: " + sMessage;

                        for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
                            SendToWoWChannel(itr2->sChannel, sFinalMessage);
                    }
                }
            }
            /* ToDo: Handle private messages
             */
        }
        else if (sCommand == "JOIN")
        {
            string sChannel = GetSubstring(sData, FindSubstring(sData, "#", 0, 1), sData.length()); // Channel without #

            DEBUG_LOG("IRCHandler: %s joins channel %s", sUser.c_str(), sChannel.c_str());

            if (sUser != m_sNick)
            {
                ChannelLinkMap::const_iterator itr = m_mIRCLinks.find(sChannel);

                if (itr != m_mIRCLinks.end())
                {
                    ChannelList channels = itr->second;
                    string sMessage = "<IRC>[" + sUser + "]: Ist dem IRC Channel beigetreten!";

                    for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
                    {
                        if (itr2->uiOptions & OPTION_DISPLAY_JOINS)
                            SendToWoWChannel(itr2->sChannel, sMessage);
                    }
                }
            }
            else
                SendToIRCChannel(sChannel, "MangChat Rewrite meldet sich zum Dienst!");
        }
        else if (sCommand == "PART")
        {
            string sChannel = GetSubstring(sData, sData.find("#") + 1, sData.length()); // Channel without #

            DEBUG_LOG("IRCHandler: %s leaves channel %s", sUser.c_str(), sChannel.c_str());

            ChannelLinkMap::const_iterator itr = m_mIRCLinks.find(sChannel);

            if (itr != m_mIRCLinks.end())
            {
                ChannelList channels = itr->second;
                string sMessage = "<IRC>[" + sUser + "]: Hat den IRC Channel verlassen.";

                for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
                {
                    if (itr2->uiOptions & OPTION_DISPLAY_LEAVES)
                        SendToWoWChannel(itr2->sChannel, sMessage);
                }
            }
        }    
        /*else if (sCommand == "QUIT")
        {
            // ToDo: Handle it if you want.
            // We don't get channelnames here.
        }*/
        /*else if (sCommand == "NICK")
        {
            // ToDo: Handle it if you want.
            // We don't get channelnames here.
            string sNewNick = sData.substr(sData.find(":", 1) + 1, sData.length());
        }*/
        else if (sCommand == "KICK")
        {
            string sChannel = GetSubstring(sData, FindSubstring(sData, "#", 0, 1), FindSubstring(sData, " ", FindSubstring(sData, "#", 0, 1))); // Channel without #
            string sWho = GetSubstring(sData, FindSubstring(sData, ":", 1, 1), sData.length());

            DEBUG_LOG("IRCHandler: %s kicks %s from channel %s", sUser.c_str(), sWho.c_str(), sChannel.c_str());

            if (sWho == m_sNick)
            {
                SendToIRC("JOIN #" + sChannel + "\n");
                SendToIRCChannel(sChannel, "X_x Ich _darf_ den Channel nicht verlassen.");
            }
            else
            {
                ChannelLinkMap::const_iterator itr = m_mIRCLinks.find(sChannel);

                if (itr != m_mIRCLinks.end())
                {
                    ChannelList channels = itr->second;
                    string sMessage = "<IRC>[" + sWho + "]: Wurde vom Channel gekickt von " + sUser;

                    for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
                    {
                        if (itr2->uiOptions & OPTION_DISPLAY_KICKS)
                            SendToWoWChannel(itr2->sChannel, sMessage);
                    }
                }
            }
        }
        else if (sCommand == "001")
        {
            /* Use nickserv
             */
            SendToIRC("PRIVMSG nickserv :IDENTIFY " + m_sPass + "\n");

            /* Join all defined IRC Channels
             */
            list<string> lJoinedIRCChannels;

            for (ChannelLinkMap::const_iterator itr = m_mWoWLinks.begin(); itr != m_mWoWLinks.end(); ++itr)
            {
                ChannelList channels = itr->second;

                for (ChannelList::const_iterator itr2 = channels.begin(); itr2 != channels.end(); ++itr2)
                {
                    if (find(lJoinedIRCChannels.begin(), lJoinedIRCChannels.end(), itr2->sChannel) == lJoinedIRCChannels.end())
                    {
                        SendToIRC("JOIN #" + itr2->sChannel + "\n");
                        lJoinedIRCChannels.push_back(itr2->sChannel);
                    }
                }
            }

            for (ChannelLinkMap::const_iterator itr = m_mIRCLinks.begin(); itr != m_mIRCLinks.end(); ++itr)
            {
                if (find(lJoinedIRCChannels.begin(), lJoinedIRCChannels.end(), itr->first) == lJoinedIRCChannels.end())
                {
                    SendToIRC("JOIN #" + itr->first + "\n");
                    lJoinedIRCChannels.push_back(itr->first);
                }
            }
        }
    }
}
