/*
 Copyright (c) 2026 ySpades developers.

 This file is part of ySpades.

 ySpades is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ySpades is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with ySpades.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ChatTranslator.h"
#include "ChatWindow.h"
#include <Core/Math.h>
#include <Core/Strings.h>
#include <Gui/ProxyManager.h>
#include <ZeroSpades.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <curl/curl.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace spades {
	namespace client {

		ChatTranslator& ChatTranslator::GetInstance() {
			static ChatTranslator instance;
			return instance;
		}

		ChatTranslator::ChatTranslator() : running(true) {
			InitTacticalDictionary();
			workerThread = std::thread(&ChatTranslator::WorkerLoop, this);
		}

		ChatTranslator::~ChatTranslator() {
			running = false;
			cv.notify_all();
			if (workerThread.joinable())
				workerThread.join();
		}

		void ChatTranslator::InitTacticalDictionary() {
			// Attack / Movement
			tacticalDict["rush"] = "al asalto";
			tacticalDict["rush bridge"] = "ataquen el puente";
			tacticalDict["rush base"] = "ataquen la base";
			tacticalDict["rush tent"] = "ataquen la tienda";
			tacticalDict["rush intel"] = "vayan por la intel";
			tacticalDict["push"] = "avancen / presionen";
			tacticalDict["push bridge"] = "avancen por el puente";
			tacticalDict["fall back"] = "retrocedan";
			tacticalDict["fallback"] = "retrocedan";
			tacticalDict["retreat"] = "retírense";
			tacticalDict["attack"] = "ataquen";
			tacticalDict["attack base"] = "ataquen la base";
			tacticalDict["defend"] = "defiendan";
			tacticalDict["defend base"] = "defiendan la base";
			tacticalDict["defend intel"] = "defiendan la intel";
			tacticalDict["cover me"] = "cúbreme";
			tacticalDict["follow me"] = "sígueme";
			tacticalDict["go go go"] = "vamos vamos";
			tacticalDict["come here"] = "vengan aquí";
			tacticalDict["wait"] = "esperen";
			tacticalDict["wait for me"] = "espérenme";
			tacticalDict["stop"] = "paren";

			// Intel
			tacticalDict["intel"] = "la intel";
			tacticalDict["drop intel"] = "suelta la intel";
			tacticalDict["take intel"] = "toma la intel";
			tacticalDict["grab intel"] = "toma la intel";
			tacticalDict["they have intel"] = "tienen la intel";
			tacticalDict["enemy has intel"] = "el enemigo tiene la intel";
			tacticalDict["we have intel"] = "tenemos la intel";
			tacticalDict["intel in base"] = "la intel está en la base";

			// Callouts & Locations
			tacticalDict["sniper"] = "francotirador";
			tacticalDict["sniper on hill"] = "francotirador en la colina";
			tacticalDict["sniper hill"] = "francotirador en la colina";
			tacticalDict["sniper tent"] = "francotirador en la tienda";
			tacticalDict["sniper bridge"] = "francotirador en el puente";
			tacticalDict["behind you"] = "detrás de ti";
			tacticalDict["behind"] = "atrás";
			tacticalDict["watch out"] = "cuidado";
			tacticalDict["careful"] = "cuidado";
			tacticalDict["help"] = "ayuda";
			tacticalDict["help me"] = "ayúdenme";
			tacticalDict["help here"] = "ayuda aquí";
			tacticalDict["hacker"] = "tramposo / hacker";
			tacticalDict["cheater"] = "tramposo / hacker";
			tacticalDict["bridge"] = "puente";
			tacticalDict["tunnel"] = "túnel";
			tacticalDict["tent"] = "tienda / carpa";
			tacticalDict["base"] = "base";
			tacticalDict["our base"] = "nuestra base";
			tacticalDict["enemy base"] = "base enemiga";
			tacticalDict["water"] = "agua";
			tacticalDict["left"] = "izquierda";
			tacticalDict["right"] = "derecha";
			tacticalDict["up"] = "arriba";
			tacticalDict["down"] = "abajo";
			tacticalDict["coming"] = "vienen";
			tacticalDict["they are coming"] = "vienen hacia aquí";
			tacticalDict["dig"] = "caven";
			tacticalDict["dig down"] = "caven hacia abajo";
			tacticalDict["build"] = "construyan";
			tacticalDict["build bridge"] = "construyan puente";

			// Common chat / courtesies
			tacticalDict["nice"] = "bien hecho";
			tacticalDict["nice shot"] = "buen tiro";
			tacticalDict["good shot"] = "buen disparo";
			tacticalDict["good job"] = "bien hecho";
			tacticalDict["well done"] = "bien jugado";
			tacticalDict["hello"] = "hola";
			tacticalDict["hi"] = "hola";
			tacticalDict["hey"] = "hola";
			tacticalDict["bye"] = "adiós";
			tacticalDict["goodbye"] = "adiós";
			tacticalDict["see you"] = "nos vemos";
			tacticalDict["why"] = "por qué";
			tacticalDict["who"] = "quién";
			tacticalDict["where"] = "dónde";
			tacticalDict["what"] = "qué";
			tacticalDict["how"] = "cómo";
			tacticalDict["yes"] = "sí";
			tacticalDict["no"] = "no";
			tacticalDict["ready"] = "listo";
			tacticalDict["good game"] = "buen juego";
			tacticalDict["revive"] = "reaparecer";
			tacticalDict["sorry"] = "perdón";
			tacticalDict["sry"] = "perdón";
			tacticalDict["my bad"] = "culpa mía";
			tacticalDict["teamkill"] = "fuego amigo";
			tacticalDict["tk"] = "fuego amigo";
		}

		bool ChatTranslator::ShouldTranslate(const std::string& msg) {
			if (msg.empty())
				return false;

			// Trim leading/trailing whitespace
			size_t start = 0;
			while (start < msg.size() && static_cast<unsigned char>(msg[start]) <= ' ')
				start++;
			size_t end = msg.size();
			while (end > start && static_cast<unsigned char>(msg[end - 1]) <= ' ')
				end--;

			if (start >= end)
				return false;

			std::string trimmed = msg.substr(start, end - start);
			if (trimmed.size() < 2 || trimmed.size() > 250)
				return false;

			char first = trimmed[0];
			// 1. Skip server commands or prefixes
			if (first == '/' || first == '!' || first == '.' || first == '@' || first == '#' || first == '$')
				return false;

			// 2. Skip URLs / web links
			std::string lower = trimmed;
			for (char& c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));

			if (lower.find("http://") != std::string::npos ||
			    lower.find("https://") != std::string::npos ||
			    lower.find("www.") != std::string::npos ||
			    lower.find("discord.gg") != std::string::npos)
				return false;

			// 3. Skip common universal gaming slang / laughs / emoticons
			static const char* kUniversalSlang[] = {
				"xd", "lol", "lmao", "rofl", "kek", "gg", "ggs", "gl", "hf", "glhf",
				"ty", "thx", "np", "f", "rip", "wtf", "omg", "noob", "ez", "pog",
				"bruh", "w", "l", "ok", "k", "pls", "plz", "yw", "idk", "idc",
				"afk", "brb", "oof", "gtg", "g2g", "gn", "gm", "dc", "wp", "gj"
			};
			for (const char* slang : kUniversalSlang) {
				if (lower == slang)
					return false;
			}

			// 4. Check if text is only punctuation, numbers, or laughs
			bool hasAlpha = false;
			bool hasLetterRepeatOnly = true;
			char firstLetter = 0;
			int letterCount = 0;

			for (size_t i = 0; i < lower.size(); i++) {
				unsigned char c = static_cast<unsigned char>(lower[i]);
				if ((c >= 'a' && c <= 'z') || c >= 128) {
					hasAlpha = true;
					letterCount++;
					if (!firstLetter) firstLetter = c;
					else if (firstLetter != c) hasLetterRepeatOnly = false;
				}
			}

			if (!hasAlpha)
				return false; // pure numbers / symbols / emoticons

			if (hasLetterRepeatOnly && letterCount >= 3)
				return false; // "aaaaaa", "hhhhhh"

			// Laughs: "jajaja...", "hahaha..."
			if (lower.find("jaja") != std::string::npos ||
			    lower.find("haha") != std::string::npos)
				return false;

			// 5. Spanish character check (á, é, í, ó, ú, ñ, ¿, ¡)
			if (msg.find("\xc3\xb1") != std::string::npos || msg.find("\xc3\x91") != std::string::npos ||
			    msg.find("\xc3\xa1") != std::string::npos || msg.find("\xc3\x81") != std::string::npos ||
			    msg.find("\xc3\xa9") != std::string::npos || msg.find("\xc3\x89") != std::string::npos ||
			    msg.find("\xc3\xad") != std::string::npos || msg.find("\xc3\x8d") != std::string::npos ||
			    msg.find("\xc3\xb3") != std::string::npos || msg.find("\xc3\x93") != std::string::npos ||
			    msg.find("\xc3\xba") != std::string::npos || msg.find("\xc3\x9a") != std::string::npos ||
			    msg.find("\xc2\xbf") != std::string::npos || msg.find("\xc2\xa1") != std::string::npos) {
				return false; // Definitely Spanish
			}

			// 6. Common Spanish words check
			static const char* kSpanishWords[] = {
				"hola", "amigo", "buena", "buenas", "vamos", "mata", "gracias",
				"dale", "pero", "para", "aqui", "todos", "nadie", "quien",
				"joder", "tio", "weon", "wey", "porfa", "jugador"
			};
			for (const char* sp : kSpanishWords) {
				size_t pos = lower.find(sp);
				if (pos != std::string::npos) {
					bool leftBoundary = (pos == 0 || !isalpha(static_cast<unsigned char>(lower[pos - 1])));
					bool rightBoundary = (pos + strlen(sp) == lower.size() || !isalpha(static_cast<unsigned char>(lower[pos + strlen(sp)])));
					if (leftBoundary && rightBoundary)
						return false; // Already Spanish
				}
			}

			return true;
		}

		bool ChatTranslator::TryFastTranslate(const std::string& msg, std::string& outTranslated) {
			// 1. Check in-memory LRU cache
			{
				std::lock_guard<std::mutex> lock(cacheMutex);
				auto it = cache.find(msg);
				if (it != cache.end()) {
					outTranslated = it->second;
					return true;
				}
			}

			// 2. Check tactical dictionary
			std::string lower = msg;
			for (char& c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
			while (!lower.empty() && (lower.back() == '!' || lower.back() == '.' || lower.back() == '?'))
				lower.pop_back();

			auto dictIt = tacticalDict.find(lower);
			if (dictIt != tacticalDict.end()) {
				outTranslated = dictIt->second;
				std::lock_guard<std::mutex> lock(cacheMutex);
				cache[msg] = outTranslated;
				return true;
			}

			return false;
		}

		void ChatTranslator::QueueTranslation(uint64_t msgId, const std::string& prefix,
		                                      const std::string& playerName, int teamId,
		                                      const std::string& state, const std::string& originalMsg) {
			TranslationJob job;
			job.msgId = msgId;
			job.prefix = prefix;
			job.playerName = playerName;
			job.teamId = teamId;
			job.state = state;
			job.originalMsg = originalMsg;

			{
				std::lock_guard<std::mutex> lock(queueMutex);
				// Limit queue size to avoid stale backlog
				if (jobQueue.size() < 20) {
					jobQueue.push(std::move(job));
					cv.notify_one();
				}
			}
		}

		std::vector<CompletedTranslation> ChatTranslator::PopCompletedTranslations() {
			std::vector<CompletedTranslation> result;
			std::lock_guard<std::mutex> lock(completedMutex);
			if (!completedList.empty()) {
				result.swap(completedList);
			}
			return result;
		}

		void ChatTranslator::ClearCache() {
			std::lock_guard<std::mutex> lock(cacheMutex);
			cache.clear();
		}

		std::string ChatTranslator::QueryGoogleTranslate(const std::string& text) {
			CURL* curl = curl_easy_init();
			if (!curl) return "";

			char* escaped = curl_easy_escape(curl, text.c_str(), static_cast<int>(text.size()));
			if (!escaped) {
				curl_easy_cleanup(curl);
				return "";
			}

			std::string url = "https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=es&dt=t&q=" + std::string(escaped);
			curl_free(escaped);

			std::string response;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_USERAGENT, PACKAGE_STRING);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, void* stream) -> size_t {
				std::string* s = static_cast<std::string*>(stream);
				s->append(static_cast<char*>(ptr), size * nmemb);
				return size * nmemb;
			});
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
			gui::ProxyManager::GetInstance().ApplyToCurl(curl);

			CURLcode res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (res != CURLE_OK || response.empty())
				return "";

			// Parse JSON: [[["translated text","original text",...
			size_t p1 = response.find("\"");
			if (p1 == std::string::npos)
				return "";

			size_t p2 = response.find("\"", p1 + 1);
			while (p2 != std::string::npos && response[p2 - 1] == '\\') {
				p2 = response.find("\"", p2 + 1);
			}
			if (p2 == std::string::npos || p2 <= p1 + 1)
				return "";

			std::string translated = response.substr(p1 + 1, p2 - p1 - 1);
			std::string clean;
			clean.reserve(translated.size());
			for (size_t i = 0; i < translated.size(); i++) {
				if (translated[i] == '\\' && i + 1 < translated.size()) {
					i++;
					if (translated[i] == 'n') clean += ' ';
					else clean += translated[i];
				} else {
					clean += translated[i];
				}
			}

			if (clean.empty() || EqualsIgnoringCase(clean, text))
				return "";

			return clean;
		}

		void ChatTranslator::WorkerLoop() {
#ifdef _WIN32
			// Lower thread priority so rendering & engine gets 100% CPU priority
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
#endif

			while (running) {
				TranslationJob job;
				{
					std::unique_lock<std::mutex> lock(queueMutex);
					cv.wait(lock, [this] { return !running || !jobQueue.empty(); });
					if (!running && jobQueue.empty())
						break;
					job = jobQueue.front();
					jobQueue.pop();
				}

				// Check cache in worker thread
				std::string translated;
				{
					std::lock_guard<std::mutex> lock(cacheMutex);
					auto it = cache.find(job.originalMsg);
					if (it != cache.end())
						translated = it->second;
				}

				if (translated.empty()) {
					translated = QueryGoogleTranslate(job.originalMsg);
					if (!translated.empty()) {
						std::lock_guard<std::mutex> lock(cacheMutex);
						if (cache.size() > 500)
							cache.clear();
						cache[job.originalMsg] = translated;
					}
				}

				if (!translated.empty()) {
					CompletedTranslation comp;
					comp.msgId = job.msgId;

					std::string fullText = job.prefix + ChatWindow::TeamColorMessage(job.playerName, job.teamId);
					if (!job.state.empty()) {
						fullText += " " + job.state;
					}
					fullText += ": [ES] " + translated;
					comp.fullTranslatedText = fullText;

					std::lock_guard<std::mutex> lock(completedMutex);
					completedList.push_back(std::move(comp));
				}
			}
		}

	} // namespace client
} // namespace spades
