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
#include <memory>
#include <curl/curl.h>
#include <Core/Settings.h>

namespace spades {
	namespace gui {

		struct ProxyEntry {
			std::string type; // "socks5" or "http"
			std::string host;
			int port{0};
			std::string user;
			std::string pass;

			std::string GetFormattedServer() const {
				if (port > 0)
					return host + ":" + std::to_string(port);
				return host;
			}

			std::string GetFormattedAuth() const {
				if (!user.empty())
					return user + ":" + pass;
				return "";
			}

			std::string ToString() const {
				std::string res;
				if (!user.empty()) {
					res += user + ":" + pass + "@";
				}
				res += host;
				if (port > 0) {
					res += ":" + std::to_string(port);
				}
				return res;
			}
		};

		class ProxyManager {
			std::vector<ProxyEntry> loadedProxies;
			size_t currentProxyIndex{0};
			std::string lastTestResult;

			ProxyManager();

		public:
			static ProxyManager& GetInstance();

			bool IsProxyEnabled() const;
			void SetProxyEnabled(bool enabled);

			std::string GetProxyType() const;
			void SetProxyType(const std::string& type);

			ProxyEntry GetActiveProxy() const;
			void SetActiveProxy(const ProxyEntry& entry);

			bool LoadProxyList(const std::string& filePath, const std::string& defaultType = "socks5");
			const std::vector<ProxyEntry>& GetLoadedProxies() const { return loadedProxies; }
			size_t GetCurrentProxyIndex() const { return currentProxyIndex; }
			bool SetProxyIndex(size_t index);

			bool NextProxy();
			bool PrevProxy();

			bool LoadDefaultSocksFile();
			bool LoadDefaultHttpFile();

			std::string TestProxy(const ProxyEntry& proxy, int timeoutSeconds = 4);
			std::string TestActiveProxy(int timeoutSeconds = 4);
			const std::string& GetLastTestResult() const { return lastTestResult; }

			void ApplyToCurl(CURL* curlHandle);

			static ProxyEntry ParseProxyString(const std::string& line, const std::string& defaultType = "socks5");
		};

	} // namespace gui
} // namespace spades
