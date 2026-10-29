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

#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "ChatTranslator.h"
#include "ChatWindow.h"
#include <Core/Math.h>
#include <Core/Strings.h>
#include <Gui/ProxyManager.h>
#include <ZeroSpades.h>

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
			tacticalDict["push base"] = "presionen la base";
			tacticalDict["pushing"] = "avanzando";
			tacticalDict["push now"] = "empujen ahora";
			tacticalDict["fall back"] = "retrocedan";
			tacticalDict["fallback"] = "retrocedan";
			tacticalDict["retreat"] = "retírense";
			tacticalDict["attack"] = "ataquen";
			tacticalDict["attack base"] = "ataquen la base";
			tacticalDict["attack now"] = "ataquen ahora";
			tacticalDict["defend"] = "defiendan";
			tacticalDict["defend base"] = "defiendan la base";
			tacticalDict["defend intel"] = "defiendan la intel";
			tacticalDict["defend here"] = "defiendan aquí";
			tacticalDict["hold"] = "mantengan";
			tacticalDict["hold here"] = "mantengan aquí";
			tacticalDict["hold bridge"] = "mantengan el puente";
			tacticalDict["hold base"] = "mantengan la base";
			tacticalDict["cover me"] = "cúbreme";
			tacticalDict["follow me"] = "sígueme";
			tacticalDict["go go go"] = "vamos vamos";
			tacticalDict["come here"] = "vengan aquí";
			tacticalDict["come on"] = "vamos";
			tacticalDict["lets go"] = "vamos";
			tacticalDict["let's go"] = "vamos";
			tacticalDict["wait"] = "esperen";
			tacticalDict["wait for me"] = "espérenme";
			tacticalDict["stop"] = "paren";
			tacticalDict["dont stop"] = "no paren";
			tacticalDict["regroup"] = "reagrúpense";
			tacticalDict["group up"] = "júntense";

			// Intel & Objectives
			tacticalDict["intel"] = "la intel";
			tacticalDict["drop intel"] = "suelta la intel";
			tacticalDict["take intel"] = "toma la intel";
			tacticalDict["grab intel"] = "toma la intel";
			tacticalDict["get intel"] = "ve por la intel";
			tacticalDict["they have intel"] = "tienen la intel";
			tacticalDict["enemy has intel"] = "el enemigo tiene la intel";
			tacticalDict["we have intel"] = "tenemos la intel";
			tacticalDict["intel in base"] = "la intel está en la base";
			tacticalDict["intel dropped"] = "intel caída";
			tacticalDict["flag"] = "bandera / intel";
			tacticalDict["get flag"] = "ve por la bandera";
			tacticalDict["drop flag"] = "suelta la bandera";

			// Callouts & Locations
			tacticalDict["sniper"] = "francotirador";
			tacticalDict["sniper on hill"] = "francotirador en la colina";
			tacticalDict["sniper hill"] = "francotirador en la colina";
			tacticalDict["sniper tent"] = "francotirador en la tienda";
			tacticalDict["sniper bridge"] = "francotirador en el puente";
			tacticalDict["sniper tower"] = "francotirador en la torre";
			tacticalDict["enemy"] = "enemigo";
			tacticalDict["enemies"] = "enemigos";
			tacticalDict["enemy here"] = "enemigo aquí";
			tacticalDict["enemies here"] = "enemigos aquí";
			tacticalDict["behind you"] = "detrás de ti";
			tacticalDict["behind"] = "atrás";
			tacticalDict["watch out"] = "cuidado";
			tacticalDict["careful"] = "cuidado";
			tacticalDict["look out"] = "cuidado";
			tacticalDict["incoming"] = "vienen";
			tacticalDict["they are coming"] = "vienen hacia aquí";
			tacticalDict["help"] = "ayuda";
			tacticalDict["help me"] = "ayúdenme";
			tacticalDict["help here"] = "ayuda aquí";
			tacticalDict["need backup"] = "necesito apoyo";
			tacticalDict["need help"] = "necesito ayuda";
			tacticalDict["on me"] = "conmigo / aquí";
			tacticalDict["bridge"] = "puente";
			tacticalDict["tunnel"] = "túnel";
			tacticalDict["tent"] = "tienda / carpa";
			tacticalDict["base"] = "base";
			tacticalDict["our base"] = "nuestra base";
			tacticalDict["enemy base"] = "base enemiga";
			tacticalDict["spawn"] = "reaparición / base";
			tacticalDict["water"] = "agua";
			tacticalDict["hill"] = "colina";
			tacticalDict["tower"] = "torre";
			tacticalDict["left"] = "izquierda";
			tacticalDict["right"] = "derecha";
			tacticalDict["up"] = "arriba";
			tacticalDict["down"] = "abajo";
			tacticalDict["middle"] = "centro / medio";
			tacticalDict["mid"] = "medio";

			// Actions / Construction
			tacticalDict["dig"] = "caven";
			tacticalDict["dig down"] = "caven hacia abajo";
			tacticalDict["dig here"] = "caven aquí";
			tacticalDict["build"] = "construyan";
			tacticalDict["build bridge"] = "construyan puente";
			tacticalDict["build wall"] = "construyan muro";
			tacticalDict["grenade"] = "granada";
			tacticalDict["nade"] = "granada";
			tacticalDict["nades"] = "granadas";
			tacticalDict["throw nade"] = "tira granada";
			tacticalDict["reload"] = "recargando";
			tacticalDict["reloading"] = "recargando";
			tacticalDict["shoot"] = "dispara";
			tacticalDict["dont shoot"] = "no dispares";
			tacticalDict["friendly"] = "soy aliado";

			// Status & Combat state
			tacticalDict["im dead"] = "estoy muerto";
			tacticalDict["i died"] = "morí";
			tacticalDict["low hp"] = "poca vida";
			tacticalDict["he is low"] = "está tocado / poca vida";
			tacticalDict["one shot"] = "a un tiro";
			tacticalDict["camper"] = "campero";
			tacticalDict["camping"] = "campeando";
			tacticalDict["hacker"] = "tramposo / hacker";
			tacticalDict["cheater"] = "tramposo / hacker";
			tacticalDict["aimbot"] = "aimbot / trampa";
			tacticalDict["votekick"] = "voten para expulsar";
			tacticalDict["kick him"] = "expúlsenlo";
			tacticalDict["ban him"] = "baneenlo";
			tacticalDict["teamkill"] = "fuego amigo";
			tacticalDict["tk"] = "fuego amigo";

			// Courtesies & Common chat
			tacticalDict["nice"] = "bien hecho";
			tacticalDict["nice shot"] = "buen tiro";
			tacticalDict["good shot"] = "buen disparo";
			tacticalDict["nice try"] = "buen intento";
			tacticalDict["good try"] = "buen intento";
			tacticalDict["good job"] = "bien hecho";
			tacticalDict["well done"] = "bien jugado";
			tacticalDict["good game"] = "buen juego";
			tacticalDict["hello"] = "hola";
			tacticalDict["hi"] = "hola";
			tacticalDict["hey"] = "hola";
			tacticalDict["bye"] = "adiós";
			tacticalDict["goodbye"] = "adiós";
			tacticalDict["see you"] = "nos vemos";
			tacticalDict["see ya"] = "nos vemos";
			tacticalDict["sorry"] = "perdón";
			tacticalDict["sry"] = "perdón";
			tacticalDict["my bad"] = "culpa mía";
			tacticalDict["no problem"] = "sin problema";
			tacticalDict["thanks"] = "gracias";
			tacticalDict["thank you"] = "gracias";
			tacticalDict["thx"] = "gracias";
			tacticalDict["ty"] = "gracias";
			tacticalDict["please"] = "por favor";
			tacticalDict["pls"] = "por favor";
			tacticalDict["plz"] = "por favor";
			tacticalDict["ready"] = "listo";
			tacticalDict["rdy"] = "listo";
			tacticalDict["yes"] = "sí";
			tacticalDict["yeah"] = "sí";
			tacticalDict["no"] = "no";
			tacticalDict["nope"] = "no";
			tacticalDict["why"] = "por qué";
			tacticalDict["who"] = "quién";
			tacticalDict["where"] = "dónde";
			tacticalDict["what"] = "qué";
			tacticalDict["how"] = "cómo";
			tacticalDict["what happened"] = "qué pasó";
			tacticalDict["what are you doing"] = "qué haces";
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

			// 3. Skip common universal gaming slang / laughs / emoticons / acronyms
			// Check for 3+ consecutive identical characters anywhere in message (e.g. "AAAAA", "noooo", "?????")
			int consec = 1;
			for (size_t i = 1; i < lower.size(); i++) {
				if (lower[i] == lower[i - 1]) {
					consec++;
					if (consec >= 3)
						return false; // Scream / keyboard spam like "AAAAAAAAAA"
				} else {
					consec = 1;
				}
			}

			// Extract all alphabetic words
			std::vector<std::string> words;
			std::string currentWord;
			for (char c : lower) {
				if (std::isalpha(static_cast<unsigned char>(c))) {
					currentWord += c;
				} else if (!currentWord.empty()) {
					words.push_back(currentWord);
					currentWord.clear();
				}
			}
			if (!currentWord.empty())
				words.push_back(currentWord);

			if (words.empty())
				return false; // No alphabetic content

			// Check if all words are gamer slang / universal acronyms ("omg wtf", "gg wp", "wtf", "ty bro", etc.)
			static const std::unordered_set<std::string> kUniversalSlangWords = {
				"xd", "xdxd", "lol", "lmao", "rofl", "kek", "gg", "ggs", "gl", "hf", "glhf",
				"ty", "thx", "thanks", "thank", "np", "yw", "f", "rip", "wtf", "wth", "omg", "noob",
				"ez", "easy", "pog", "poggers", "bruh", "w", "l", "ok", "k", "kk", "pls", "plz",
				"idk", "idc", "imo", "imho", "afk", "brb", "oof", "gtg", "g2g", "gn", "gm", "dc", "wp", "gj",
				"lag", "ping", "fps", "ms", "hacker", "aimbot", "vxl", "aos", "admin", "mod",
				"server", "pro", "bot", "team", "bro", "dude", "man", "u", "ur", "r", "sry", "stfu",
				"hi", "hello", "hey", "yo", "bye", "cya", "bb"
			};

			bool allSlang = true;
			for (const std::string& w : words) {
				if (kUniversalSlangWords.find(w) == kUniversalSlangWords.end()) {
					allSlang = false;
					break;
				}
			}
			if (allSlang)
				return false; // Gamers understand "omg wtf", "gg", etc. perfectly

			// Check for consonant keyboard mash (e.g. "asdfgh")
			for (const std::string& w : words) {
				if (w.size() >= 5) {
					int vowels = 0;
					for (char c : w) {
						if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y')
							vowels++;
					}
					if (vowels == 0)
						return false; // Keyboard spam
				}
			}

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
			// Fast normalization: lower case and stripped trailing punctuation
			std::string lower = msg;
			for (char& c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));

			size_t s = 0;
			while (s < lower.size() && (lower[s] == ' ' || lower[s] == '\t')) s++;
			size_t e = lower.size();
			while (e > s && (lower[e - 1] == ' ' || lower[e - 1] == '\t' || lower[e - 1] == '!' || lower[e - 1] == '?' || lower[e - 1] == '.' || lower[e - 1] == ',')) e--;
			std::string key = (s < e) ? lower.substr(s, e - s) : lower;

			// 1. Check in-memory LRU cache
			{
				std::lock_guard<std::mutex> lock(cacheMutex);
				auto it = cache.find(key);
				if (it != cache.end()) {
					outTranslated = it->second;
					return true;
				}
				auto itRaw = cache.find(msg);
				if (itRaw != cache.end()) {
					outTranslated = itRaw->second;
					return true;
				}
			}

			// 2. Check tactical dictionary
			auto dictIt = tacticalDict.find(key);
			if (dictIt != tacticalDict.end()) {
				outTranslated = dictIt->second;
				std::lock_guard<std::mutex> lock(cacheMutex);
				cache[key] = outTranslated;
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
			if (!hasCompletedTranslations.load(std::memory_order_acquire)) {
				return {};
			}
			std::vector<CompletedTranslation> result;
			{
				std::lock_guard<std::mutex> lock(completedMutex);
				result.swap(completedList);
			}
			hasCompletedTranslations.store(false, std::memory_order_release);
			return result;
		}

		void ChatTranslator::ClearCache() {
			std::lock_guard<std::mutex> lock(cacheMutex);
			cache.clear();
		}

		std::string ChatTranslator::QueryGoogleTranslate(void* curlHandle, const std::string& text) {
			CURL* curl = static_cast<CURL*>(curlHandle);
			if (!curl || text.empty()) return "";

			char* escaped = curl_easy_escape(curl, text.c_str(), static_cast<int>(text.size()));
			if (!escaped) return "";

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
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
			gui::ProxyManager::GetInstance().ApplyToCurl(curl);

			CURLcode res = curl_easy_perform(curl);
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

			std::string raw = response.substr(p1 + 1, p2 - p1 - 1);
			std::string clean;
			clean.reserve(raw.size());
			for (size_t i = 0; i < raw.size(); i++) {
				if (raw[i] == '\\' && i + 1 < raw.size()) {
					i++;
					if (raw[i] == 'n' || raw[i] == 'r' || raw[i] == 't') {
						if (!clean.empty() && clean.back() != ' ') clean += ' ';
					} else {
						clean += raw[i];
					}
				} else if (raw[i] == '\r' || raw[i] == '\n' || raw[i] == '\t') {
					if (!clean.empty() && clean.back() != ' ') clean += ' ';
				} else {
					clean += raw[i];
				}
			}

			// Trim leading and trailing whitespace
			while (!clean.empty() && clean.front() == ' ') clean.erase(clean.begin());
			while (!clean.empty() && clean.back() == ' ') clean.pop_back();

			if (clean.empty() || EqualsIgnoringCase(clean, text))
				return "";

			// Keep message compact to avoid HUD clipping
			if (clean.size() > 95) {
				clean = clean.substr(0, 92) + "...";
			}

			return clean;
		}

		void ChatTranslator::WorkerLoop() {
#ifdef _WIN32
			// Lowest thread priority so game rendering never gets interrupted
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif

			CURL* workerCurl = curl_easy_init();

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

				if (translated.empty() && workerCurl) {
					translated = QueryGoogleTranslate(workerCurl, job.originalMsg);
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
					comp.originalMsg = job.originalMsg;
					comp.translatedMsg = translated;

					std::string fullText = job.prefix + ChatWindow::TeamColorMessage(job.playerName, job.teamId);
					if (!job.state.empty()) {
						fullText += " " + job.state;
					}
					std::string trPrefix;
					trPrefix += ChatWindow::MsgTranslateIcon;
					fullText += ": " + trPrefix + translated;
					comp.fullTranslatedText = fullText;

					{
						std::lock_guard<std::mutex> lock(completedMutex);
						completedList.push_back(std::move(comp));
					}
					hasCompletedTranslations.store(true, std::memory_order_release);
				}
			}

			if (workerCurl) {
				curl_easy_cleanup(workerCurl);
			}
		}

	} // namespace client
} // namespace spades
