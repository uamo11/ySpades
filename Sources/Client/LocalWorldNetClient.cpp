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

#include "LocalWorldNetClient.h"
#include "Client.h"
#include "ChatWindow.h"
#include <algorithm>
#include <cmath>
#include <Core/Debug.h>
#include <Core/Settings.h>

SPADES_SETTING(cg_playerName);

namespace spades {
	namespace client {
		LocalWorldNetClient::LocalWorldNetClient(Client* c, const std::string& path)
		    : client(c),
		      mapPath(path),
		      status(NetClientStatusConnecting),
		      statusString("Cargando mundo local..."),
		      localBlockColor(MakeIntVector3(128, 128, 128)),
		      initialized(false) {
			properties = std::make_shared<GameProperties>(ProtocolVersion::v075);
			properties->isStaff = false;
		}

		bool LocalWorldNetClient::OpenWorld() {
			std::string path = mapPath;
			std::unique_ptr<IStream> stream{FileManager::OpenForReading(path.c_str())};
			if (!stream && path.find("Maps/") != 0 && path.find("Maps\\") != 0) {
				path = "Maps/" + mapPath;
				stream = FileManager::OpenForReading(path.c_str());
			}
			if (!stream) {
				statusString = "No se pudo abrir el mapa: " + mapPath;
				return false;
			}

			GameMap* map = GameMap::Load(stream.get());
			if (!map) {
				statusString = "Error al decodificar mapa .vxl";
				return false;
			}

			World* w = new World(properties);
			w->SetMap(map);
			map->Release();

			client->SetWorld(w);

			int pId = 0;
			w->SetLocalPlayerIndex(pId);

			// Search for suitable ground spawn position near map center
			int spawnX = 256;
			int spawnY = 256;
			int spawnZ = 32;
			for (int z = 1; z < 62; z++) {
				if (map->IsSolid(spawnX, spawnY, z)) {
					spawnZ = z - 2;
					break;
				}
			}
			if (spawnZ < 2)
				spawnZ = 20;

			Vector3 pos = MakeVector3(static_cast<float>(spawnX) + 0.5F,
			                          static_cast<float>(spawnY) + 0.5F,
			                          static_cast<float>(spawnZ));

			auto p = stmp::make_unique<Player>(*w, pId, WeaponTypeRifle, TeamGreen);
			p->SetPosition(pos);
			w->SetPlayer(pId, std::move(p));

			std::string pName = StripNewlines(cg_playerName.operator std::string());
			if (pName.empty())
				pName = "Deuce";
			w->GetPlayerPersistent(pId).name = pName;

			client->LocalPlayerCreated();

			status = NetClientStatusConnected;
			statusString = "Conectado al mundo local";
			initialized = true;
			return true;
		}

		void LocalWorldNetClient::DoEvents(float dt) {
			(void)dt;
		}

		void LocalWorldNetClient::SendBlockAction(IntVector3 v, BlockActionType type) {
			auto wOpt = client->GetWorld();
			if (!wOpt)
				return;
			World& w = *wOpt;
			GameMap* map = w.GetMap();
			if (!map)
				return;

			if (type == BlockActionDig || type == BlockActionTool) {
				w.DestroyBlock(v.x, v.y, v.z);
			} else if (type == BlockActionCreate) {
				if (v.x >= 0 && v.x < 512 && v.y >= 0 && v.y < 512 && v.z >= 0 && v.z < 64) {
					unsigned int color = 0xFF000000 | (localBlockColor.x << 16) | (localBlockColor.y << 8) | localBlockColor.z;
					map->SetColor(v.x, v.y, v.z, color);
				}
			}
		}

		void LocalWorldNetClient::SendBlockLine(IntVector3 v1, IntVector3 v2) {
			auto wOpt = client->GetWorld();
			if (!wOpt)
				return;
			World& w = *wOpt;
			GameMap* map = w.GetMap();
			if (!map)
				return;

			int dx = std::abs(v2.x - v1.x);
			int dy = std::abs(v2.y - v1.y);
			int dz = std::abs(v2.z - v1.z);
			int steps = std::max({dx, dy, dz, 1});
			for (int i = 0; i <= steps; i++) {
				float t = static_cast<float>(i) / static_cast<float>(steps);
				int x = static_cast<int>(v1.x + t * (v2.x - v1.x) + 0.5F);
				int y = static_cast<int>(v1.y + t * (v2.y - v1.y) + 0.5F);
				int z = static_cast<int>(v1.z + t * (v2.z - v1.z) + 0.5F);
				if (x >= 0 && x < 512 && y >= 0 && y < 512 && z >= 0 && z < 64) {
					unsigned int color = 0xFF000000 | (localBlockColor.x << 16) | (localBlockColor.y << 8) | localBlockColor.z;
					map->SetColor(x, y, z, color);
				}
			}
		}

		void LocalWorldNetClient::SendChat(std::string text, bool global) {
			(void)global;
			if (client->chatWindow)
				client->chatWindow->AddMessage("[Mundo Local] " + text);
		}
	} // namespace client
} // namespace spades
