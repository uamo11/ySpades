/*
 Copyright (c) 2026 Fran6nd, ZeroSpades developers.

 This file is part of ZeroSpades, a fork of OpenSpades.

 ZeroSpades is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ZeroSpades is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with ZeroSpades.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <Client/IRenderer.h>
#include <Core/Math.h>
#include <Gui/UI/Widgets/DrawUtils.h>

namespace spades {
	namespace gui {
		enum class GuiIconType {
			None,
			// Navigation / Actions
			Lobby,
			Settings,
			Quit,
			Credits,
			Skin,
			// Server Browser
			AddServer,
			Minus,
			Connect,
			Refresh,
			Search,
			Players,
			Ping,
			// Settings Tabs
			GameOptions,
			Interface,
			Graphics,
			Controls,
			Misc,
			Detailed,
			Back,
			// HKS Mod Menu Tabs
			Building,
			Flight,
			Movement,
			Visuals,
			Combat
		};

		inline void DrawGuiIcon(client::IRenderer& r, GuiIconType type, Vector2 center, Vector4 color, float s = 1.0F) {
			using ui::SetColorNP;
			SetColorNP(r, color);
			float cx = center.x;
			float cy = center.y;

			switch (type) {
			case GuiIconType::Lobby:
			case GuiIconType::Back: {
				// Arrow pointing left
				r.DrawLine(MakeVector2(cx - 5.0F * s, cy), MakeVector2(cx + 5.0F * s, cy), 2.0F * s);
				r.DrawLine(MakeVector2(cx - 5.0F * s, cy), MakeVector2(cx - 1.0F * s, cy - 4.0F * s), 2.0F * s);
				r.DrawLine(MakeVector2(cx - 5.0F * s, cy), MakeVector2(cx - 1.0F * s, cy + 4.0F * s), 2.0F * s);
				break;
			}
			case GuiIconType::Settings: {
				// Gear icon
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx + 6.0F * s, cy), 2.2F * s);
				r.DrawLine(MakeVector2(cx, cy - 6.0F * s), MakeVector2(cx, cy + 6.0F * s), 2.2F * s);
				r.DrawLine(MakeVector2(cx - 4.2F * s, cy - 4.2F * s), MakeVector2(cx + 4.2F * s, cy + 4.2F * s), 2.2F * s);
				r.DrawLine(MakeVector2(cx - 4.2F * s, cy + 4.2F * s), MakeVector2(cx + 4.2F * s, cy - 4.2F * s), 2.2F * s);
				r.DrawImage(nullptr, AABB2(cx - 3.0F * s, cy - 3.0F * s, 6.0F * s, 6.0F * s));
				break;
			}
			case GuiIconType::Quit: {
				// Cross X
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy - 4.5F * s), MakeVector2(cx + 4.5F * s, cy + 4.5F * s), 2.0F * s);
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy + 4.5F * s), MakeVector2(cx + 4.5F * s, cy - 4.5F * s), 2.0F * s);
				break;
			}
			case GuiIconType::Credits: {
				// Star / Info
				r.DrawLine(MakeVector2(cx, cy - 1.0F * s), MakeVector2(cx, cy + 5.0F * s), 1.8F * s);
				r.DrawImage(nullptr, AABB2(cx - 1.0F * s, cy - 5.0F * s, 2.0F * s, 2.0F * s));
				r.DrawLine(MakeVector2(cx - 2.0F * s, cy - 1.0F * s), MakeVector2(cx, cy - 1.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 2.5F * s, cy + 5.0F * s), MakeVector2(cx + 2.5F * s, cy + 5.0F * s), 1.5F * s);
				break;
			}
			case GuiIconType::Skin: {
				// Character / T-shirt silhouette
				r.DrawLine(MakeVector2(cx - 5.5F * s, cy - 2.5F * s), MakeVector2(cx - 2.0F * s, cy - 5.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy - 5.0F * s), MakeVector2(cx + 5.5F * s, cy - 2.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 2.0F * s, cy - 5.0F * s), MakeVector2(cx, cy - 2.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy - 5.0F * s), MakeVector2(cx, cy - 2.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 5.5F * s, cy - 2.5F * s), MakeVector2(cx - 7.0F * s, cy + 1.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 7.0F * s, cy + 1.0F * s), MakeVector2(cx - 4.5F * s, cy + 1.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 5.5F * s, cy - 2.5F * s), MakeVector2(cx + 7.0F * s, cy + 1.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 7.0F * s, cy + 1.0F * s), MakeVector2(cx + 4.5F * s, cy + 1.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy + 1.5F * s), MakeVector2(cx - 4.5F * s, cy + 6.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 4.5F * s, cy + 1.5F * s), MakeVector2(cx + 4.5F * s, cy + 6.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy + 6.0F * s), MakeVector2(cx + 4.5F * s, cy + 6.0F * s), 1.5F * s);
				break;
			}
			case GuiIconType::AddServer: {
				// Bold '+'
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx + 6.0F * s, cy), 2.2F * s);
				r.DrawLine(MakeVector2(cx, cy - 6.0F * s), MakeVector2(cx, cy + 6.0F * s), 2.2F * s);
				break;
			}
			case GuiIconType::Minus: {
				// Bold '-'
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx + 6.0F * s, cy), 2.2F * s);
				break;
			}
			case GuiIconType::Connect: {
				// Play arrow pointing right
				float leftX = cx - 3.5F * s;
				float rightX = cx + 5.0F * s;
				for (float x = leftX; x <= rightX; x += 1.0F) {
					float t = (x - leftX) / (rightX - leftX);
					float h = 5.5F * s * (1.0F - t);
					r.DrawLine(MakeVector2(x, cy - h), MakeVector2(x, cy + h), 1.2F * s);
				}
				break;
			}
			case GuiIconType::Refresh: {
				// Refresh loop
				r.DrawLine(MakeVector2(cx - 4.0F * s, cy - 4.0F * s), MakeVector2(cx + 4.0F * s, cy - 4.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 4.0F * s, cy - 4.0F * s), MakeVector2(cx + 4.0F * s, cy + 2.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 4.0F * s, cy + 4.0F * s), MakeVector2(cx - 4.0F * s, cy + 4.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 4.0F * s, cy + 4.0F * s), MakeVector2(cx - 4.0F * s, cy - 2.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 4.0F * s, cy - 4.0F * s), MakeVector2(cx + 1.5F * s, cy - 6.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 4.0F * s, cy + 4.0F * s), MakeVector2(cx - 1.5F * s, cy + 6.5F * s), 1.5F * s);
				break;
			}
			case GuiIconType::Search: {
				// Magnifying glass
				r.DrawLine(MakeVector2(cx - 4.0F * s, cy - 5.0F * s), MakeVector2(cx + 2.0F * s, cy - 5.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy - 5.0F * s), MakeVector2(cx + 2.0F * s, cy + 1.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy + 1.0F * s), MakeVector2(cx - 4.0F * s, cy + 1.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 4.0F * s, cy + 1.0F * s), MakeVector2(cx - 4.0F * s, cy - 5.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 1.5F * s, cy + 1.0F * s), MakeVector2(cx + 5.5F * s, cy + 5.0F * s), 2.0F * s);
				break;
			}
			case GuiIconType::Players: {
				// Player silhouette
				r.DrawImage(nullptr, AABB2(cx - 2.0F * s, cy - 5.5F * s, 4.0F * s, 4.0F * s));
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy + 4.0F * s), MakeVector2(cx + 4.5F * s, cy + 4.0F * s), 2.2F * s);
				r.DrawLine(MakeVector2(cx - 3.0F * s, cy + 1.5F * s), MakeVector2(cx + 3.0F * s, cy + 1.5F * s), 2.0F * s);
				break;
			}
			case GuiIconType::Ping: {
				// Signal strength bars
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy + 1.5F * s), MakeVector2(cx - 3.5F * s, cy + 4.5F * s), 1.6F * s);
				r.DrawLine(MakeVector2(cx - 0.5F * s, cy - 1.0F * s), MakeVector2(cx - 0.5F * s, cy + 4.5F * s), 1.6F * s);
				r.DrawLine(MakeVector2(cx + 2.5F * s, cy - 3.5F * s), MakeVector2(cx + 2.5F * s, cy + 4.5F * s), 1.6F * s);
				break;
			}
			case GuiIconType::GameOptions: {
				// Gamepad / Crosshairs
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx - 2.0F * s, cy), 1.6F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy), MakeVector2(cx + 6.0F * s, cy), 1.6F * s);
				r.DrawLine(MakeVector2(cx, cy - 6.0F * s), MakeVector2(cx, cy - 2.0F * s), 1.6F * s);
				r.DrawLine(MakeVector2(cx, cy + 2.0F * s), MakeVector2(cx, cy + 6.0F * s), 1.6F * s);
				r.DrawImage(nullptr, AABB2(cx - 1.2F * s, cy - 1.2F * s, 2.4F * s, 2.4F * s));
				break;
			}
			case GuiIconType::Interface: {
				// HUD / Window
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy - 5.0F * s), MakeVector2(cx + 6.0F * s, cy - 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 6.0F * s, cy - 5.0F * s), MakeVector2(cx + 6.0F * s, cy + 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 6.0F * s, cy + 5.0F * s), MakeVector2(cx - 6.0F * s, cy + 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy + 5.0F * s), MakeVector2(cx - 6.0F * s, cy - 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy - 2.0F * s), MakeVector2(cx + 6.0F * s, cy - 2.0F * s), 1.0F * s);
				r.DrawImage(nullptr, AABB2(cx - 4.0F * s, cy, 3.0F * s, 3.0F * s));
				break;
			}
			case GuiIconType::Graphics: {
				// Display monitor
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy - 5.0F * s), MakeVector2(cx + 6.0F * s, cy - 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 6.0F * s, cy - 5.0F * s), MakeVector2(cx + 6.0F * s, cy + 3.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 6.0F * s, cy + 3.0F * s), MakeVector2(cx - 6.0F * s, cy + 3.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy + 3.0F * s), MakeVector2(cx - 6.0F * s, cy - 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx, cy + 3.0F * s), MakeVector2(cx, cy + 5.5F * s), 1.6F * s);
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy + 5.5F * s), MakeVector2(cx + 3.5F * s, cy + 5.5F * s), 1.6F * s);
				break;
			}
			case GuiIconType::Controls: {
				// Mouse
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy - 5.0F * s), MakeVector2(cx + 3.5F * s, cy - 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 3.5F * s, cy - 5.0F * s), MakeVector2(cx + 3.5F * s, cy + 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 3.5F * s, cy + 5.0F * s), MakeVector2(cx - 3.5F * s, cy + 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy + 5.0F * s), MakeVector2(cx - 3.5F * s, cy - 5.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx, cy - 5.0F * s), MakeVector2(cx, cy - 1.5F * s), 1.4F * s);
				break;
			}
			case GuiIconType::Misc: {
				// Speaker
				r.DrawImage(nullptr, AABB2(cx - 5.0F * s, cy - 2.5F * s, 3.0F * s, 5.0F * s));
				r.DrawLine(MakeVector2(cx - 2.0F * s, cy - 2.5F * s), MakeVector2(cx + 1.5F * s, cy - 5.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 1.5F * s, cy - 5.5F * s), MakeVector2(cx + 1.5F * s, cy + 5.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 1.5F * s, cy + 5.5F * s), MakeVector2(cx - 2.0F * s, cy + 2.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 4.0F * s, cy - 3.0F * s), MakeVector2(cx + 4.0F * s, cy + 3.0F * s), 1.4F * s);
				break;
			}
			case GuiIconType::Detailed: {
				// Sliders / Equalizer
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy - 5.5F * s), MakeVector2(cx - 4.5F * s, cy + 5.5F * s), 1.2F * s);
				r.DrawLine(MakeVector2(cx, cy - 5.5F * s), MakeVector2(cx, cy + 5.5F * s), 1.2F * s);
				r.DrawLine(MakeVector2(cx + 4.5F * s, cy - 5.5F * s), MakeVector2(cx + 4.5F * s, cy + 5.5F * s), 1.2F * s);
				r.DrawImage(nullptr, AABB2(cx - 6.0F * s, cy - 3.5F * s, 3.0F * s, 2.5F * s));
				r.DrawImage(nullptr, AABB2(cx - 1.5F * s, cy + 1.0F * s, 3.0F * s, 2.5F * s));
				r.DrawImage(nullptr, AABB2(cx + 3.0F * s, cy - 2.0F * s, 3.0F * s, 2.5F * s));
				break;
			}
			case GuiIconType::Building: {
				// Spade / Shovel
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy + 4.5F * s), MakeVector2(cx + 2.0F * s, cy - 1.0F * s), 1.8F * s);
				// Handle grip
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy + 3.5F * s), MakeVector2(cx - 2.5F * s, cy + 5.5F * s), 1.8F * s);
				// Blade head
				r.DrawLine(MakeVector2(cx + 1.0F * s, cy - 2.0F * s), MakeVector2(cx + 5.0F * s, cy + 2.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 5.0F * s, cy + 2.0F * s), MakeVector2(cx + 6.0F * s, cy - 5.0F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 1.0F * s, cy - 2.0F * s), MakeVector2(cx + 6.0F * s, cy - 5.0F * s), 1.5F * s);
				break;
			}
			case GuiIconType::Flight: {
				// Wings / Jet
				r.DrawLine(MakeVector2(cx, cy - 5.5F * s), MakeVector2(cx, cy + 5.0F * s), 2.0F * s);
				// Swept wings
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy + 2.0F * s), MakeVector2(cx, cy - 1.0F * s), 1.8F * s);
				r.DrawLine(MakeVector2(cx + 6.0F * s, cy + 2.0F * s), MakeVector2(cx, cy - 1.0F * s), 1.8F * s);
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy + 2.0F * s), MakeVector2(cx - 1.5F * s, cy + 3.0F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 6.0F * s, cy + 2.0F * s), MakeVector2(cx + 1.5F * s, cy + 3.0F * s), 1.4F * s);
				// Tail
				r.DrawLine(MakeVector2(cx - 2.5F * s, cy + 5.0F * s), MakeVector2(cx + 2.5F * s, cy + 5.0F * s), 1.6F * s);
				break;
			}
			case GuiIconType::Movement: {
				// Swift speed arrows
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy - 3.5F * s), MakeVector2(cx + 1.0F * s, cy - 3.5F * s), 1.6F * s);
				r.DrawLine(MakeVector2(cx - 2.0F * s, cy), MakeVector2(cx + 4.5F * s, cy), 1.8F * s);
				r.DrawLine(MakeVector2(cx - 4.5F * s, cy + 3.5F * s), MakeVector2(cx + 2.0F * s, cy + 3.5F * s), 1.6F * s);
				// Arrowheads
				r.DrawLine(MakeVector2(cx + 2.5F * s, cy - 2.0F * s), MakeVector2(cx + 4.5F * s, cy), 1.6F * s);
				r.DrawLine(MakeVector2(cx + 2.5F * s, cy + 2.0F * s), MakeVector2(cx + 4.5F * s, cy), 1.6F * s);
				break;
			}
			case GuiIconType::Visuals: {
				// Eye outline
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx - 2.0F * s, cy - 3.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 2.0F * s, cy - 3.5F * s), MakeVector2(cx + 2.0F * s, cy - 3.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy - 3.5F * s), MakeVector2(cx + 6.0F * s, cy), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx - 2.0F * s, cy + 3.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx - 2.0F * s, cy + 3.5F * s), MakeVector2(cx + 2.0F * s, cy + 3.5F * s), 1.5F * s);
				r.DrawLine(MakeVector2(cx + 2.0F * s, cy + 3.5F * s), MakeVector2(cx + 6.0F * s, cy), 1.5F * s);
				// Pupil
				r.DrawImage(nullptr, AABB2(cx - 1.5F * s, cy - 1.5F * s, 3.0F * s, 3.0F * s));
				break;
			}
			case GuiIconType::Combat: {
				// Crosshair with corner brackets
				r.DrawLine(MakeVector2(cx - 6.0F * s, cy), MakeVector2(cx - 2.5F * s, cy), 1.8F * s);
				r.DrawLine(MakeVector2(cx + 2.5F * s, cy), MakeVector2(cx + 6.0F * s, cy), 1.8F * s);
				r.DrawLine(MakeVector2(cx, cy - 6.0F * s), MakeVector2(cx, cy - 2.5F * s), 1.8F * s);
				r.DrawLine(MakeVector2(cx, cy + 2.5F * s), MakeVector2(cx, cy + 6.0F * s), 1.8F * s);
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy - 3.5F * s), MakeVector2(cx - 1.5F * s, cy - 3.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy - 3.5F * s), MakeVector2(cx - 3.5F * s, cy - 1.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 3.5F * s, cy - 3.5F * s), MakeVector2(cx + 1.5F * s, cy - 3.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 3.5F * s, cy - 3.5F * s), MakeVector2(cx + 3.5F * s, cy - 1.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy + 3.5F * s), MakeVector2(cx - 1.5F * s, cy + 3.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx - 3.5F * s, cy + 3.5F * s), MakeVector2(cx - 3.5F * s, cy + 1.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 3.5F * s, cy + 3.5F * s), MakeVector2(cx + 1.5F * s, cy + 3.5F * s), 1.4F * s);
				r.DrawLine(MakeVector2(cx + 3.5F * s, cy + 3.5F * s), MakeVector2(cx + 3.5F * s, cy + 1.5F * s), 1.4F * s);
				r.DrawImage(nullptr, AABB2(cx - 0.8F * s, cy - 0.8F * s, 1.6F * s, 1.6F * s));
				break;
			}
			default:
				break;
			}
		}
	} // namespace gui
} // namespace spades
