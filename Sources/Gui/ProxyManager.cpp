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

#include "ProxyManager.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <Core/Debug.h>

DEFINE_SPADES_SETTING(net_useProxy, "0");
DEFINE_SPADES_SETTING(net_proxyType, "socks5");
DEFINE_SPADES_SETTING(net_proxyServer, "");
DEFINE_SPADES_SETTING(net_proxyAuth, "");
DEFINE_SPADES_SETTING(net_proxyFile, "");
DEFINE_SPADES_SETTING(net_proxyIndex, "0");

namespace spades {
	namespace gui {

		namespace {
			struct CURLEasyDeleter {
				void operator()(CURL* handle) const {
					if (handle)
						curl_easy_cleanup(handle);
				}
			};

			std::string Trim(const std::string& str) {
				size_t first = str.find_first_not_of(" \t\r\n");
				if (first == std::string::npos)
					return "";
				size_t last = str.find_last_not_of(" \t\r\n");
				return str.substr(first, (last - first + 1));
			}
		} // namespace

		ProxyManager::ProxyManager() {
			int idx = static_cast<int>(net_proxyIndex);
			currentProxyIndex = (idx >= 0) ? static_cast<size_t>(idx) : 0;

			// If a proxy file was previously saved in settings, attempt to load it
			std::string savedFile = net_proxyFile.CString();
			if (!savedFile.empty()) {
				LoadProxyList(savedFile, net_proxyType.CString());
			}
		}

		ProxyManager& ProxyManager::GetInstance() {
			static ProxyManager instance;
			return instance;
		}

		bool ProxyManager::IsProxyEnabled() const {
			return static_cast<int>(net_useProxy) != 0;
		}

		void ProxyManager::SetProxyEnabled(bool enabled) {
			net_useProxy = enabled ? 1 : 0;
		}

		std::string ProxyManager::GetProxyType() const {
			std::string t = net_proxyType.CString();
			return t.empty() ? "socks5" : t;
		}

		void ProxyManager::SetProxyType(const std::string& type) {
			net_proxyType = type;
		}

		ProxyEntry ProxyManager::GetActiveProxy() const {
			ProxyEntry p;
			p.type = GetProxyType();

			std::string srv = net_proxyServer.CString();
			size_t colon = srv.find_last_of(':');
			if (colon != std::string::npos) {
				p.host = srv.substr(0, colon);
				try {
					p.port = std::stoi(srv.substr(colon + 1));
				} catch (...) {
					p.port = 0;
				}
			} else {
				p.host = srv;
				p.port = 0;
			}

			std::string auth = net_proxyAuth.CString();
			size_t authColon = auth.find(':');
			if (authColon != std::string::npos) {
				p.user = auth.substr(0, authColon);
				p.pass = auth.substr(authColon + 1);
			} else {
				p.user = auth;
				p.pass = "";
			}

			return p;
		}

		void ProxyManager::SetActiveProxy(const ProxyEntry& entry) {
			net_proxyServer = entry.GetFormattedServer();
			net_proxyAuth = entry.GetFormattedAuth();
			net_proxyType = entry.type;
			SPLog("ProxyManager: Active proxy set to %s (%s)", entry.GetFormattedServer().c_str(), entry.type.c_str());
		}

		ProxyEntry ProxyManager::ParseProxyString(const std::string& rawLine, const std::string& defaultType) {
			std::string line = Trim(rawLine);
			ProxyEntry entry;
			entry.type = defaultType;

			if (line.empty() || line[0] == '#' || (line.size() >= 2 && line[0] == '/' && line[1] == '/')) {
				return entry;
			}

			std::string lower = line;
			std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

			if (lower.rfind("socks5://", 0) == 0) {
				entry.type = "socks5";
				line = line.substr(9);
			} else if (lower.rfind("socks4://", 0) == 0) {
				entry.type = "socks4";
				line = line.substr(9);
			} else if (lower.rfind("http://", 0) == 0) {
				entry.type = "http";
				line = line.substr(7);
			} else if (lower.rfind("https://", 0) == 0) {
				entry.type = "http";
				line = line.substr(8);
			}

			size_t atPos = line.find('@');
			if (atPos != std::string::npos) {
				std::string authPart = line.substr(0, atPos);
				std::string hostPart = line.substr(atPos + 1);

				size_t userColon = authPart.find(':');
				if (userColon != std::string::npos) {
					entry.user = authPart.substr(0, userColon);
					entry.pass = authPart.substr(userColon + 1);
				} else {
					entry.user = authPart;
				}

				size_t portColon = hostPart.find(':');
				if (portColon != std::string::npos) {
					entry.host = hostPart.substr(0, portColon);
					try {
						entry.port = std::stoi(hostPart.substr(portColon + 1));
					} catch (...) {
						entry.port = 0;
					}
				} else {
					entry.host = hostPart;
				}
			} else {
				// Could be host:port or host:port:user:pass
				std::vector<std::string> parts;
				std::stringstream ss(line);
				std::string item;
				while (std::getline(ss, item, ':')) {
					parts.push_back(item);
				}

				if (parts.size() == 2) {
					entry.host = parts[0];
					try {
						entry.port = std::stoi(parts[1]);
					} catch (...) {
						entry.port = 0;
					}
				} else if (parts.size() >= 4) {
					entry.host = parts[0];
					try {
						entry.port = std::stoi(parts[1]);
					} catch (...) {
						entry.port = 0;
					}
					entry.user = parts[2];
					entry.pass = parts[3];
				} else if (!parts.empty()) {
					entry.host = parts[0];
				}
			}

			return entry;
		}

		bool ProxyManager::LoadProxyList(const std::string& filePath, const std::string& defaultType) {
			std::ifstream file(filePath);
			if (!file.is_open()) {
				SPLog("ProxyManager: Could not open proxy file '%s'", filePath.c_str());
				return false;
			}

			loadedProxies.clear();
			std::string line;
			while (std::getline(file, line)) {
				ProxyEntry entry = ParseProxyString(line, defaultType);
				if (!entry.host.empty() && entry.port > 0) {
					loadedProxies.push_back(entry);
				}
			}

			SPLog("ProxyManager: Loaded %zu proxies from '%s'", loadedProxies.size(), filePath.c_str());
			if (loadedProxies.empty())
				return false;

			net_proxyFile = filePath;
			if (currentProxyIndex >= loadedProxies.size()) {
				currentProxyIndex = 0;
			}
			net_proxyIndex = static_cast<int>(currentProxyIndex);
			return true;
		}

		bool ProxyManager::SetProxyIndex(size_t index) {
			if (index >= loadedProxies.size())
				return false;
			currentProxyIndex = index;
			net_proxyIndex = static_cast<int>(currentProxyIndex);
			SetActiveProxy(loadedProxies[currentProxyIndex]);
			return true;
		}

		bool ProxyManager::NextProxy() {
			if (loadedProxies.empty())
				return false;
			currentProxyIndex = (currentProxyIndex + 1) % loadedProxies.size();
			net_proxyIndex = static_cast<int>(currentProxyIndex);
			SetActiveProxy(loadedProxies[currentProxyIndex]);
			return true;
		}

		bool ProxyManager::PrevProxy() {
			if (loadedProxies.empty())
				return false;
			currentProxyIndex = (currentProxyIndex + loadedProxies.size() - 1) % loadedProxies.size();
			net_proxyIndex = static_cast<int>(currentProxyIndex);
			SetActiveProxy(loadedProxies[currentProxyIndex]);
			return true;
		}

		bool ProxyManager::LoadDefaultSocksFile() {
			std::vector<std::string> paths;
			std::string configured = net_proxyFile.CString();
			if (!configured.empty() && configured.find("socks") != std::string::npos) {
				paths.push_back(configured);
			}
			paths.push_back("C:\\Users\\Usuario\\Downloads\\proxyscrape_premium_socks_proxies.txt");
			const char* up = getenv("USERPROFILE");
			if (up) {
				paths.push_back(std::string(up) + "\\Downloads\\proxyscrape_premium_socks_proxies.txt");
			}

			for (const auto& path : paths) {
				if (LoadProxyList(path, "socks5")) {
					net_proxyType = "socks5";
					SetProxyIndex(0);
					SetProxyEnabled(true);
					return true;
				}
			}
			return false;
		}

		bool ProxyManager::LoadDefaultHttpFile() {
			std::vector<std::string> paths;
			std::string configured = net_proxyFile.CString();
			if (!configured.empty() && configured.find("http") != std::string::npos) {
				paths.push_back(configured);
			}
			paths.push_back("C:\\Users\\Usuario\\Downloads\\proxyscrape_premium_http_proxies.txt");
			const char* up = getenv("USERPROFILE");
			if (up) {
				paths.push_back(std::string(up) + "\\Downloads\\proxyscrape_premium_http_proxies.txt");
			}

			for (const auto& path : paths) {
				if (LoadProxyList(path, "http")) {
					net_proxyType = "http";
					SetProxyIndex(0);
					SetProxyEnabled(true);
					return true;
				}
			}
			return false;
		}

		std::string ProxyManager::TestProxy(const ProxyEntry& proxy, int timeoutSeconds) {
			if (proxy.host.empty() || proxy.port <= 0) {
				lastTestResult = "Error: Servidor proxy no especificado";
				return lastTestResult;
			}

			std::unique_ptr<CURL, CURLEasyDeleter> handle{curl_easy_init()};
			if (!handle) {
				lastTestResult = "Error: fallo al inicializar cURL";
				return lastTestResult;
			}

			std::string proxyUrl = proxy.host + ":" + std::to_string(proxy.port);
			curl_easy_setopt(handle.get(), CURLOPT_URL, "http://services.buildandshoot.com/");
			curl_easy_setopt(handle.get(), CURLOPT_PROXY, proxyUrl.c_str());

			if (proxy.type == "http") {
				curl_easy_setopt(handle.get(), CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			} else {
				curl_easy_setopt(handle.get(), CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
			}

			if (!proxy.user.empty()) {
				std::string auth = proxy.user + ":" + proxy.pass;
				curl_easy_setopt(handle.get(), CURLOPT_PROXYUSERPWD, auth.c_str());
			}

			curl_easy_setopt(handle.get(), CURLOPT_NOBODY, 1L);
			curl_easy_setopt(handle.get(), CURLOPT_CONNECTTIMEOUT, static_cast<long>(timeoutSeconds));
			curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT, static_cast<long>(timeoutSeconds));
			curl_easy_setopt(handle.get(), CURLOPT_USERAGENT, "ZeroSpades-ProxyTest");

			auto start = std::chrono::steady_clock::now();
			CURLcode res = curl_easy_perform(handle.get());
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			                   std::chrono::steady_clock::now() - start)
			                   .count();

			if (res == CURLE_OK) {
				long httpCode = 0;
				curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &httpCode);
				lastTestResult = "¡Conectado! Latencia: " + std::to_string(elapsed) +
				                 " ms (HTTP " + std::to_string(httpCode) + ")";
			} else {
				lastTestResult = "Fallo (" + std::to_string(elapsed) +
				                 " ms): " + std::string(curl_easy_strerror(res));
			}

			SPLog("ProxyManager: Test result for %s: %s", proxyUrl.c_str(), lastTestResult.c_str());
			return lastTestResult;
		}

		std::string ProxyManager::TestActiveProxy(int timeoutSeconds) {
			return TestProxy(GetActiveProxy(), timeoutSeconds);
		}

		void ProxyManager::ApplyToCurl(CURL* curlHandle) {
			if (!curlHandle)
				return;
			if (!IsProxyEnabled())
				return;

			std::string srv = net_proxyServer.CString();
			if (srv.empty())
				return;

			curl_easy_setopt(curlHandle, CURLOPT_PROXY, srv.c_str());
			std::string type = GetProxyType();
			if (type == "http") {
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			} else {
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
			}

			std::string auth = net_proxyAuth.CString();
			if (!auth.empty()) {
				curl_easy_setopt(curlHandle, CURLOPT_PROXYUSERPWD, auth.c_str());
			}
		}

	} // namespace gui
} // namespace spades
