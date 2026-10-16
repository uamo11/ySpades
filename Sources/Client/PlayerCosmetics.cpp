/*
 Copyright (c) 2026 Fran6nd, ySpades developers.

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

#include "PlayerCosmetics.h"

#include <map>
#include <Client/IModel.h>
#include <Client/IRenderer.h>
#include <Core/VoxelModel.h>

namespace spades {
	namespace client {
		namespace {
			static std::map<std::string, Handle<IModel>> g_capeCache;
			static std::map<std::string, Handle<IModel>> g_accCache;
		} // namespace

		Vector3 PlayerCosmetics::GetSkinColor(const std::string& skinId, const Vector3& defaultColor) {
			if (skinId == "gold")
				return MakeVector3(1.0F, 0.82F, 0.15F); // VIP Gold
			else if (skinId == "green")
				return MakeVector3(0.22F, 0.45F, 0.25F); // Military Green
			else if (skinId == "red")
				return MakeVector3(0.85F, 0.20F, 0.20F); // Assault Red
			else if (skinId == "black")
				return MakeVector3(0.15F, 0.15F, 0.18F); // Tactical Shadow
			else if (skinId == "white")
				return MakeVector3(0.92F, 0.94F, 0.96F); // Arctic Snow
			return defaultColor;
		}

		Handle<IModel> PlayerCosmetics::GetCapeModel(IRenderer& renderer, const std::string& capeId) {
			if (capeId.empty() || capeId == "none")
				return Handle<IModel>();

			auto it = g_capeCache.find(capeId);
			if (it != g_capeCache.end() && it->second)
				return it->second;

			try {
				int const cw = 10;
				int const ch = 16;
				int const cd = 1;
				Handle<VoxelModel> vm = Handle<VoxelModel>::New(cw, cd, ch);
				vm->SetOrigin(MakeVector3(-static_cast<float>(cw) * 0.5F, 0.0F, 0.0F));

				// Colors stored as little-endian 0xBBGGRR
				uint32_t mainCol = 0x884422;   // Classic Blue
				uint32_t borderCol = 0xEEEEEE; // Silver / White trim
				if (capeId == "gold") {
					mainCol = 0x1AE0FF;   // Vibrant Gold
					borderCol = 0x05B0E0; // Deep Golden Rim
				} else if (capeId == "black") {
					mainCol = 0x181818;   // Deep Charcoal
					borderCol = 0x383838; // Dark Steel
				} else if (capeId == "red") {
					mainCol = 0x2020E0;   // Crimson Red
					borderCol = 0xEEEEEE; // White Border
				}

				for (int x = 0; x < cw; ++x) {
					for (int z = 0; z < ch; ++z) {
						bool isBorder = (x == 0 || x == cw - 1 || z == 0 || z == ch - 1);
						uint32_t c = isBorder ? borderCol : mainCol;
						// Gold cape insignia in center
						if (capeId == "gold" && (x == cw / 2 || x == cw / 2 - 1) && (z >= 4 && z <= 8))
							c = 0xFFFFFF;
						vm->SetSolid(x, 0, z, c);
					}
				}

				Handle<IModel> model = renderer.CreateModel(*vm);
				g_capeCache[capeId] = model;
				return model;
			} catch (...) {
				return Handle<IModel>();
			}
		}

		Handle<IModel> PlayerCosmetics::GetAccessoryModel(IRenderer& renderer, const std::string& accId) {
			if (accId.empty() || accId == "none")
				return Handle<IModel>();

			auto it = g_accCache.find(accId);
			if (it != g_accCache.end() && it->second)
				return it->second;

			try {
				if (accId == "crown") {
					int const sz = 10;
					int const h = 4;
					Handle<VoxelModel> vm = Handle<VoxelModel>::New(sz, sz, h);
					vm->SetOrigin(MakeVector3(-static_cast<float>(sz) * 0.5F, -static_cast<float>(sz) * 0.5F, 0.0F));
					uint32_t gold = 0x1AE0FF; // Gold
					uint32_t ruby = 0x2222EE; // Red Ruby Gem

					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							bool edge = (x == 0 || x == sz - 1 || y == 0 || y == sz - 1);
							if (edge) {
								vm->SetSolid(x, y, 0, gold);
								if (x == 0 || x == sz - 1 || x == sz / 2) {
									if (y == 0 || y == sz - 1 || y == sz / 2) {
										vm->SetSolid(x, y, 1, gold);
										vm->SetSolid(x, y, 2, (x == sz / 2 || y == sz / 2) ? ruby : gold);
									}
								}
							}
						}
					}

					Handle<IModel> model = renderer.CreateModel(*vm);
					g_accCache[accId] = model;
					return model;
				} else if (accId == "helmet") {
					int const sz = 10;
					int const h = 5;
					Handle<VoxelModel> vm = Handle<VoxelModel>::New(sz, sz, h);
					vm->SetOrigin(MakeVector3(-static_cast<float>(sz) * 0.5F, -static_cast<float>(sz) * 0.5F, 0.0F));
					uint32_t camoDark = 0x1E2B1E; // Dark Olive
					uint32_t camoBand = 0x121A12; // Straps / Trim

					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							for (int z = 0; z < h; ++z) {
								bool isTop = (z == h - 1);
								bool isRim = (z == 0 && (x == 0 || x == sz - 1 || y == 0 || y == sz - 1));
								bool isSide = (z > 0 && (x == 0 || x == sz - 1 || y == 0 || y == sz - 1));

								if (isTop) {
									vm->SetSolid(x, y, z, camoDark);
								} else if (isRim) {
									vm->SetSolid(x, y, z, camoBand);
								} else if (isSide) {
									vm->SetSolid(x, y, z, camoDark);
								}
							}
						}
					}

					Handle<IModel> model = renderer.CreateModel(*vm);
					g_accCache[accId] = model;
					return model;
				}
				return Handle<IModel>();
			} catch (...) {
				return Handle<IModel>();
			}
		}

		void PlayerCosmetics::ClearCache() {
			g_capeCache.clear();
			g_accCache.clear();
		}
	} // namespace client
} // namespace spades
