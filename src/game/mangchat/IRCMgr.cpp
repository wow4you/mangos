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
#include "Database/DatabaseEnv.h"

#include "Policies/Singleton.h"
INSTANTIATE_SINGLETON_1(IRCMgr);

extern DatabaseType WorldDatabase;

IRCMgr::IRCMgr()
{
}

IRCMgr::~IRCMgr()
{
    for (ClientList::iterator itr = m_lClients.begin(); itr != m_lClients.end(); ++itr)
        delete *itr;

    m_lClients.clear();
}

void IRCMgr::Initialize()
{
    if (m_lClients.size())
        return;

    sLog.outString("%s\n%s\n%s\n%s",
        "***************************************",
        "**   MangChat Threaded IRC Client    **",
        "**    rewrite by kid10 & wow4you     **",
        "***************************************");

    QueryResult* pResult = WorldDatabase.PQuery("SELECT mangchat_id, wow_channel, wow_channel_options, irc_channel, irc_channel_options FROM mangchat_links");

    if (!pResult)
    {
        sLog.outErrorDb("mangChat: Could not load table 'mangchat_links'");
        return;
    }

    typedef map<uint32, ChannelLinkMap> LinkedChannelsByService;

    LinkedChannelsByService WoWLinkServiceMap;
    LinkedChannelsByService IRCLinkServiceMap;

    do
    {
        Field* pFields = pResult->Fetch();

        uint32 uiId = pFields[0].GetUInt32();
        string sWoWChannel = pFields[1].GetCppString();
        uint16 uiWoWChannelOptions = pFields[2].GetUInt16();
        string sIRCChannel = pFields[3].GetCppString();
        uint16 uiIRCChannelOptions = pFields[4].GetUInt16();

        if (uiWoWChannelOptions & OPTION_LINK_WITH_OTHER_CHANNEL)
            WoWLinkServiceMap[uiId][sWoWChannel].push_back(MangChatChannel(sIRCChannel, uiIRCChannelOptions));

        if (uiIRCChannelOptions & OPTION_LINK_WITH_OTHER_CHANNEL)
            IRCLinkServiceMap[uiId][sIRCChannel].push_back(MangChatChannel(sWoWChannel, uiWoWChannelOptions));
    }
    while (pResult->NextRow());

    delete pResult;

    pResult = WorldDatabase.PQuery("SELECT id, host, port, user, pass, nick, auth FROM mangchat");

    if (!pResult)
    {
        sLog.outErrorDb("mangChat: Could not load table 'mangchat'");
        return;
    }

    sLog.outString("Set up %u mangChat connections...", pResult->GetRowCount());

    do
    {
        Field* pFields = pResult->Fetch();

        uint32 uiId = pFields[0].GetUInt32();

        ChannelLinkMap WoWChannelLinks;
        LinkedChannelsByService::const_iterator itr = WoWLinkServiceMap.find(uiId);

        if (itr != WoWLinkServiceMap.end())
            WoWChannelLinks = itr->second;

        ChannelLinkMap IRCChannelLinks;
        itr = IRCLinkServiceMap.find(uiId);

        if (itr != IRCLinkServiceMap.end())
            IRCChannelLinks = itr->second;

        if (!WoWChannelLinks.size() && !IRCChannelLinks.size())
        {
            sLog.outErrorDb("mangChat: No channels linked in table 'mangchat_links' for Id '%u'", uiId);
            continue;
        }

        uint8 uiAuth = pFields[6].GetUInt8();

        if (uiAuth >= MAX_AUTH_METHODS)
        {
            sLog.outErrorDb("mangChat: Unknown auth method '%u' for Id '%u'", uiAuth, uiId);
            continue;
        }

        IRCClient* pClient = new IRCClient(pFields[1].GetCppString(), pFields[2].GetInt32(), pFields[3].GetCppString(), pFields[4].GetCppString(),
            pFields[5].GetCppString(), WoWChannelLinks, IRCChannelLinks, uiAuth);

        m_lClients.push_back(pClient);

        /* Start IRC Thread
         */
        ACE_Based::Thread irc(pClient);
        irc.setPriority(ACE_Based::High);
    }
    while (pResult->NextRow());

    delete pResult;
}
