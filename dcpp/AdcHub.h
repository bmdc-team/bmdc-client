/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_ADC_HUB_H
#define DCPLUSPLUS_DCPP_ADC_HUB_H

#include "Client.h"
#include "AdcCommand.h"
#include "Socket.h"
#ifdef _USELUA
	#include "ScriptManager.h"
#endif
#include "QueueManager.h"
#include "HubUsersMap.h" //RSX++

namespace dcpp {
using std::unordered_map;	 


class ClientManager;
#ifdef _USELUA
class AdcHub;
struct AdcScriptInstance : public ScriptInstance {
	bool onClientMessage(AdcHub* aClient, const string& aLine);
};
#endif
class AdcHub : public Client, public CommandHandler<AdcHub>
#ifdef _USELUA
, public AdcScriptInstance
#endif
{
public:
	using Client::send;
	using Client::connect;

	virtual void connect(const OnlineUser& user, const string& token);
	void connect(const OnlineUser& user, string const& token, bool secure);

	virtual void hubMessage(const string& aMessage, bool thirdPerson = false);
	virtual void privateMessage(const OnlineUser& user, const string& aMessage, bool thirdPerson = false);
	virtual void sendUserCmd(const UserCommand& command, const StringMap& params);
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList);
	virtual void password(const string& pwd);
	virtual void info(bool alwaysSend);

	virtual size_t getUserCount() const { Lock l(cs); return users.size(); }
	virtual int64_t getAvailable() const;

	virtual string escape(string const& str) const { return AdcCommand::escape(str, false); }
	virtual void send(const AdcCommand& cmd);
	void refreshUserList(bool);

	string getMySID() { return AdcCommand::fromSID(sid); }
	void sendUserCmd(const string& aUserCmd) { send(aUserCmd); }

	static const vector<StringList>& getSearchExts();
	static StringList parseSearchExts(int flag);

private:
	friend class ClientManager;
	friend class CommandHandler<AdcHub>;
	friend class Identity;

	AdcHub(const string& aHubURL, bool secure);

	AdcHub(const AdcHub&);
	AdcHub& operator=(const AdcHub&);
	virtual ~AdcHub() throw();

	/** Map session id to OnlineUser */
	//typedef unordered_map<uint32_t, OnlineUser*> SIDMap;
	//typedef SIDMap::iterator SIDIter;

	/** Map session id to OnlineUser */ //thread with detections
	typedef unordered_map<uint32_t, OnlineUser*> ADCMap;
	typedef HubUsersMap<true, ADCMap> SIDMap;
	typedef SIDMap::const_iterator SIDIter;

	bool oldPassword;
	Socket udp;
	SIDMap users;

	StringMap lastInfoMap;
	mutable CriticalSection cs;

	string salt;
	uint32_t sid;

	std::unordered_set<uint32_t> forbiddenCommands;

	static const string CLIENT_PROTOCOL;
	static const string SECURE_CLIENT_PROTOCOL_TEST;
	static const string ADCS_FEATURE;
	static const string TCP4_FEATURE;
	static const string UDP4_FEATURE;
	static const string NAT0_FEATURE;
	static const string SEGA_FEATURE;
	static const string BASE_SUPPORT;
	static const string BAS0_SUPPORT;
	static const string TIGR_SUPPORT;
	static const string UCM0_SUPPORT;
	static const string BLO0_SUPPORT;
	static const string ZLIF_SUPPORT;

	virtual string checkNick(const string& nick);

	OnlineUser& getUser(const uint32_t aSID, const CID& aCID);
	OnlineUser* findUser(const uint32_t sid) const;
	OnlineUser* findUser(const CID& cid) const;
	void putUser(const uint32_t sid, bool disconnect);

	void clearUsers();

	void handle(AdcCommand::SUP, AdcCommand& c) throw();
	void handle(AdcCommand::SID, AdcCommand& c) throw();
	void handle(AdcCommand::MSG, AdcCommand& c) throw();
	void handle(AdcCommand::INF, AdcCommand& c) throw();
	void handle(AdcCommand::GPA, AdcCommand& c) throw();
	void handle(AdcCommand::QUI, AdcCommand& c) throw();
	void handle(AdcCommand::CTM, AdcCommand& c) throw();
	void handle(AdcCommand::RCM, AdcCommand& c) throw();
	void handle(AdcCommand::STA, AdcCommand& c) throw();
	void handle(AdcCommand::SCH, AdcCommand& c) throw();
	void handle(AdcCommand::CMD, AdcCommand& c) throw();
	void handle(AdcCommand::RES, AdcCommand& c) throw();
	void handle(AdcCommand::GET, AdcCommand& c) throw();
	void handle(AdcCommand::NAT, AdcCommand& c) throw();
	void handle(AdcCommand::RNT, AdcCommand& c) throw();
	void handle(AdcCommand::ZON, AdcCommand& c) throw();
	//void handle(AdcCommand::ZOF, AdcCommand& c) throw();

	template<typename T> void handle(T, AdcCommand&) { }

	void sendUDP(const AdcCommand& cmd) throw();
	void unknownProtocol(uint32_t target, const string& protocol, const string& token);
	bool secureAvail(uint32_t target, const string& protocol, const string& token);
	void sendSearch(AdcCommand& c);

	virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
	virtual void on(Connected) throw();
	virtual void on(Line, const string& aLine) throw();
	virtual void on(Failed, const string& aLine) throw();

	virtual void on(Second, uint64_t aTick) throw();

	void getUserList(OnlineUserList& list) const {
		Lock l(cs);
		for(SIDMap::const_iterator i = users.begin(); i != users.end(); i++) {
			if(i->first != AdcCommand::HUB_SID) {
				list.push_back(i->second);
			}
		}
	}

	string startChecking(const string& aParams) { return users.startChecking(this, aParams); }//checker
	void stopChecking()	{ users.stopCheck(); }
	void stopMyINFOCheck() { users.stopMyINFOCheck(); }
	
	static const vector<StringList> searchExts;

};

} // namespace dcpp

#endif // !defined(ADC_HUB_H)
