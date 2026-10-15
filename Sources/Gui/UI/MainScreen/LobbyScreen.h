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

#include "CountryFlags.h"
#include <Gui/UI/Widgets/Button.h>
#include <Gui/UI/Widgets/ButtonBase.h>

namespace spades {
	namespace gui {
		class MainScreenUI;
		class CountryBadgeButton;

		/** The 3D Lobby UI element: buttons for Play, Settings, Country selector, and Quit. */
		class LobbyScreen : public ui::UIElement {
			MainScreenUI* ui; // weak
			Handle<FlagIconRenderer> flagIconRenderer;

			ui::Button* playButton = nullptr;
			ui::Button* settingsButton = nullptr;
			CountryBadgeButton* countryBadge = nullptr;
			ui::Button* quitButton = nullptr;

			void OnPlayPressed(ui::UIElement& sender);
			void OnSettingsPressed(ui::UIElement& sender);
			void OnCountryPressed(ui::UIElement& sender);
			void OnQuitPressed(ui::UIElement& sender);

		public:
			LobbyScreen(MainScreenUI* ui);

			void UpdateCountryBadge();
			void Layout();

			void HotKey(const std::string& key) override;
			void Render() override;
		};
	} // namespace gui
} // namespace spades
