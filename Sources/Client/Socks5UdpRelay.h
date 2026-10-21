/*
 Copyright (c) 2026 Fran6nd, ZeroSpades developers.

 This file is part of ZeroSpades, a fork of OpenSpades.

 OpenSpades is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 OpenSpades is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with OpenSpades.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <cstdint>
#include <enet/enet.h>

namespace spades {
	namespace client {

		/**
		 * Transparent UDP loopback relay that tunnels ENet UDP traffic through a
		 * SOCKS5 proxy via RFC 1928 SOCKS5 UDP ASSOCIATE.
		 */
		class Socks5UdpRelay {
			ENetSocket tcpSocket{ENET_SOCKET_NULL};
			ENetSocket clientUdpSocket{ENET_SOCKET_NULL};
			ENetSocket proxyUdpSocket{ENET_SOCKET_NULL};
			ENetAddress relayAddress{};
			ENetAddress targetServerAddress{};
			ENetAddress localBindAddress{};
			uint16_t localPort{0};
			std::thread relayThread;
			std::atomic<bool> isRunning{false};
			std::string lastError;

			bool SendAllTcp(const void* data, size_t len);
			bool RecvExactTcp(void* data, size_t len);
			void RelayLoop();

		public:
			Socks5UdpRelay();
			~Socks5UdpRelay();

			bool Start(const std::string& proxyHost, int proxyPort,
			           const std::string& proxyUser, const std::string& proxyPass,
			           const ENetAddress& targetServer, uint16_t& outLocalPort);
			void Stop();
			bool IsActive() const { return isRunning.load(); }
			const std::string& GetLastError() const { return lastError; }
		};

	} // namespace client
} // namespace spades
