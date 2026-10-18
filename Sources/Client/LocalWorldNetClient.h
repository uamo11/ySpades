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

#include <memory>
#include <string>
#include <vector>

#include "GameConstants.h"
#include "GameMap.h"
#include "GameProperties.h"
#include "INetClient.h"
#include "Player.h"
#include "World.h"
#include <Core/FileManager.h>
#include <Core/Math.h>
#include <Core/Strings.h>

namespace spades {
	namespace client {
		class Client;

		class LocalWorldNetClient : public INetClient {
			Client* client;
			std::shared_ptr<GameProperties> properties;
			std::string mapPath;
			NetClientStatus status;
			std::string statusString;
			IntVector3 localBlockColor;
			bool initialized;

		public:
			LocalWorldNetClient(Client* c, const std::string& path);
			~LocalWorldNetClient() override = default;

			bool OpenWorld();

			NetClientStatus GetStatus() override { return status; }
			std::string GetStatusString() override { return statusString; }
			float GetMapReceivingProgress() override { return 1.0F; }
			const std::shared_ptr<GameProperties>& GetGameProperties() override { return properties; }

			void DoEvents(float dt) override;

			int GetPing() override { return 0; }
			float GetPacketLoss() override { return 0.0F; }
			float GetPacketThrottle() override { return 0.0F; }
			double GetDownlinkBps() override { return 0.0; }
			double GetUplinkBps() override { return 0.0; }

			void Disconnect() override { status = NetClientStatusNotConnected; }

			void SendJoin(int team, WeaponType, std::string name, int score) override {}
			void SendPosition(Vector3) override {}
			void SendOrientation(Vector3) override {}
			void SendPlayerInput(PlayerInput) override {}
			void SendWeaponInput(WeaponInput) override {}
			void SendHit(int targetPlayerId, HitType) override {}
			void SendGrenade(const Grenade&) override {}
			void SendTool() override {}
			void SendHeldBlockColor() override {}

			void SendBlockAction(IntVector3 v, BlockActionType type) override;
			void SendBlockLine(IntVector3 v1, IntVector3 v2) override;
			void SendChat(std::string text, bool global) override;
			void SendReload() override {}
			void SendTeamChange(int team) override {}
			void SendWeaponChange(WeaponType) override {}
			void SendTeamplayPing(Vector3, const std::string&) override {}

			bool ServerReportsDamage() const override { return false; }
		};
	} // namespace client
} // namespace spades
