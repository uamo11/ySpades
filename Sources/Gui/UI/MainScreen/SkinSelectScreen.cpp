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

		/**
		 * Sleek interactive cosmetic card option.
		 * Replaces basic desktop radio buttons with a gaming UI card featuring
		 * category accent colors, two-line descriptive layout, selection indicators,
		 * and gold elite status badges.
		 */
		class SkinOptionCard : public RadioButton {
		public:
			std::string subtitle;
			std::string tagText;
			bool isElite = false;
			Vector4 accentColor = MakeVector4(0.25F, 0.75F, 1.0F, 1.0F);

			SkinOptionCard(UIManager* manager) : RadioButton(manager) {}

			void Render() override {
				client::IRenderer& r = GetManager().GetRenderer();
				Vector2 pos = GetScreenPosition();
				Vector2 sz = size;

				// 1. Background card fill
				if (!enable) {
					SetColorNP(r, MakeVector4(0.06F, 0.07F, 0.10F, 0.40F));
				} else if (toggled) {
					if (isElite)
						SetColorNP(r, MakeVector4(0.24F, 0.18F, 0.05F, 0.95F));
					else
						SetColorNP(r, MakeVector4(0.08F, 0.18F, 0.30F, 0.95F));
				} else if (hover) {
					if (isElite)
						SetColorNP(r, MakeVector4(0.18F, 0.14F, 0.07F, 0.90F));
					else
						SetColorNP(r, MakeVector4(0.12F, 0.16F, 0.22F, 0.90F));
				} else {
					if (isElite)
						SetColorNP(r, MakeVector4(0.10F, 0.09F, 0.05F, 0.85F));
					else
						SetColorNP(r, MakeVector4(0.07F, 0.09F, 0.13F, 0.85F));
				}
				r.DrawImage(nullptr, AABB2(pos.x, pos.y, sz.x, sz.y));

				// 2. Left accent bar
				if (toggled) {
					SetColorNP(r, isElite ? MakeVector4(1.0F, 0.84F, 0.20F, 1.0F) : accentColor);
					r.DrawImage(nullptr, AABB2(pos.x, pos.y, 3.5F, sz.y));
				} else if (hover) {
					SetColorNP(r, isElite ? MakeVector4(0.85F, 0.70F, 0.20F, 0.60F) : MakeVector4(accentColor.x, accentColor.y, accentColor.z, 0.50F));
					r.DrawImage(nullptr, AABB2(pos.x, pos.y, 2.0F, sz.y));
				}

				// 3. Border outline
				if (!enable) {
					SetColorNP(r, MakeVector4(0.2F, 0.2F, 0.2F, 0.2F));
				} else if (toggled) {
					SetColorNP(r, isElite ? MakeVector4(1.0F, 0.85F, 0.25F, 0.95F) : accentColor);
				} else if (hover) {
					SetColorNP(r, isElite ? MakeVector4(0.75F, 0.60F, 0.20F, 0.65F) : MakeVector4(0.35F, 0.55F, 0.80F, 0.65F));
				} else {
					SetColorNP(r, isElite ? MakeVector4(0.40F, 0.35F, 0.15F, 0.35F) : MakeVector4(0.20F, 0.25F, 0.35F, 0.35F));
				}
				r.DrawOutlinedRect(pos.x, pos.y, pos.x + sz.x, pos.y + sz.y);

				// 4. Selection indicator
				float indY = pos.y + (sz.y - 12.0F) * 0.5F;
				float indX = pos.x + 9.0F;
				if (toggled) {
					SetColorNP(r, isElite ? MakeVector4(1.0F, 0.84F, 0.20F, 0.95F) : accentColor);
					r.DrawOutlinedRect(indX, indY, indX + 12.0F, indY + 12.0F);
					SetColorNP(r, isElite ? MakeVector4(1.0F, 0.95F, 0.50F, 1.0F) : MakeVector4(1.0F, 1.0F, 1.0F, 1.0F));
					r.DrawImage(nullptr, AABB2(indX + 3.0F, indY + 3.0F, 6.0F, 6.0F));
				} else {
					SetColorNP(r, hover ? MakeVector4(0.50F, 0.60F, 0.75F, 0.60F) : MakeVector4(0.30F, 0.35F, 0.45F, 0.50F));
					r.DrawOutlinedRect(indX, indY, indX + 12.0F, indY + 12.0F);
				}

				client::IFont* font = GetFont();
				if (!font)
					return;

				// 5. Elite badge tag (if present)
				if (!tagText.empty()) {
					float tagW = 46.0F;
					float tagH = 13.0F;
					float tagX = pos.x + sz.x - tagW - 6.0F;
					float tagY = pos.y + 5.0F;

					SetColorNP(r, isElite ? MakeVector4(0.85F, 0.65F, 0.10F, 0.35F) : MakeVector4(0.20F, 0.50F, 0.90F, 0.30F));
					r.DrawImage(nullptr, AABB2(tagX, tagY, tagW, tagH));
					SetColorNP(r, isElite ? MakeVector4(1.0F, 0.85F, 0.25F, 0.85F) : MakeVector4(0.40F, 0.75F, 1.0F, 0.80F));
					r.DrawOutlinedRect(tagX, tagY, tagX + tagW, tagY + tagH);

					font->DrawShadow(tagText, MakeVector2(tagX + 3.5F, tagY + 1.5F), 0.55F,
					                 isElite ? MakeVector4(1.0F, 0.92F, 0.40F, 1.0F) : MakeVector4(0.60F, 0.85F, 1.0F, 1.0F),
					                 MakeVector4(0.0F, 0.0F, 0.0F, 0.6F));
				}

				// 6. Title (Line 1)
				Vector4 titleColor;
				if (!enable)
					titleColor = MakeVector4(0.5F, 0.5F, 0.5F, 0.4F);
				else if (isElite)
					titleColor = toggled ? MakeVector4(1.0F, 0.92F, 0.40F, 1.0F) : MakeVector4(0.95F, 0.82F, 0.35F, 0.9F);
				else if (toggled)
					titleColor = MakeVector4(1.0F, 1.0F, 1.0F, 1.0F);
				else if (hover)
					titleColor = MakeVector4(0.92F, 0.96F, 1.0F, 0.95F);
				else
					titleColor = MakeVector4(0.82F, 0.86F, 0.92F, 0.85F);

				font->DrawShadow(caption, MakeVector2(pos.x + 27.0F, pos.y + 6.0F), 0.80F,
				                 titleColor, MakeVector4(0.0F, 0.0F, 0.0F, 0.5F));

				// 7. Subtitle (Line 2)
				if (!subtitle.empty()) {
					Vector4 subColor;
					if (!enable)
						subColor = MakeVector4(0.4F, 0.4F, 0.4F, 0.3F);
					else if (isElite)
						subColor = MakeVector4(0.80F, 0.70F, 0.40F, 0.75F);
					else if (toggled)
						subColor = MakeVector4(0.50F, 0.75F, 0.95F, 0.85F);
					else
						subColor = MakeVector4(0.45F, 0.50F, 0.60F, 0.70F);

					font->DrawShadow(subtitle, MakeVector2(pos.x + 27.0F, pos.y + 24.0F), 0.65F,
					                 subColor, MakeVector4(0.0F, 0.0F, 0.0F, 0.4F));
				}
			}
		};

		SkinSelectScreen::SkinSelectScreen(UIElement* owner, client::FontManager* fontManager)
		    : UIElement(&owner->GetManager()), owner(owner), fontManager(fontManager) {
			UIManager* manager = &GetManager();
			SetFont(&fontManager->GetGuiFont());

			float sw = manager->screenWidth;
			float sh = manager->screenHeight;
			SetBounds(AABB2(0.0F, 0.0F, sw, sh));

			float dlgW = std::min(600.0F, sw - 32.0F);
			float dlgH = std::min(398.0F, sh - 40.0F);
			float dlgX = (sw - dlgW) * 0.5F;
			float dlgY = (sh - dlgH) * 0.5F;

			std::string currentSkin = static_cast<std::string>(cg_playerSkin);
			std::string currentCape = static_cast<std::string>(cg_playerCape);
			std::string currentAcc = static_cast<std::string>(cg_playerAccessory);

			float cardH = 44.0F;
			float optW = (dlgW - 64.0F) / 3.0F;
			float gap = 8.0F;

			// --- Section 1: Modelo de Skin ---
			float rowY = dlgY + 90.0F;
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioSkinDefault = card.GetPointerOrNull();
				card->groupName = "skinType";
				card->caption = _Tr("MainScreen", "Voxel Original");
				card->subtitle = _Tr("MainScreen", "Clasico OpenSpades");
				card->accentColor = MakeVector4(0.25F, 0.75F, 1.0F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F, rowY, optW, cardH));
				if (currentSkin == "default" || currentSkin.empty())
					card->Check();
				AddChild(card);
			}
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioSkinMinecraft = card.GetPointerOrNull();
				card->groupName = "skinType";
				card->caption = _Tr("MainScreen", "Minecraft (PNG)");
				card->subtitle = _Tr("MainScreen", "Skin estandar 64x64");
				card->accentColor = MakeVector4(0.25F, 0.75F, 1.0F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F + (optW + gap), rowY, optW, cardH));
				if (currentSkin == "minecraft")
					card->Check();
				AddChild(card);
			}
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioSkinCustom = card.GetPointerOrNull();
				card->groupName = "skinType";
				card->caption = _Tr("MainScreen", "Custom 3D (KV6)");
				card->subtitle = _Tr("MainScreen", "Modelo voxeles 3D");
				card->accentColor = MakeVector4(0.25F, 0.75F, 1.0F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F + (optW + gap) * 2.0F, rowY, optW, cardH));
				if (currentSkin == "custom")
					card->Check();
				AddChild(card);
			}

			// --- Section 2: Capa Física (Cloak) ---
			rowY = dlgY + 168.0F;
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioCapeNone = card.GetPointerOrNull();
				card->groupName = "capeType";
				card->caption = _Tr("MainScreen", "Sin Capa");
				card->subtitle = _Tr("MainScreen", "Espalda libre");
				card->accentColor = MakeVector4(0.70F, 0.45F, 1.0F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F, rowY, optW, cardH));
				if (currentCape == "none" || currentCape.empty())
					card->Check();
				AddChild(card);
			}
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioCapeClassic = card.GetPointerOrNull();
				card->groupName = "capeType";
				card->caption = _Tr("MainScreen", "Capa Tactica");
				card->subtitle = _Tr("MainScreen", "Fisica de tela");
				card->accentColor = MakeVector4(0.70F, 0.45F, 1.0F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F + (optW + gap), rowY, optW, cardH));
				if (currentCape == "classic")
					card->Check();
				AddChild(card);
			}
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioCapeGold = card.GetPointerOrNull();
				card->groupName = "capeType";
				card->caption = _Tr("MainScreen", "Capa Dorada");
				card->subtitle = _Tr("MainScreen", "Cosmetico dorado");
				card->tagText = "* ELITE";
				card->isElite = true;
				card->accentColor = MakeVector4(1.0F, 0.82F, 0.22F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F + (optW + gap) * 2.0F, rowY, optW, cardH));
				if (currentCape == "gold")
					card->Check();
				AddChild(card);
			}

			// --- Section 3: Accesorio de Cabeza ---
			rowY = dlgY + 246.0F;
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioAccNone = card.GetPointerOrNull();
				card->groupName = "accType";
				card->caption = _Tr("MainScreen", "Sin Accesorio");
				card->subtitle = _Tr("MainScreen", "Cabeza estandar");
				card->accentColor = MakeVector4(0.35F, 0.80F, 0.70F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F, rowY, optW, cardH));
				if (currentAcc == "none" || currentAcc.empty())
					card->Check();
				AddChild(card);
			}
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioAccHelmet = card.GetPointerOrNull();
				card->groupName = "accType";
				card->caption = _Tr("MainScreen", "Casco Tactico");
				card->subtitle = _Tr("MainScreen", "Proteccion visual");
				card->accentColor = MakeVector4(0.35F, 0.80F, 0.70F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F + (optW + gap), rowY, optW, cardH));
				if (currentAcc == "helmet")
					card->Check();
				AddChild(card);
			}
			{
				Handle<SkinOptionCard> card = Handle<SkinOptionCard>::New(manager);
				radioAccCrown = card.GetPointerOrNull();
				card->groupName = "accType";
				card->caption = _Tr("MainScreen", "Corona Real");
				card->subtitle = _Tr("MainScreen", "Corona de oro");
				card->tagText = "* ELITE";
				card->isElite = true;
				card->accentColor = MakeVector4(1.0F, 0.82F, 0.22F, 1.0F);
				card->SetBounds(AABB2(dlgX + 24.0F + (optW + gap) * 2.0F, rowY, optW, cardH));
				if (currentAcc == "crown")
					card->Check();
				AddChild(card);
			}

			// --- Bottom Buttons ---
			float btnBottomY = dlgY + dlgH - 46.0F;
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				openFolderButton = btn.GetPointerOrNull();
				openFolderButton->caption = _Tr("MainScreen", "Abrir Carpeta Skins");
				openFolderButton->SetBounds(AABB2(dlgX + 24.0F, btnBottomY, 175.0F, 32.0F));
				openFolderButton->activated = [this](UIElement& s) { OnOpenFolder(s); };
				AddChild(openFolderButton);
			}
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				saveButton = btn.GetPointerOrNull();
				saveButton->caption = _Tr("MainScreen", "Guardar");
				saveButton->SetBounds(AABB2(dlgX + dlgW - 200.0F, btnBottomY, 96.0F, 32.0F));
				saveButton->activated = [this](UIElement& s) { OnSave(s); };
				AddChild(saveButton);
			}
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				cancelButton = btn.GetPointerOrNull();
				cancelButton->caption = _Tr("MainScreen", "Cancelar");
				cancelButton->SetBounds(AABB2(dlgX + dlgW - 96.0F, btnBottomY, 72.0F, 32.0F));
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

			// Dimmed background
			SetColorNP(r, MakeVector4(0.02F, 0.03F, 0.05F, 0.82F));
			r.DrawImage(nullptr, AABB2(0.0F, 0.0F, sw, sh));

			float dlgW = std::min(600.0F, sw - 32.0F);
			float dlgH = std::min(398.0F, sh - 40.0F);
			float dlgX = (sw - dlgW) * 0.5F;
			float dlgY = (sh - dlgH) * 0.5F;

			// Main dialog body
			SetColorNP(r, MakeVector4(0.05F, 0.07F, 0.11F, 0.96F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY, dlgW, dlgH));

			// Dialog border
			SetColorNP(r, MakeVector4(0.20F, 0.25F, 0.38F, 0.80F));
			r.DrawOutlinedRect(dlgX, dlgY, dlgX + dlgW, dlgY + dlgH);

			// Glowing top header strip
			SetColorNP(r, MakeVector4(0.70F, 0.40F, 1.0F, 0.95F));
			r.DrawImage(nullptr, AABB2(dlgX, dlgY, dlgW, 3.0F));

			if (fontManager) {
				client::IFont& font = fontManager->GetGuiFont();
				Vector4 shadowCol = MakeVector4(0.0F, 0.0F, 0.0F, 0.7F);

				// Dialog title
				font.DrawShadow(_Tr("MainScreen", "GESTOR DE SKINS Y COSMETICOS"),
				                MakeVector2(dlgX + 24.0F, dlgY + 16.0F), 1.05F,
				                MakeVector4(0.96F, 0.95F, 1.0F, 1.0F), shadowCol);

				font.DrawShadow(_Tr("MainScreen", "Personalizacion 3D, Capas y Accesorios para la Version Y / 1.0"),
				                MakeVector2(dlgX + 24.0F, dlgY + 38.0F), 0.72F,
				                MakeVector4(0.55F, 0.65F, 0.82F, 0.90F), shadowCol);

				// Top horizontal divider
				SetColorNP(r, MakeVector4(0.25F, 0.30F, 0.45F, 0.40F));
				r.DrawImage(nullptr, AABB2(dlgX + 24.0F, dlgY + 56.0F, dlgW - 48.0F, 1.0F));

				// Section 1 Header with Cyan badge
				SetColorNP(r, MakeVector4(0.25F, 0.75F, 1.0F, 0.90F));
				r.DrawImage(nullptr, AABB2(dlgX + 24.0F, dlgY + 70.0F, 4.0F, 12.0F));
				font.DrawShadow(_Tr("MainScreen", "1. MODELO DE SKIN (3D Voxel / MC Extrusion)"),
				                MakeVector2(dlgX + 34.0F, dlgY + 68.0F), 0.78F,
				                MakeVector4(0.85F, 0.92F, 1.0F, 1.0F), shadowCol);

				// Section 2 Header with Purple badge
				SetColorNP(r, MakeVector4(0.70F, 0.45F, 1.0F, 0.90F));
				r.DrawImage(nullptr, AABB2(dlgX + 24.0F, dlgY + 148.0F, 4.0F, 12.0F));
				font.DrawShadow(_Tr("MainScreen", "2. CAPA FISICA (Trailing Cloak Physics)"),
				                MakeVector2(dlgX + 34.0F, dlgY + 146.0F), 0.78F,
				                MakeVector4(0.85F, 0.92F, 1.0F, 1.0F), shadowCol);

				// Section 3 Header with Amber badge
				SetColorNP(r, MakeVector4(0.35F, 0.80F, 0.70F, 0.90F));
				r.DrawImage(nullptr, AABB2(dlgX + 24.0F, dlgY + 226.0F, 4.0F, 12.0F));
				font.DrawShadow(_Tr("MainScreen", "3. ACCESORIO DE CABEZA (3D Props)"),
				                MakeVector2(dlgX + 34.0F, dlgY + 224.0F), 0.78F,
				                MakeVector4(0.85F, 0.92F, 1.0F, 1.0F), shadowCol);

				// Info footer box
				float infoY = dlgY + 302.0F;
				float infoW = dlgW - 48.0F;
				SetColorNP(r, MakeVector4(0.08F, 0.11F, 0.17F, 0.75F));
				r.DrawImage(nullptr, AABB2(dlgX + 24.0F, infoY, infoW, 30.0F));
				SetColorNP(r, MakeVector4(0.20F, 0.28F, 0.42F, 0.50F));
				r.DrawOutlinedRect(dlgX + 24.0F, infoY, dlgX + 24.0F + infoW, infoY + 30.0F);

				// Info icon indicator
				SetColorNP(r, MakeVector4(0.30F, 0.70F, 1.0F, 0.90F));
				r.DrawImage(nullptr, AABB2(dlgX + 32.0F, infoY + 10.0F, 4.0F, 10.0F));

				font.DrawShadow(_Tr("MainScreen", "Skins en 'Skins/skin.png' | Capas en 'Skins/Capes/' | Totalmente sincronizado en Version Y"),
				                MakeVector2(dlgX + 44.0F, infoY + 8.0F), 0.65F,
				                MakeVector4(0.60F, 0.70F, 0.85F, 0.85F), shadowCol);
			}

			UIElement::Render();
		}
	} // namespace gui
} // namespace spades
