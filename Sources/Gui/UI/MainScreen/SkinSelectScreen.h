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

#include <string>
#include <vector>

#include <Gui/UI/Framework/UIElement.h>
#include <Gui/UI/Widgets/Button.h>
#include <Gui/UI/Widgets/GuiIcons.h>
#include <Gui/UI/Widgets/Label.h>

namespace spades {
	namespace client {
		class FontManager;
	}
	namespace gui {
		class MainScreenUI;
		class CosmeticOptionRow;

		/**
		 * High-performance cosmetic & skin selection view.
		 * Styled identically to the Preferences / Ajustes menu with a sleek sidebar,
		 * categories (Skins, Capes, Accessories), and real-time lobby 3D updates.
		 */
		class SkinSelectScreen : public ui::UIElement {
			ui::UIElement* owner;             // weak
			client::FontManager* fontManager; // weak
			MainScreenUI* mainScreenUI;       // weak

			int activeCategory = 0; // 0 = Skins, 1 = Capes, 2 = Accessories

			// Sidebar tabs
			ui::Button* tabSkins = nullptr;
			ui::Button* tabCapes = nullptr;
			ui::Button* tabAccessories = nullptr;

			// Sidebar actions
			ui::Button* openFolderButton = nullptr;
			ui::Button* saveButton = nullptr;
			ui::Button* cancelButton = nullptr;

			// Category containers
			ui::UIElement* skinsPanel = nullptr;
			ui::UIElement* capesPanel = nullptr;
			ui::UIElement* accPanel = nullptr;

			std::vector<CosmeticOptionRow*> skinRows;
			std::vector<CosmeticOptionRow*> capeRows;
			std::vector<CosmeticOptionRow*> accRows;

			void SwitchCategory(int index);
			void OnOpenFolder(ui::UIElement& sender);
			void OnSave(ui::UIElement& sender);
			void OnCancel(ui::UIElement& sender);

		public:
			SkinSelectScreen(ui::UIElement* owner, client::FontManager* fontManager,
			                 MainScreenUI* mainScreenUI = nullptr);

			void Run();
			void Close();

			void HotKey(const std::string& key) override;
			void Render() override;
		};
	} // namespace gui
} // namespace spades
