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

#include "LobbyScreen.h"

#include "MainScreenUI.h"
#include "SelectCountryScreen.h"
#include <Client/Fonts.h>
#include <Client/IFont.h>
#include <Client/IRenderer.h>
#include <Core/Settings.h>
#include <Core/Strings.h>
#include <Gui/UI/Framework/UIManager.h>
#include <Gui/UI/Settings/PreferenceView.h>
#include <Gui/UI/Widgets/Button.h>
#include <Gui/UI/Widgets/DrawUtils.h>
#include <Gui/UI/Widgets/Label.h>

SPADES_SETTING(cg_country);

namespace spades {
	namespace gui {
		using ui::Button;
		using ui::Label;
		using ui::SetColorNP;
		using ui::UIElement;
		using ui::UIManager;

		class CountryBadgeButton : public ui::ButtonBase {
			FlagIconRenderer* flagRenderer;
			client::FontManager* fontManager;
			std::string countryCode;
			std::string countryName;

		public:
			CountryBadgeButton(UIManager* manager, FlagIconRenderer* flagRenderer,
			                   client::FontManager* fontManager)
			    : ui::ButtonBase(manager),
			      flagRenderer(flagRenderer),
			      fontManager(fontManager) {
				UpdateBadge();
			}

			void UpdateBadge() {
				countryCode = cg_country;
				if (countryCode.empty())
					countryCode = "US";
				countryName = FlagIconRenderer::GetCountryName(countryCode);
			}

			void Render() override {
				client::IRenderer& r = GetManager().GetRenderer();
				client::IFont* font = &fontManager->GetGuiFont();
				client::IFont* smallFont = &fontManager->GetSmallFont();
				Vector2 pos = GetScreenPosition();
				Vector2 sz = size;

				Vector4 bg = hover ? MakeVector4(0.08F, 0.12F, 0.18F, 0.85F)
				                   : MakeVector4(0.04F, 0.06F, 0.09F, 0.70F);
				if (pressed)
					bg = MakeVector4(0.12F, 0.18F, 0.28F, 0.95F);

				// Background card
				SetColorNP(r, bg);
				r.DrawImage(nullptr, AABB2(pos.x, pos.y, sz.x, sz.y));

				// Border line
				Vector4 borderColor = hover ? MakeVector4(0.35F, 0.65F, 1.0F, 0.6F)
				                            : MakeVector4(1.0F, 1.0F, 1.0F, 0.18F);
				SetColorNP(r, borderColor);
				r.DrawImage(nullptr, AABB2(pos.x, pos.y, sz.x, 1.0F));
				r.DrawImage(nullptr, AABB2(pos.x, pos.y + sz.y - 1.0F, sz.x, 1.0F));
				r.DrawImage(nullptr, AABB2(pos.x, pos.y, 1.0F, sz.y));
				r.DrawImage(nullptr, AABB2(pos.x + sz.x - 1.0F, pos.y, 1.0F, sz.y));

				// Flag icon
				if (flagRenderer) {
					flagRenderer->DrawIcon(countryCode, MakeVector2(pos.x + 22.0F, pos.y + sz.y * 0.5F));
				}

				// Top small label
				if (smallFont) {
					smallFont->Draw(_Tr("Lobby", "BANDERA / COUNTRY:"),
					                MakeVector2(pos.x + 42.0F, pos.y + 7.0F), 1.0F,
					                MakeVector4(0.7F, 0.8F, 0.95F, 0.8F));
				}

				// Country name
				if (font) {
					font->Draw(countryName, MakeVector2(pos.x + 42.0F, pos.y + 20.0F), 1.0F,
					           MakeVector4(1.0F, 1.0F, 1.0F, 1.0F));
				}

				// Hint to change
				if (smallFont) {
					std::string hint = _Tr("Lobby", "(Click para cambiar)");
					float hw = smallFont->Measure(hint).x;
					smallFont->Draw(hint,
					                MakeVector2(pos.x + sz.x - hw - 10.0F, pos.y + (sz.y - 12.0F) * 0.5F),
					                1.0F, MakeVector4(0.55F, 0.75F, 1.0F, hover ? 0.95F : 0.6F));
				}
			}
		};

		LobbyScreen::LobbyScreen(MainScreenUI* ui)
		    : UIElement(&ui->GetUIManager()), ui(ui) {
			UIManager* manager = &GetManager();
			flagIconRenderer = Handle<FlagIconRenderer>::New(&manager->GetRenderer());

			SetBounds(manager->GetRootElement().GetBounds());

			// Play Button
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				btn->caption = _Tr("Lobby", "JUGAR (SERVIDORES)");
				btn->hotKeyText = _Tr("Client", "[Enter]");
				btn->activated = [this](UIElement& s) { OnPlayPressed(s); };
				playButton = btn.GetPointerOrNull();
				AddChild(playButton);
			}

			// Settings Button
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				btn->caption = _Tr("Lobby", "AJUSTES");
				btn->activated = [this](UIElement& s) { OnSettingsPressed(s); };
				settingsButton = btn.GetPointerOrNull();
				AddChild(settingsButton);
			}

			// Country Badge Button
			{
				Handle<CountryBadgeButton> badge = Handle<CountryBadgeButton>::New(
				    manager, flagIconRenderer.GetPointerOrNull(), &ui->GetFontManager());
				badge->activated = [this](UIElement& s) { OnCountryPressed(s); };
				countryBadge = badge.GetPointerOrNull();
				AddChild(countryBadge);
			}

			// Quit Button
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				btn->caption = _Tr("Lobby", "SALIR");
				btn->hotKeyText = _Tr("Client", "[Esc]");
				btn->activated = [this](UIElement& s) { OnQuitPressed(s); };
				quitButton = btn.GetPointerOrNull();
				AddChild(quitButton);
			}

			Layout();
		}

		void LobbyScreen::Layout() {
			float sw = GetManager().screenWidth;
			float sh = GetManager().screenHeight;
			SetBounds(AABB2(0.0F, 0.0F, sw, sh));

			float btnW = 270.0F;
			float btnX = 80.0F;
			if (sw < 640.0F)
				btnX = (sw - btnW) * 0.5F;

			float startY = 240.0F;
			if (sh < 500.0F)
				startY = 180.0F;

			if (playButton)
				playButton->SetBounds(AABB2(btnX, startY, btnW, 46.0F));

			if (settingsButton)
				settingsButton->SetBounds(AABB2(btnX, startY + 54.0F, btnW, 38.0F));

			if (countryBadge)
				countryBadge->SetBounds(AABB2(btnX, startY + 100.0F, btnW, 46.0F));

			if (quitButton)
				quitButton->SetBounds(AABB2(btnX, startY + 154.0F, btnW, 36.0F));
		}

		void LobbyScreen::UpdateCountryBadge() {
			if (countryBadge)
				countryBadge->UpdateBadge();
		}

		void LobbyScreen::OnPlayPressed(UIElement&) {
			ui->ShowServerBrowser();
		}

		void LobbyScreen::OnSettingsPressed(UIElement&) {
			PreferenceViewOptions opt;
			opt.showOverlay = true;
			Handle<PreferenceView> pref =
			    Handle<PreferenceView>::New(this, opt, &ui->GetFontManager());
			pref->Run();
		}

		void LobbyScreen::OnCountryPressed(UIElement&) {
			Handle<SelectCountryScreen> sc = Handle<SelectCountryScreen>::New(
			    this, &ui->GetFontManager(),
			    [this](const std::string& code) {
				    ui->SetCountry(code);
				    UpdateCountryBadge();
			    },
			    true /* canCancel */
			);
			sc->Run();
		}

		void LobbyScreen::OnQuitPressed(UIElement&) {
			ui->shouldExit = true;
		}

		void LobbyScreen::HotKey(const std::string& key) {
			if (IsEnabled() && visible) {
				if (key == "Enter") {
					OnPlayPressed(*this);
					return;
				} else if (key == "Escape") {
					OnQuitPressed(*this);
					return;
				}
			}
			UIElement::HotKey(key);
		}

		void LobbyScreen::Render() {
			Layout();

			client::IRenderer& r = GetManager().GetRenderer();
			client::IFont* font = &ui->GetFontManager().GetSmallFont();
			Vector2 sz = size;

			// Sleek backdrop card behind the menu items on the left
			float btnW = 270.0F;
			float btnX = 80.0F;
			if (sz.x < 640.0F)
				btnX = (sz.x - btnW) * 0.5F;

			float startY = 240.0F;
			if (sz.y < 500.0F)
				startY = 180.0F;

			SetColorNP(r, MakeVector4(0.0F, 0.0F, 0.0F, 0.35F));
			r.DrawImage(nullptr, AABB2(btnX - 16.0F, startY - 16.0F, btnW + 32.0F, 230.0F));

			SetColorNP(r, MakeVector4(0.2F, 0.45F, 0.85F, 0.3F));
			r.DrawImage(nullptr, AABB2(btnX - 16.0F, startY - 16.0F, 3.0F, 230.0F));

			// Lobby title / version in the corner
			if (font) {
				font->Draw("ZeroSpades 3D Lobby", MakeVector2(btnX - 12.0F, startY - 32.0F), 1.0F,
				           MakeVector4(0.8F, 0.85F, 0.9F, 0.75F));
			}

			UIElement::Render();
		}
	} // namespace gui
} // namespace spades
