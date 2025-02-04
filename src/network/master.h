//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name master.h - The master server headerfile. */
//
//      (c) Copyright 2003-2007 by Tom Zickel and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.

#pragma once

#include "network/netsockets.h"

struct lua_State;

// Log data used in metaserver client
struct CClientLog {
	std::string entry;     // command itself
};

// Class representing meta server client structure
class CMetaClient
{
public:
	CMetaClient() : metaSocket() {}
	~CMetaClient();
	void SetMetaServer(const std::string host, const int port);

	[[nodiscard]]
	boost::asio::awaitable<int> Init();

	void Close();

	[[nodiscard]]
	boost::asio::awaitable<int> Send(const std::string cmd);

	[[nodiscard]]
	boost::asio::awaitable<int> Recv();

	int GetLastRecvState() const { return lastRecvState; }
	int GetLogSize() const { return events.size(); }
	const CClientLog *GetLastMessage() const { return events.back().get(); }

private:
	CTCPSocket metaSocket;                     /// This is a TCP socket
	std::string metaHost;                      /// Address of metaserver
	int metaPort = -1;                         /// Port of metaserver
	std::list<std::unique_ptr<CClientLog>> events;           /// All commands received from metaserver
	int lastRecvState = -1;                    /// Now many bytes have been received in last reply
};

// Metaserver itself
extern CMetaClient MetaClient;
