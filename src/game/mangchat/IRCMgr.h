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

#ifndef _IRC_MGR_H
#define _IRC_MGR_H

#include "IRCClient.h"
#include "../Player.h"
#include "Policies/Singleton.h"

enum ChannelAction
{
    ACTION_JOIN_CHANNEL = 1,
    ACTION_LEAVE_CHANNEL = 2
};

using namespace std;

typedef list<IRCClient*> ClientList;

// IRCMgr main class
class IRCMgr
{
    public:
        IRCMgr();
        ~IRCMgr();

        void Initialize();

        void HandleWoWChannelAction(string sChannel, ChannelAction action, Player* pPlayer);
        void HandleWoWChannelAction(string sChannel, string sMessage, Player* pPlayer);

    private:
        ClientList m_lClients;
};
#endif
#define sIRCMgr MaNGOS::Singleton<IRCMgr>::Instance()
