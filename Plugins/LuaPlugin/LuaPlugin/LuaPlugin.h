/*
 * Copyright (C) 2007-2010 Crise, crise<at>mail.berlios.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef LUA_PLUGIN_H
#define LUA_PLUGIN_H

#include "version.h"
#include "ScriptInstance.h"

#include <Singleton.h>

#include <map>
#include <set>

using dcpp::Singleton;

using std::string;
using std::map;
using std::set;

class LuaPlugin : public ScriptInstance, public Singleton<LuaPlugin>
{
public:
	void onLoad(DCCorePtr core, bool install, Bool& loadRes);
	Bool onHubEnter(HubDataPtr hHub, CommandDataPtr cmd);
	Bool onOwnChatOut(HubDataPtr hHub, const char* message);
	Bool onHubDisconnected(HubDataPtr hHub);
	Bool onHubConnected(HubDataPtr hHub);
	Bool onHubDataIn(HubDataPtr hHub, const char* message);
	Bool onHubDataOut(HubDataPtr hHub, const char* message, Bool* bBreak);
	Bool onConnectionDataIn(ConnectionDataPtr hConn, const char* message);
	Bool onConnectionDataOut(ConnectionDataPtr hConn, const char* message);
	Bool onFormatChat(HubDataPtr hHub, StringDataPtr line, Bool* bBreak);
	Bool onTimer();

	// Event wrappers
	static Bool DCAPI chatOutEvent(dcptr_t pObject, dcptr_t pData, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onOwnChatOut((HubDataPtr)pObject, (char*)pData); }
	static Bool DCAPI hubOfflineEvent(dcptr_t pObject, dcptr_t /*pData*/, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onHubDisconnected((HubDataPtr)pObject); }
	static Bool DCAPI hubOnlineEvent(dcptr_t pObject, dcptr_t /*pData*/, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onHubConnected((HubDataPtr)pObject); }
	static Bool DCAPI netHubInEvent(dcptr_t pObject, dcptr_t pData, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onHubDataIn((HubDataPtr)pObject, (char*)pData); }
	static Bool DCAPI netHubOutEvent(dcptr_t pObject, dcptr_t pData, Bool* bBreak) { return LuaPlugin::getInstance()->onHubDataOut((HubDataPtr)pObject, (char*)pData, bBreak); }
	static Bool DCAPI netConnInEvent(dcptr_t pObject, dcptr_t pData, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onConnectionDataIn((ConnectionDataPtr)pObject, (char*)pData); }
	static Bool DCAPI netConnOutEvent(dcptr_t pObject, dcptr_t pData, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onConnectionDataOut((ConnectionDataPtr)pObject, (char*)pData); }
	static Bool DCAPI chatCmdEvent(dcptr_t pObject, dcptr_t pData, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onHubEnter((HubDataPtr)pObject, (CommandDataPtr)pData); }
	static Bool DCAPI chatDisplayEvent(dcptr_t pObject, dcptr_t pData, Bool* bBreak) { return LuaPlugin::getInstance()->onFormatChat((HubDataPtr)pObject, (StringDataPtr)pData, bBreak); }
	static Bool DCAPI timerEvent(dcptr_t /*pObject*/, dcptr_t /*pData*/, Bool* /*bBreak*/) { return LuaPlugin::getInstance()->onTimer(); }

	void setTimer(bool bState) {
		auto i = events.find(HOOK_TIMER_SECOND);
		if(bState &&  i == events.end()) {
			events[HOOK_TIMER_SECOND] = hooks->bind_hook(HOOK_TIMER_SECOND, &LuaPlugin::timerEvent, NULL);
		} else if(i != events.end()) {
			hooks->release_hook(i->second);
			i->second = NULL;
		}
	}

	HubDataPtr createHub(const char* url, const char* nick, const char* password) {
		auto hHub = hub->add_hub(url, nick, password);
		if(!hHub) hubs.insert(hHub);
		return hHub;
	}

	void destroyHub(HubDataPtr hHub) {
		auto i = hubs.find(hHub);
		if(i != hubs.end()) {
			hub->remove_hub(*i);
			hub->release(*i);
			hubs.erase(i);
		}
	}

	void sendHubCommand(HubDataPtr hHub, const string& cmd) {
		if(hHub)
			hub->send_protocol_cmd(hHub, cmd.c_str());
	}

	void injectHubCommand(HubDataPtr hHub, const string& cmd) {
		if(hHub)
			hub->emulate_protocol_cmd(hHub, cmd.c_str());
	}

	// Accessors (c<->c connections)
	void sendUdpPacket(const char* ip, uint32_t port, const string& data) {
		connection->send_udp_data(ip, port, (dcptr_t)data.c_str(), data.size());
	}

	void sendClientCommand(ConnectionDataPtr hConn, const char* cmd) {
		if(hConn)
			connection->send_protocol_cmd(hConn, cmd);
	}

	void dropClientConnection(ConnectionDataPtr hConn, bool graceless) {
		if(hConn)
			connection->terminate_conn(hConn, graceless ? True : False);
	}

private:
	friend class dcpp::Singleton<LuaPlugin>;

	LuaPlugin();
	~LuaPlugin();

	void removeChatCache(const HubDataPtr hub) {
		auto j = chatCache.find(hub->url);
		if(j != chatCache.end())
			chatCache.erase(j);
	}

	map<string, subsHandle> events;
	map<string, string> chatCache;
	set<HubDataPtr> hubs;

	DCCorePtr dcpp;
	DCHooksPtr hooks;

	DCHubPtr hub;
	DCConnectionPtr connection;

	static LuaPlugin* instance;
};

#endif // LUA_PLUGIN_H