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
#include <Gui/UI/Widgets/Label.h>

namespace spades {
	namespace client {
		class FontManager;
	}
	namespace gui {
		/**
		 * Modal dialog for selecting player skins, cloaks/capes, and 3D cosmetics.
		 * Supports standard models, Minecraft PNG imports, and custom voxel models.
		 */
		class SkinSelectScreen : public ui::UIElement {
			ui::UIElement* owner;             // weak
			client::FontManager* fontManager; // weak

			// Skin selection radios
			ui::RadioButton* radioSkinDefault = nullptr;
			ui::RadioButton* radioSkinMinecraft = nullptr;
			ui::RadioButton* radioSkinCustom = nullptr;

			// Cape selection radios
			ui::RadioButton* radioCapeNone = nullptr;
			ui::RadioButton* radioCapeClassic = nullptr;
			ui::RadioButton* radioCapeGold = nullptr;

			// Accessory selection radios
			ui::RadioButton* radioAccNone = nullptr;
			ui::RadioButton* radioAccHelmet = nullptr;
			ui::RadioButton* radioAccCrown = nullptr;

			// Action buttons
			ui::Button* openFolderButton = nullptr;
			ui::Button* saveButton = nullptr;
			ui::Button* cancelButton = nullptr;

			void OnOpenFolder(ui::UIElement& sender);
			void OnSave(ui::UIElement& sender);
			void OnCancel(ui::UIElement& sender);

		public:
			SkinSelectScreen(ui::UIElement* owner, client::FontManager* fontManager);

			void Run();
			void Close();

			void HotKey(const std::string& key) override;
			void Render() override;
		};
	} // namespace gui
} // namespace spades
