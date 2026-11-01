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

#include "Socks5UdpRelay.h"
#include <cstring>
#include <Core/Debug.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
typedef int SOCKET;
// POSIX has no `INVALID_SOCKET`; ENet uses -1 for an invalid descriptor.
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif

namespace spades {
	namespace client {

		Socks5UdpRelay::Socks5UdpRelay()
		    : tcpSocket(ENET_SOCKET_NULL),
		      clientUdpSocket(ENET_SOCKET_NULL),
		      proxyUdpSocket(ENET_SOCKET_NULL),
		      localPort(0),
		      isRunning(false) {}

		Socks5UdpRelay::~Socks5UdpRelay() {
			Stop();
		}

		bool Socks5UdpRelay::SendAllTcp(const void* data, size_t len) {
			const char* ptr = static_cast<const char*>(data);
			size_t remaining = len;
			while (remaining > 0) {
				ENetBuffer buf;
				buf.data = const_cast<char*>(ptr);
				buf.dataLength = remaining;
				int sent = enet_socket_send(tcpSocket, nullptr, &buf, 1);
				if (sent <= 0) {
					enet_uint32 condition = ENET_SOCKET_WAIT_SEND;
					if (enet_socket_wait(tcpSocket, &condition, 3000) != 0 ||
					    !(condition & ENET_SOCKET_WAIT_SEND)) {
						return false;
					}
					sent = enet_socket_send(tcpSocket, nullptr, &buf, 1);
					if (sent <= 0)
						return false;
				}
				ptr += sent;
				remaining -= sent;
			}
			return true;
		}

		bool Socks5UdpRelay::RecvExactTcp(void* data, size_t len) {
			char* ptr = static_cast<char*>(data);
			size_t remaining = len;
			while (remaining > 0) {
				ENetBuffer buf;
				buf.data = ptr;
				buf.dataLength = remaining;
				int recvd = enet_socket_receive(tcpSocket, nullptr, &buf, 1);
				if (recvd <= 0) {
					enet_uint32 condition = ENET_SOCKET_WAIT_RECEIVE;
					if (enet_socket_wait(tcpSocket, &condition, 4000) != 0 ||
					    !(condition & ENET_SOCKET_WAIT_RECEIVE)) {
						return false;
					}
					recvd = enet_socket_receive(tcpSocket, nullptr, &buf, 1);
					if (recvd <= 0)
						return false;
				}
				ptr += recvd;
				remaining -= recvd;
			}
			return true;
		}

		bool Socks5UdpRelay::Start(const std::string& proxyHost, int proxyPort,
		                           const std::string& proxyUser, const std::string& proxyPass,
		                           const ENetAddress& targetServer, uint16_t& outLocalPort) {
			Stop();

			ENetAddress proxyAddress;
			if (enet_address_set_host(&proxyAddress, proxyHost.c_str()) != 0) {
				lastError = "Could not resolve SOCKS5 proxy host: " + proxyHost;
				SPLog("Socks5UdpRelay: %s", lastError.c_str());
				return false;
			}
			proxyAddress.port = static_cast<enet_uint16>(proxyPort);

			tcpSocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
			if (tcpSocket == ENET_SOCKET_NULL) {
				lastError = "Could not create TCP socket for SOCKS5 proxy";
				SPLog("Socks5UdpRelay: %s", lastError.c_str());
				return false;
			}

			if (enet_socket_connect(tcpSocket, &proxyAddress) != 0) {
				lastError = "Failed to connect TCP to SOCKS5 proxy at " + proxyHost + ":" + std::to_string(proxyPort);
				SPLog("Socks5UdpRelay: %s", lastError.c_str());
				enet_socket_destroy(tcpSocket);
				tcpSocket = ENET_SOCKET_NULL;
				return false;
			}

			enet_socket_set_option(tcpSocket, ENET_SOCKOPT_NODELAY, 1);
#ifdef _WIN32
			BOOL keepalive = TRUE;
			setsockopt(static_cast<SOCKET>(tcpSocket), SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepalive, sizeof(keepalive));
#else
			int keepalive = 1;
			setsockopt(tcpSocket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
#endif

			// Greeting: VER 5, 2 methods (0=No Auth, 2=User/Pass)
			uint8_t greeting[] = {0x05, 0x02, 0x00, 0x02};
			if (!SendAllTcp(greeting, sizeof(greeting))) {
				lastError = "Failed to send greeting to SOCKS5 proxy";
				Stop();
				return false;
			}

			uint8_t methodResp[2] = {0, 0};
			if (!RecvExactTcp(methodResp, 2)) {
				lastError = "Failed to receive method response from SOCKS5 proxy";
				Stop();
				return false;
			}

			if (methodResp[0] != 0x05) {
				lastError = "Invalid SOCKS5 version in proxy response";
				Stop();
				return false;
			}

			if (methodResp[1] == 0x02) { // Username/Password Auth (RFC 1929)
				std::vector<uint8_t> authReq;
				authReq.push_back(0x01);
				authReq.push_back(static_cast<uint8_t>(proxyUser.size()));
				authReq.insert(authReq.end(), proxyUser.begin(), proxyUser.end());
				authReq.push_back(static_cast<uint8_t>(proxyPass.size()));
				authReq.insert(authReq.end(), proxyPass.begin(), proxyPass.end());

				if (!SendAllTcp(authReq.data(), authReq.size())) {
					lastError = "Failed to send credentials to SOCKS5 proxy";
					Stop();
					return false;
				}

				uint8_t authResp[2] = {0, 0};
				if (!RecvExactTcp(authResp, 2) || authResp[1] != 0x00) {
					lastError = "SOCKS5 proxy authentication failed";
					Stop();
					return false;
				}
			} else if (methodResp[1] != 0x00) {
				lastError = "SOCKS5 proxy rejected authentication method (" + std::to_string(methodResp[1]) + ")";
				Stop();
				return false;
			}

			// Request UDP ASSOCIATE (CMD=3, ATYP=1, 0.0.0.0:0)
			uint8_t udpAssocReq[] = {0x05, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
			if (!SendAllTcp(udpAssocReq, sizeof(udpAssocReq))) {
				lastError = "Failed to send UDP ASSOCIATE request to SOCKS5 proxy";
				Stop();
				return false;
			}

			uint8_t respHead[4] = {0, 0, 0, 0};
			if (!RecvExactTcp(respHead, 4)) {
				lastError = "Failed to receive UDP ASSOCIATE reply from SOCKS5 proxy";
				Stop();
				return false;
			}

			if (respHead[0] != 0x05 || respHead[1] != 0x00) {
				lastError = "SOCKS5 proxy rejected UDP ASSOCIATE (reply code " + std::to_string(respHead[1]) + ")";
				Stop();
				return false;
			}

			if (respHead[3] == 0x01) { // IPv4
				uint8_t addrPort[6];
				if (!RecvExactTcp(addrPort, 6)) {
					lastError = "Failed to read relay address from SOCKS5 proxy";
					Stop();
					return false;
				}
				uint32_t rawIp;
				memcpy(&rawIp, addrPort, 4);
				if (rawIp == 0) {
					relayAddress.host = proxyAddress.host;
				} else {
					relayAddress.host = rawIp;
				}
				uint16_t netPort;
				memcpy(&netPort, addrPort + 4, 2);
				relayAddress.port = ENET_NET_TO_HOST_16(netPort);
			} else if (respHead[3] == 0x03) { // Domain
				uint8_t len = 0;
				if (!RecvExactTcp(&len, 1)) {
					Stop();
					return false;
				}
				std::vector<char> domain(len + 1, 0);
				if (!RecvExactTcp(domain.data(), len)) {
					Stop();
					return false;
				}
				uint16_t netPort;
				if (!RecvExactTcp(&netPort, 2)) {
					Stop();
					return false;
				}
				enet_address_set_host(&relayAddress, domain.data());
				relayAddress.port = ENET_NET_TO_HOST_16(netPort);
			} else {
				lastError = "Unsupported address type in SOCKS5 UDP ASSOCIATE reply";
				Stop();
				return false;
			}

			// 1. Create client-facing UDP relay socket bound to 127.0.0.1
			clientUdpSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
			if (clientUdpSocket == ENET_SOCKET_NULL) {
				lastError = "Failed to create client UDP relay socket";
				Stop();
				return false;
			}

			enet_socket_set_option(clientUdpSocket, ENET_SOCKOPT_NONBLOCK, 1);
			enet_socket_set_option(clientUdpSocket, ENET_SOCKOPT_RCVBUF, 1024 * 1024);
			enet_socket_set_option(clientUdpSocket, ENET_SOCKOPT_SNDBUF, 1024 * 1024);
			enet_address_set_host_ip(&localBindAddress, "127.0.0.1");
			localBindAddress.port = 0;

			if (enet_socket_bind(clientUdpSocket, &localBindAddress) != 0) {
				lastError = "Failed to bind client UDP socket to 127.0.0.1";
				Stop();
				return false;
			}

			ENetAddress actualAddr;
			if (enet_socket_get_address(clientUdpSocket, &actualAddr) != 0) {
				lastError = "Failed to query bound local client port";
				Stop();
				return false;
			}

			localPort = actualAddr.port;
			outLocalPort = localPort;

			// 2. Create WAN-facing UDP relay socket bound to 0.0.0.0
			proxyUdpSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
			if (proxyUdpSocket == ENET_SOCKET_NULL) {
				lastError = "Failed to create proxy WAN UDP socket";
				Stop();
				return false;
			}

			enet_socket_set_option(proxyUdpSocket, ENET_SOCKOPT_NONBLOCK, 1);
			enet_socket_set_option(proxyUdpSocket, ENET_SOCKOPT_RCVBUF, 1024 * 1024);
			enet_socket_set_option(proxyUdpSocket, ENET_SOCKOPT_SNDBUF, 1024 * 1024);
			ENetAddress anyAddr;
			anyAddr.host = ENET_HOST_ANY;
			anyAddr.port = 0;

			if (enet_socket_bind(proxyUdpSocket, &anyAddr) != 0) {
				lastError = "Failed to bind proxy WAN UDP socket";
				Stop();
				return false;
			}

			targetServerAddress = targetServer;

			isRunning.store(true);
			relayThread = std::thread(&Socks5UdpRelay::RelayLoop, this);

			SPLog("Socks5UdpRelay: Active, client on 127.0.0.1:%u relaying through proxy relay %s:%u",
			      (unsigned int)localPort, proxyHost.c_str(), (unsigned int)relayAddress.port);
			return true;
		}

		void Socks5UdpRelay::Stop() {
			isRunning.store(false);
			if (relayThread.joinable()) {
				relayThread.join();
			}
			if (clientUdpSocket != ENET_SOCKET_NULL) {
				enet_socket_destroy(clientUdpSocket);
				clientUdpSocket = ENET_SOCKET_NULL;
			}
			if (proxyUdpSocket != ENET_SOCKET_NULL) {
				enet_socket_destroy(proxyUdpSocket);
				proxyUdpSocket = ENET_SOCKET_NULL;
			}
			if (tcpSocket != ENET_SOCKET_NULL) {
				enet_socket_destroy(tcpSocket);
				tcpSocket = ENET_SOCKET_NULL;
			}
			localPort = 0;
		}

		void Socks5UdpRelay::RelayLoop() {
			ENetAddress enetClientAddr = {0, 0};
			bool hasClient = false;
			std::vector<uint8_t> inBuf(4096);
			std::vector<uint8_t> outBuf(4096);

			while (isRunning.load()) {
				fd_set readSet;
				FD_ZERO(&readSet);
				SOCKET sClient = static_cast<SOCKET>(clientUdpSocket);
				SOCKET sProxy = static_cast<SOCKET>(proxyUdpSocket);
				SOCKET sTcp = static_cast<SOCKET>(tcpSocket);
				SOCKET maxFd = (sClient > sProxy) ? sClient : sProxy;
				if (sTcp != INVALID_SOCKET && sTcp > maxFd) {
					maxFd = sTcp;
				}
				FD_SET(sClient, &readSet);
				FD_SET(sProxy, &readSet);
				if (sTcp != INVALID_SOCKET) {
					FD_SET(sTcp, &readSet);
				}

				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = 3000; // 3ms fast poll for ultra-low relay jitter

				int sel = select(static_cast<int>(maxFd + 1), &readSet, nullptr, nullptr, &tv);
				if (sel <= 0) {
					continue;
				}

				// Check TCP control socket status (detect if proxy server closed session)
				if (sTcp != INVALID_SOCKET && FD_ISSET(sTcp, &readSet)) {
					char probeBuf[64];
					int recvd = recv(sTcp, probeBuf, sizeof(probeBuf), 0);
					if (recvd <= 0) {
						SPLog("Socks5UdpRelay: SOCKS5 control TCP socket disconnected or EOF received (code %d)", recvd);
						break;
					}
				}

				// 1. Datagram received from local game client -> Encapsulate and send to proxy
				if (FD_ISSET(sClient, &readSet)) {
					ENetAddress fromAddr;
					ENetBuffer rxBuf;
					rxBuf.data = inBuf.data();
					rxBuf.dataLength = inBuf.size();
					int recvd = enet_socket_receive(clientUdpSocket, &fromAddr, &rxBuf, 1);
					if (recvd > 0) {
						enetClientAddr = fromAddr;
						hasClient = true;

						// Prepend 10-byte SOCKS5 UDP header:
						// [RSV(2)=0x0000, FRAG=0x00, ATYP=0x01, DST.ADDR(4), DST.PORT(2)]
						size_t needed = static_cast<size_t>(recvd + 10);
						if (outBuf.size() < needed) {
							outBuf.resize(needed);
						}
						outBuf[0] = 0x00;
						outBuf[1] = 0x00;
						outBuf[2] = 0x00;
						outBuf[3] = 0x01;
						memcpy(outBuf.data() + 4, &targetServerAddress.host, 4);
						enet_uint16 netPort = ENET_HOST_TO_NET_16(targetServerAddress.port);
						memcpy(outBuf.data() + 8, &netPort, 2);
						memcpy(outBuf.data() + 10, inBuf.data(), recvd);

						ENetBuffer txBuf;
						txBuf.data = outBuf.data();
						txBuf.dataLength = needed;
						enet_socket_send(proxyUdpSocket, &relayAddress, &txBuf, 1);
					}
				}

				// 2. Datagram received from SOCKS5 proxy relay -> Decapsulate and forward to game client
				if (FD_ISSET(sProxy, &readSet)) {
					ENetAddress fromAddr;
					ENetBuffer rxBuf;
					rxBuf.data = inBuf.data();
					rxBuf.dataLength = inBuf.size();
					int recvd = enet_socket_receive(proxyUdpSocket, &fromAddr, &rxBuf, 1);
					if (recvd > 10 && hasClient) {
						size_t hdrLen = 10;
						if (inBuf[3] == 0x03) { // Domain name in SOCKS5 header
							hdrLen = 4 + 1 + static_cast<size_t>(inBuf[4]) + 2;
						} else if (inBuf[3] == 0x04) { // IPv6 in SOCKS5 header
							hdrLen = 22;
						}
						if (static_cast<size_t>(recvd) > hdrLen) {
							ENetBuffer txBuf;
							txBuf.data = inBuf.data() + hdrLen;
							txBuf.dataLength = recvd - hdrLen;
							enet_socket_send(clientUdpSocket, &enetClientAddr, &txBuf, 1);
						}
					}
				}
			}
		}

	} // namespace client
} // namespace spades
