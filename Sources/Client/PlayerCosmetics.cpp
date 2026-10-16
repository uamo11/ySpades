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

#include <algorithm>
#include <map>
#include <Client/IModel.h>
#include <Client/IRenderer.h>
#include <Core/Bitmap.h>
#include <Core/FileManager.h>
#include <Core/VoxelModel.h>

namespace spades {
	namespace client {
		namespace {
			static std::map<std::string, Handle<IModel>> g_capeCache;
			static std::map<std::string, Handle<IModel>> g_accCache;
			static Vector3 g_customSkinColor(0.20F, 0.50F, 0.85F);
			static bool g_customSkinLoaded = false;
		} // namespace

		Vector3 PlayerCosmetics::GetCustomSkinColor() {
			if (!g_customSkinLoaded)
				ReloadCustomSkin();
			return g_customSkinColor;
		}

		void PlayerCosmetics::ReloadCustomSkin() {
			g_customSkinLoaded = true;
			try {
				std::string path = "Skins/custom.png";
				if (!FileManager::FileExists(path))
					path = "Resources/Skins/custom.png";
				if (FileManager::FileExists(path)) {
					Handle<Bitmap> bmp = Bitmap::Load(path);
					if (bmp && bmp->GetWidth() >= 16 && bmp->GetHeight() >= 16) {
						// Sample front torso shirt center around (24, 24)
						int sampleX = std::min(24, bmp->GetWidth() - 1);
						int sampleY = std::min(24, bmp->GetHeight() - 1);
						uint32_t px = bmp->GetPixel(sampleX, sampleY);
						if ((px >> 24) > 32) {
							float r = static_cast<float>(px & 0xFF) / 255.0F;
							float g = static_cast<float>((px >> 8) & 0xFF) / 255.0F;
							float b = static_cast<float>((px >> 16) & 0xFF) / 255.0F;
							g_customSkinColor = MakeVector3(r, g, b);
							return;
						}
					}
				}
			} catch (...) {}
			g_customSkinColor = MakeVector3(0.25F, 0.65F, 0.95F);
		}

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
			else if (skinId == "custom")
				return GetCustomSkinColor();
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
					// Authentic Imperial Royal Crown
					// Sculpted with ermine fur base, gold circlet with rubies, emeralds, sapphires,
					// crimson velvet bonnet, golden arches, and apical orb with royal cross.
					int const sz = 12;
					int const h = 10;
					Handle<VoxelModel> vm = Handle<VoxelModel>::New(sz, sz, h);
					vm->SetOrigin(MakeVector3(-static_cast<float>(sz) * 0.5F, -static_cast<float>(sz) * 0.5F, -9.0F));

					// Little-endian RGB: 0xBBGGRR
					const uint32_t goldBright  = 0x1AE0FF; // Pure gold
					const uint32_t goldDeep    = 0x0DA0D0; // Shaded gold
					const uint32_t ermineWhite = 0xF5F5F5; // Ermine fur white
					const uint32_t ermineSpot  = 0x151515; // Ermine black tail dot
					const uint32_t ruby        = 0x1818EE; // Royal ruby (crimson red)
					const uint32_t sapphire    = 0xEE5018; // Royal sapphire (rich blue)
					const uint32_t emerald     = 0x20D820; // Royal emerald (vibrant green)
					const uint32_t diamond     = 0xFFFFFF; // Diamond / pearl
					const uint32_t amethyst    = 0x902090; // Royal amethyst
					const uint32_t velvetRed   = 0x241484; // Crimson velvet bonnet

					// Layer z = 9: Base ermine fur trim
					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							if (x == 0 || x == sz - 1 || y == 0 || y == sz - 1) {
								bool isSpot = ((x % 3 == 0 && y % 3 == 0) || x == 5 || x == 6 || y == 5 || y == 6);
								vm->SetSolid(x, y, 9, isSpot ? ermineSpot : ermineWhite);
							}
						}
					}

					// Layer z = 8: Golden circlet band with royal jewels
					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							if (x == 0 || x == sz - 1 || y == 0 || y == sz - 1) {
								uint32_t c = goldBright;
								if ((x == 0 && y == 0) || (x == sz - 1 && y == 0) ||
								    (x == 0 && y == sz - 1) || (x == sz - 1 && y == sz - 1)) {
									c = diamond;
								} else if ((x == 5 || x == 6) && y == 0) {
									c = ruby;
								} else if ((x == 5 || x == 6) && y == sz - 1) {
									c = sapphire;
								} else if ((y == 5 || y == 6) && x == 0) {
									c = emerald;
								} else if ((y == 5 || y == 6) && x == sz - 1) {
									c = ruby;
								} else if ((x == 2 || x == 9) && (y == 0 || y == sz - 1)) {
									c = amethyst;
								}
								vm->SetSolid(x, y, 8, c);
							}
						}
					}

					// Layer z = 7: Upper golden rim & interior velvet lining
					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							if (x == 0 || x == sz - 1 || y == 0 || y == sz - 1) {
								vm->SetSolid(x, y, 7, goldDeep);
							} else if (x >= 2 && x <= sz - 3 && y >= 2 && y <= sz - 3) {
								vm->SetSolid(x, y, 7, velvetRed);
							}
						}
					}

					// Layer z = 6: Fleur-de-lis peaks & velvet bonnet dome
					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							bool isCornerPeak = (x <= 1 && y <= 1) || (x >= sz - 2 && y <= 1) ||
							                    (x <= 1 && y >= sz - 2) || (x >= sz - 2 && y >= sz - 2);
							bool isMidPeak = ((x == 5 || x == 6) && (y == 0 || y == sz - 1)) ||
							                 ((y == 5 || y == 6) && (x == 0 || x == sz - 1));
							if (isCornerPeak || isMidPeak) {
								vm->SetSolid(x, y, 6, goldBright);
							} else if (x >= 2 && x <= sz - 3 && y >= 2 && y <= sz - 3) {
								vm->SetSolid(x, y, 6, velvetRed);
							}
						}
					}

					// Layer z = 5: Fleuron jewels, rising arches, velvet dome
					// Peaks finials
					vm->SetSolid(5, 0, 5, ruby);
					vm->SetSolid(6, 0, 5, ruby);
					vm->SetSolid(5, sz - 1, 5, sapphire);
					vm->SetSolid(6, sz - 1, 5, sapphire);
					vm->SetSolid(0, 5, 5, emerald);
					vm->SetSolid(0, 6, 5, emerald);
					vm->SetSolid(sz - 1, 5, 5, ruby);
					vm->SetSolid(sz - 1, 6, 5, ruby);
					// Arches curving inward
					vm->SetSolid(5, 1, 5, goldBright);
					vm->SetSolid(6, 1, 5, goldBright);
					vm->SetSolid(5, sz - 2, 5, goldBright);
					vm->SetSolid(6, sz - 2, 5, goldBright);
					vm->SetSolid(1, 5, 5, goldBright);
					vm->SetSolid(1, 6, 5, goldBright);
					vm->SetSolid(sz - 2, 5, 5, goldBright);
					vm->SetSolid(sz - 2, 6, 5, goldBright);
					// Velvet bonnet dome
					for (int x = 3; x <= sz - 4; ++x) {
						for (int y = 3; y <= sz - 4; ++y) {
							vm->SetSolid(x, y, 5, velvetRed);
						}
					}

					// Layer z = 4: Arches converging with pearls & bonnet apex
					vm->SetSolid(5, 2, 4, diamond);
					vm->SetSolid(6, 2, 4, goldBright);
					vm->SetSolid(5, sz - 3, 4, goldBright);
					vm->SetSolid(6, sz - 3, 4, diamond);
					vm->SetSolid(2, 5, 4, goldBright);
					vm->SetSolid(2, 6, 4, diamond);
					vm->SetSolid(sz - 3, 5, 4, diamond);
					vm->SetSolid(sz - 3, 6, 4, goldBright);

					vm->SetSolid(5, 3, 4, goldBright);
					vm->SetSolid(6, 3, 4, goldBright);
					vm->SetSolid(5, sz - 4, 4, goldBright);
					vm->SetSolid(6, sz - 4, 4, goldBright);
					vm->SetSolid(3, 5, 4, goldBright);
					vm->SetSolid(3, 6, 4, goldBright);
					vm->SetSolid(sz - 4, 5, 4, goldBright);
					vm->SetSolid(sz - 4, 6, 4, goldBright);

					// Velvet apex
					for (int x = 4; x <= sz - 5; ++x) {
						for (int y = 4; y <= sz - 5; ++y) {
							vm->SetSolid(x, y, 4, velvetRed);
						}
					}

					// Layer z = 3: Arches intersecting at summit
					for (int x = 4; x <= 7; ++x) {
						for (int y = 4; y <= 7; ++y) {
							bool isPearl = (x == 5 && y == 4) || (x == 6 && y == 7) ||
							               (x == 4 && y == 5) || (x == 7 && y == 6);
							vm->SetSolid(x, y, 3, isPearl ? diamond : goldBright);
						}
					}

					// Layer z = 2: Golden Monde (Orb / Globe)
					vm->SetSolid(5, 5, 2, goldBright);
					vm->SetSolid(6, 5, 2, emerald);
					vm->SetSolid(5, 6, 2, emerald);
					vm->SetSolid(6, 6, 2, goldBright);

					// Layer z = 1: Royal Cross Pattée horizontal crossbar
					for (int x = 4; x <= 7; ++x) {
						for (int y = 5; y <= 6; ++y) {
							bool isCenter = (x == 5 || x == 6);
							vm->SetSolid(x, y, 1, isCenter ? diamond : goldBright);
						}
					}

					// Layer z = 0: Royal Cross Pattée vertical apex
					vm->SetSolid(5, 5, 0, goldBright);
					vm->SetSolid(6, 5, 0, diamond);
					vm->SetSolid(5, 6, 0, diamond);
					vm->SetSolid(6, 6, 0, goldBright);

					Handle<IModel> model = renderer.CreateModel(*vm);
					g_accCache[accId] = model;
					return model;
				} else if (accId == "helmet") {
					// Tactical Military Combat Helmet
					int const sz = 12;
					int const h = 8;
					Handle<VoxelModel> vm = Handle<VoxelModel>::New(sz, sz, h);
					vm->SetOrigin(MakeVector3(-static_cast<float>(sz) * 0.5F, -static_cast<float>(sz) * 0.5F, -7.0F));

					uint32_t camoDark  = 0x1E3520; // Deep olive
					uint32_t camoMid   = 0x284828; // Foliage green
					uint32_t camoBrown = 0x1E2B38; // Earth brown
					uint32_t camoTrim  = 0x121A12; // Black Kevlar rim

					// Layer z = 7: Rim & straps
					for (int x = 0; x < sz; ++x) {
						for (int y = 0; y < sz; ++y) {
							if (x == 0 || x == sz - 1 || y == 0 || y == sz - 1) {
								vm->SetSolid(x, y, 7, camoTrim);
							}
						}
					}

					// Layers z = 3 to 6: Outer curved dome
					for (int z = 3; z <= 6; ++z) {
						int inset = (z <= 4) ? 2 : 1;
						for (int x = inset; x < sz - inset; ++x) {
							for (int y = inset; y < sz - inset; ++y) {
								bool isEdge = (x == inset || x == sz - 1 - inset ||
								               y == inset || y == sz - 1 - inset);
								if (isEdge || z <= 4) {
									uint32_t col = camoDark;
									if ((x + y + z) % 3 == 0)
										col = camoMid;
									else if ((x * 2 + y) % 5 == 0)
										col = camoBrown;
									vm->SetSolid(x, y, z, col);
								}
							}
						}
					}

					// Layer z = 2: Top plate
					for (int x = 4; x <= sz - 5; ++x) {
						for (int y = 4; y <= sz - 5; ++y) {
							vm->SetSolid(x, y, 2, camoDark);
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
