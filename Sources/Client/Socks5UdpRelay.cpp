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

namespace spades {
	namespace client {

		Socks5UdpRelay::Socks5UdpRelay()
		    : tcpSocket(ENET_SOCKET_NULL),
		      localUdpSocket(ENET_SOCKET_NULL),
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

			// Create local UDP relay socket
			localUdpSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
			if (localUdpSocket == ENET_SOCKET_NULL) {
				lastError = "Failed to create local UDP relay socket";
				Stop();
				return false;
			}

			enet_socket_set_option(localUdpSocket, ENET_SOCKOPT_NONBLOCK, 1);
			enet_address_set_host_ip(&localBindAddress, "127.0.0.1");
			localBindAddress.port = 0;

			if (enet_socket_bind(localUdpSocket, &localBindAddress) != 0) {
				lastError = "Failed to bind local UDP socket to 127.0.0.1";
				Stop();
				return false;
			}

			ENetAddress actualAddr;
			if (enet_socket_get_address(localUdpSocket, &actualAddr) != 0) {
				lastError = "Failed to query bound local port";
				Stop();
				return false;
			}

			localPort = actualAddr.port;
			outLocalPort = localPort;
			targetServerAddress = targetServer;

			isRunning.store(true);
			relayThread = std::thread(&Socks5UdpRelay::RelayLoop, this);

			SPLog("Socks5UdpRelay: Active, relaying between 127.0.0.1:%u and proxy relay port %u",
			      (unsigned int)localPort, (unsigned int)relayAddress.port);
			return true;
		}

		void Socks5UdpRelay::Stop() {
			isRunning.store(false);
			if (relayThread.joinable()) {
				relayThread.join();
			}
			if (localUdpSocket != ENET_SOCKET_NULL) {
				enet_socket_destroy(localUdpSocket);
				localUdpSocket = ENET_SOCKET_NULL;
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
				enet_uint32 condition = ENET_SOCKET_WAIT_RECEIVE;
				if (enet_socket_wait(localUdpSocket, &condition, 15) == 0 &&
				    (condition & ENET_SOCKET_WAIT_RECEIVE)) {
					ENetAddress fromAddr;
					ENetBuffer rxBuf;
					rxBuf.data = inBuf.data();
					rxBuf.dataLength = inBuf.size();
					int recvd = enet_socket_receive(localUdpSocket, &fromAddr, &rxBuf, 1);
					if (recvd > 0) {
						// Datagram received from local game client (127.0.0.1)
						if (fromAddr.host == localBindAddress.host &&
						    !(fromAddr.host == relayAddress.host && fromAddr.port == relayAddress.port)) {
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
							enet_socket_send(localUdpSocket, &relayAddress, &txBuf, 1);
						}
						// Datagram received from SOCKS5 relay
						else if (recvd > 10 && fromAddr.host == relayAddress.host &&
						         fromAddr.port == relayAddress.port) {
							if (hasClient) {
								size_t hdrLen = 10;
								if (inBuf[3] == 0x03) { // Domain name in SOCKS5 header
									hdrLen = 4 + 1 + static_cast<size_t>(inBuf[4]) + 2;
								}
								if (static_cast<size_t>(recvd) > hdrLen) {
									ENetBuffer txBuf;
									txBuf.data = inBuf.data() + hdrLen;
									txBuf.dataLength = recvd - hdrLen;
									enet_socket_send(localUdpSocket, &enetClientAddr, &txBuf, 1);
								}
							}
						}
					}
				}
			}
		}

	} // namespace client
} // namespace spades
