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
#include <vector>

#include <Gui/UI/Components/IModalMenu.h>
#include <Gui/UI/Framework/UIElement.h>

namespace spades {
	namespace gui {
		class PreferenceTabButton;
		class HeadingNavIndex;
		namespace ui {
			class TextViewer;
		}
	} // namespace gui
	namespace client {
		class ClientUI;
		class ClientUIHelper;

		struct HksMenuTab {
			Handle<gui::ui::UIElement> panel;
			gui::PreferenceTabButton* button = nullptr; // weak
			Handle<gui::HeadingNavIndex> nav;
		};

		/**
		 * Mod menu with the in-game settings UI (PreferenceView style), initially empty.
		 * Located in folder 'hks' and opens when pressing [Insert].
		 */
		class HksMenu : public gui::ui::UIElement, public gui::IModalMenu {
			ClientUI* ui;           // weak
			ClientUIHelper* helper; // weak

			gui::ui::TextViewer* helpView = nullptr; // weak

			float contentsRight = 0.0F;
			float contentsHeight = 0.0F;
			float panelTop = 0.0F;
			float panelBottom = 0.0F;
			float tabLeft = 0.0F;
			float tabWidth = 0.0F;
			float tabTop = 0.0F;
			float tabRowHeight = 0.0F;

			std::vector<HksMenuTab> tabs;
			int selectedTabIndex = 0;

			void AddTab(gui::ui::UIElement* panel, const std::string& caption,
			            gui::HeadingNavIndex* nav);
			void SelectTab(int idx);

		public:
			HksMenu(ClientUI* ui);

			void HotKey(const std::string& key) override;

			// IModalMenu implementation
			bool IsActive() const override;
			void Open() override;
			void Close() override;
			bool KeyEvent(const std::string& key, bool down) override;
			void TextInputEvent(const std::string& text) override;
			bool AcceptsTextInput() const override;
			void Draw() override;
			void Render() override;
		};
	} // namespace client
} // namespace spades
