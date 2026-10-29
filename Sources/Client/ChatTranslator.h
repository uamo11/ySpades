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

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cstdint>

namespace spades {
	namespace client {

		struct TranslationJob {
			uint64_t msgId;
			std::string prefix;
			std::string playerName;
			int teamId;
			std::string state;
			std::string originalMsg;
		};

		struct CompletedTranslation {
			uint64_t msgId;
			std::string fullTranslatedText;
			std::string originalMsg;
			std::string translatedMsg;
		};

		/**
		 * Ultra-lightweight asynchronous in-game chat translator.
		 * Operates strictly on a background worker thread with 0 CPU impact on the render loop.
		 * Features smart skip filtering, tactical offline dictionary, and in-memory LRU caching.
		 */
		class ChatTranslator {
		private:
			ChatTranslator();
			~ChatTranslator();

			ChatTranslator(const ChatTranslator&) = delete;
			ChatTranslator& operator=(const ChatTranslator&) = delete;

			std::mutex queueMutex;
			std::condition_variable cv;
			std::queue<TranslationJob> jobQueue;

			std::mutex completedMutex;
			std::vector<CompletedTranslation> completedList;

			std::thread workerThread;
			std::atomic<bool> running;

			// In-memory cache for ultra-fast 0ms repeated phrases
			std::mutex cacheMutex;
			std::unordered_map<std::string, std::string> cache;

			// Instant tactical gaming dictionary (<0.001ms)
			std::unordered_map<std::string, std::string> tacticalDict;

			std::atomic<bool> hasCompletedTranslations{false};

			void WorkerLoop();
			void InitTacticalDictionary();
			std::string QueryGoogleTranslate(void* curlHandle, const std::string& text);

		public:
			static ChatTranslator& GetInstance();

			// Check if any translations finished (lock-free 0-cost check for render loop)
			bool HasCompletedTranslations() const {
				return hasCompletedTranslations.load(std::memory_order_relaxed);
			}

			// Fast heuristics check to skip unnecessary translations and save CPU/network
			bool ShouldTranslate(const std::string& msg);

			// Fast-path instant translate from offline dictionary or memory cache
			bool TryFastTranslate(const std::string& msg, std::string& outTranslated);

			// Queue background translation
			void QueueTranslation(uint64_t msgId, const std::string& prefix,
			                      const std::string& playerName, int teamId,
			                      const std::string& state, const std::string& originalMsg);

			// Retrieve finished translations to update ChatWindow in-place
			std::vector<CompletedTranslation> PopCompletedTranslations();

			// Clear cache on map change or disconnect
			void ClearCache();
		};

	} // namespace client
} // namespace spades
