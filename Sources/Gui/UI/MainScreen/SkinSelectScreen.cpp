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

#include "SkinSelectScreen.h"

#include <algorithm>
#include <string>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <Client/Fonts.h>
#include <Client/IFont.h>
#include <Client/IRenderer.h>
#include <Core/FileManager.h>
#include <Core/Settings.h>
#include <Core/ShellApi.h>
#include <Core/Strings.h>
#include <Gui/UI/Framework/UIManager.h>
#include <Gui/UI/Widgets/DrawUtils.h>

DEFINE_SPADES_SETTING(cg_playerSkin, "default");
DEFINE_SPADES_SETTING(cg_playerCape, "none");
DEFINE_SPADES_SETTING(cg_playerAccessory, "none");

namespace spades {
	namespace gui {
		using ui::Button;
		using ui::RadioButton;
		using ui::SetColorNP;
		using ui::UIElement;
		using ui::UIManager;

		SkinSelectScreen::SkinSelectScreen(UIElement* owner, client::FontManager* fontManager)
		    : UIElement(&owner->GetManager()), owner(owner), fontManager(fontManager) {
			UIManager* manager = &GetManager();
			SetFont(&fontManager->GetGuiFont());

			float sw = manager->screenWidth;
			float sh = manager->screenHeight;
			SetBounds(AABB2(0.0F, 0.0F, sw, sh));

			float dlgW = std::min(580.0F, sw - 32.0F);
			float dlgH = std::min(450.0F, sh - 40.0F);
			float dlgX = (sw - dlgW) * 0.5F;
			float dlgY = (sh - dlgH) * 0.5F;

			std::string currentSkin = static_cast<std::string>(cg_playerSkin);
			std::string currentCape = static_cast<std::string>(cg_playerCape);
			std::string currentAcc = static_cast<std::string>(cg_playerAccessory);

			// --- Section 1: Modelo de Skin ---
			float rowY = dlgY + 95.0F;
			float optW = (dlgW - 56.0F) / 3.0F;

			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioSkinDefault = rb.GetPointerOrNull();
				radioSkinDefault->groupName = "skinType";
				radioSkinDefault->caption = _Tr("MainScreen", "Voxel Original");
				radioSkinDefault->SetBounds(AABB2(dlgX + 24.0F, rowY, optW, 32.0F));
				if (currentSkin == "default" || currentSkin.empty())
					radioSkinDefault->Check();
				AddChild(radioSkinDefault);
			}
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioSkinMinecraft = rb.GetPointerOrNull();
				radioSkinMinecraft->groupName = "skinType";
				radioSkinMinecraft->caption = _Tr("MainScreen", "Minecraft (PNG)");
				radioSkinMinecraft->SetBounds(AABB2(dlgX + 28.0F + optW, rowY, optW, 32.0F));
				if (currentSkin == "minecraft")
					radioSkinMinecraft->Check();
				AddChild(radioSkinMinecraft);
			}
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioSkinCustom = rb.GetPointerOrNull();
				radioSkinCustom->groupName = "skinType";
				radioSkinCustom->caption = _Tr("MainScreen", "Custom 3D (KV6)");
				radioSkinCustom->SetBounds(AABB2(dlgX + 32.0F + optW * 2.0F, rowY, optW, 32.0F));
				if (currentSkin == "custom")
					radioSkinCustom->Check();
				AddChild(radioSkinCustom);
			}

			// --- Section 2: Capa Física (Cloak) ---
			rowY += 90.0F;
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioCapeNone = rb.GetPointerOrNull();
				radioCapeNone->groupName = "capeType";
				radioCapeNone->caption = _Tr("MainScreen", "Sin Capa");
				radioCapeNone->SetBounds(AABB2(dlgX + 24.0F, rowY, optW, 32.0F));
				if (currentCape == "none" || currentCape.empty())
					radioCapeNone->Check();
				AddChild(radioCapeNone);
			}
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioCapeClassic = rb.GetPointerOrNull();
				radioCapeClassic->groupName = "capeType";
				radioCapeClassic->caption = _Tr("MainScreen", "Capa Táctica");
				radioCapeClassic->SetBounds(AABB2(dlgX + 28.0F + optW, rowY, optW, 32.0F));
				if (currentCape == "classic")
					radioCapeClassic->Check();
				AddChild(radioCapeClassic);
			}
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioCapeGold = rb.GetPointerOrNull();
				radioCapeGold->groupName = "capeType";
				radioCapeGold->caption = _Tr("MainScreen", "Capa Dorada [★]");
				radioCapeGold->SetBounds(AABB2(dlgX + 32.0F + optW * 2.0F, rowY, optW, 32.0F));
				if (currentCape == "gold")
					radioCapeGold->Check();
				AddChild(radioCapeGold);
			}

			// --- Section 3: Accesorio de Cabeza ---
			rowY += 90.0F;
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioAccNone = rb.GetPointerOrNull();
				radioAccNone->groupName = "accType";
				radioAccNone->caption = _Tr("MainScreen", "Sin Accesorio");
				radioAccNone->SetBounds(AABB2(dlgX + 24.0F, rowY, optW, 32.0F));
				if (currentAcc == "none" || currentAcc.empty())
					radioAccNone->Check();
				AddChild(radioAccNone);
			}
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioAccHelmet = rb.GetPointerOrNull();
				radioAccHelmet->groupName = "accType";
				radioAccHelmet->caption = _Tr("MainScreen", "Casco Táctico");
				radioAccHelmet->SetBounds(AABB2(dlgX + 28.0F + optW, rowY, optW, 32.0F));
				if (currentAcc == "helmet")
					radioAccHelmet->Check();
				AddChild(radioAccHelmet);
			}
			{
				Handle<RadioButton> rb = Handle<RadioButton>::New(manager);
				radioAccCrown = rb.GetPointerOrNull();
				radioAccCrown->groupName = "accType";
				radioAccCrown->caption = _Tr("MainScreen", "Corona Dorada [★]");
				radioAccCrown->SetBounds(AABB2(dlgX + 32.0F + optW * 2.0F, rowY, optW, 32.0F));
				if (currentAcc == "crown")
					radioAccCrown->Check();
				AddChild(radioAccCrown);
			}

			// --- Bottom Buttons ---
			float btnBottomY = dlgY + dlgH - 52.0F;
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				openFolderButton = btn.GetPointerOrNull();
				openFolderButton->caption = _Tr("MainScreen", "Abrir Carpeta Skins");
				openFolderButton->SetBounds(AABB2(dlgX + 24.0F, btnBottomY, 175.0F, 34.0F));
				openFolderButton->activated = [this](UIElement& s) { OnOpenFolder(s); };
				AddChild(openFolderButton);
			}
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				saveButton = btn.GetPointerOrNull();
				saveButton->caption = _Tr("MainScreen", "Guardar");
				saveButton->SetBounds(AABB2(dlgX + dlgW - 228.0F, btnBottomY, 100.0F, 34.0F));
				saveButton->activated = [this](UIElement& s) { OnSave(s); };
				AddChild(saveButton);
			}
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				cancelButton = btn.GetPointerOrNull();
				cancelButton->caption = _Tr("MainScreen", "Cancelar");
				cancelButton->SetBounds(AABB2(dlgX + dlgW - 120.0F, btnBottomY, 96.0F, 34.0F));
				cancelButton->activated = [this](UIElement& s) { OnCancel(s); };
				AddChild(cancelButton);
			}
		}

		void SkinSelectScreen::OnOpenFolder(UIElement&) {
#ifdef _WIN32
			_mkdir("Skins");
			_mkdir("Skins/Capes");
#else
			mkdir("Skins", 0755);
			mkdir("Skins/Capes", 0755);
#endif
			ShowDirectoryInShell("Skins");
		}

		void SkinSelectScreen::OnSave(UIElement&) {
			if (radioSkinMinecraft && radioSkinMinecraft->toggled)
				cg_playerSkin = "minecraft";
			else if (radioSkinCustom && radioSkinCustom->toggled)
				cg_playerSkin = "custom";
			else
				cg_playerSkin = "default";

			if (radioCapeClassic && radioCapeClassic->toggled)
				cg_playerCape = "classic";
			else if (radioCapeGold && radioCapeGold->toggled)
				cg_playerCape = "gold";
			else
				cg_playerCape = "none";

			if (radioAccHelmet && radioAccHelmet->toggled)
				cg_playerAccessory = "helmet";
			else if (radioAccCrown && radioAccCrown->toggled)
				cg_playerAccessory = "crown";
			else
				cg_playerAccessory = "none";

			Settings::GetInstance()->Flush();
			Close();
		}

		void SkinSelectScreen::OnCancel(UIElement&) {
			Close();
		}

		void SkinSelectScreen::Run() {
			owner->enable = false;
			owner->GetParent()->AddChild(this);
		}

		void SkinSelectScreen::Close() {
			owner->enable = true;
			if (GetParent())
				GetParent()->RemoveChild(this);
		}

		void SkinSelectScreen::HotKey(const std::string& key) {
			if (IsEnabled() && key == "Escape") {
				OnCancel(*this);
				return;
			} else if (IsEnabled() && key == "Enter") {
				OnSave(*this);
				return;
			}
			UIElement::HotKey(key);
		}

		void SkinSelectScreen::Render() {
			client::IRenderer& r = GetManager().GetRenderer();
			float sw = GetManager().screenWidth;
			float sh = GetManager().screenHeight;

			// Dark overlay covering background
			SetColorNP(r, MakeVector4(0.02F, 0.03F, 0.05F, 0.78F));
			r.DrawImage(nullptr, AABB2(0.0F, 0.0F, sw, sh));

			float dlgW = std::min(580.0F, sw - 32.0F);
			float dlgH = std::min(450.0F, sh - 40.0F);
			float dlgX = (sw - dlgW) * 0.5F;
			float dlgY = (sh - dlgH) * 0.5F;

			// Main dialog card
			SetColorNP(r, MakeVector4(0.06F, 0.08F, 0.12F, 0.95F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY, dlgW, dlgH));

			// Border
			SetColorNP(r, MakeVector4(0.35F, 0.25F, 0.65F, 0.70F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY, dlgW, 1.5F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY + dlgH - 1.5F, dlgW, 1.5F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY, 1.5F, dlgH));
			r.DrawImage(nullptr, AABB2(dlgX + dlgW - 1.5F, dlgY, 1.5F, dlgH));

			// Top accent banner (Purple / Gold gradient line)
			SetColorNP(r, MakeVector4(0.85F, 0.50F, 1.0F, 0.95F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY, dlgW, 3.5F));

			// Header Title
			if (fontManager) {
				client::IFont& font = fontManager->GetGuiFont();
				Vector4 shadowCol = MakeVector4(0.0F, 0.0F, 0.0F, 0.8F);

				font.DrawShadow(_Tr("MainScreen", "GESTOR DE SKINS Y COSMÉTICOS"),
				                MakeVector2(dlgX + 24.0F, dlgY + 16.0F), 1.0F,
				                MakeVector4(0.95F, 0.92F, 1.0F, 1.0F), shadowCol);

				font.DrawShadow(_Tr("MainScreen", "Personalización 3D, Capas y Accesorios para la Versión Y / 1.0"),
				                MakeVector2(dlgX + 24.0F, dlgY + 38.0F), 1.0F,
				                MakeVector4(0.65F, 0.75F, 0.95F, 0.85F), shadowCol);

				// Section Header: Modelo de Skin
				font.DrawShadow(_Tr("MainScreen", "1. Modelo de Skin (3D Voxel / MC Extrusion)"),
				                MakeVector2(dlgX + 24.0F, dlgY + 74.0F), 1.0F,
				                MakeVector4(0.90F, 0.70F, 1.0F, 1.0F), shadowCol);

				// Section Header: Capa Física
				font.DrawShadow(_Tr("MainScreen", "2. Capa Física (Trailing Cloak Physics)"),
				                MakeVector2(dlgX + 24.0F, dlgY + 164.0F), 1.0F,
				                MakeVector4(0.90F, 0.70F, 1.0F, 1.0F), shadowCol);

				// Section Header: Accesorio Cabeza
				font.DrawShadow(_Tr("MainScreen", "3. Accesorio / Cosmético"),
				                MakeVector2(dlgX + 24.0F, dlgY + 254.0F), 1.0F,
				                MakeVector4(0.90F, 0.70F, 1.0F, 1.0F), shadowCol);
			}

			UIElement::Render();
		}
	} // namespace gui
} // namespace spades
