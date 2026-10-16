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

#include "ClientMenu.h"
#include <Client/ClientUI.h>
#include <Client/ClientUIHelper.h>
#include <Client/Fonts.h>
#include <Client/NetClient.h>
#include <Core/Strings.h>
#include <Gui/UI/Framework/UIManager.h>
#include <Gui/UI/Widgets/Button.h>
#include <Gui/UI/Widgets/Label.h>
#include <Gui/UI/Widgets/GuiIcons.h>

namespace spades {
	namespace client {
		using gui::PreferenceView;
		using gui::PreferenceViewOptions;
		using gui::PreferenceViewPersistedState;
		using gui::ui::Button;
		using gui::ui::Label;
		using gui::ui::UIElement;
		using gui::ui::UIManager;

		namespace {
			class InGameMenuButton : public Button {
			public:
				gui::GuiIconType iconType = gui::GuiIconType::None;
				Vector4 iconColor = MakeVector4(0.85F, 0.90F, 0.95F, 0.90F);

				InGameMenuButton(UIManager* manager, gui::GuiIconType icon, const Vector4& iconCol)
				    : Button(manager), iconType(icon), iconColor(iconCol) {}

				void Render() override {
					client::IRenderer& r = GetManager().GetRenderer();
					Vector2 pos = GetScreenPosition();
					Vector2 sz = size;

					Vector4 bgCol = MakeVector4(0.06F, 0.10F, 0.16F, 0.85F);
					Vector4 icCol = iconColor;

					if (IsEnabled()) {
						if (pressed && hover) {
							bgCol = MakeVector4(0.18F, 0.28F, 0.44F, 0.95F);
							icCol = MakeVector4(1.0F, 1.0F, 1.0F, 1.0F);
						} else if (hover) {
							bgCol = MakeVector4(0.12F, 0.18F, 0.28F, 0.90F);
							icCol = MakeVector4(1.0F, 1.0F, 1.0F, 1.0F);
						}
					} else {
						bgCol.w *= 0.5F;
						icCol.w *= 0.4F;
					}

					gui::ui::SetColorNP(r, bgCol);
					r.DrawFilledRect(pos.x, pos.y, pos.x + sz.x, pos.y + sz.y);

					Vector4 borderCol = hover ? MakeVector4(0.35F, 0.65F, 0.95F, 0.85F)
					                          : MakeVector4(0.20F, 0.35F, 0.55F, 0.45F);
					gui::ui::SetColorNP(r, borderCol);
					r.DrawOutlinedRect(pos.x, pos.y, pos.x + sz.x, pos.y + sz.y);

					if (iconType != gui::GuiIconType::None) {
						gui::DrawGuiIcon(r, iconType, MakeVector2(pos.x + 18.0F, pos.y + sz.y * 0.5F), icCol, 1.15F);
					}

					client::IFont* font = GetFont();
					if (font && !caption.empty()) {
						Vector4 textCol = hover ? MakeVector4(1.0F, 1.0F, 1.0F, 1.0F)
						                        : MakeVector4(0.88F, 0.92F, 0.96F, 0.90F);
						Vector2 textSize = font->Measure(caption);
						Vector2 textPos = MakeVector2(pos.x + 38.0F, pos.y + (sz.y - textSize.y) * 0.5F);
						font->DrawShadow(caption, textPos, 1.0F, textCol, MakeVector4(0.0F, 0.0F, 0.0F, 0.6F));
					}
				}
			};
		} // namespace

		ClientMenu::ClientMenu(ClientUI* ui)
		    : UIElement(&ui->GetUIManager()), ui(ui), helper(&ui->GetHelper()) {
			preferenceState = Handle<PreferenceViewPersistedState>::New();

			UIManager* manager = &GetManager();
			float sw = manager->screenWidth;
			float sh = manager->screenHeight;

			float winW = 200.0F;
			float btnH = 32.0F;
			float spacing = 4.0F;
			float headerH = 26.0F;
			float winH = headerH + (btnH + spacing) * 4.0F;
			float winX = (sw - winW) * 0.5F;
			float winY = (sh - winH) * 0.5F;

			// draw full background overlay
			{
				Handle<Label> overlay = Handle<Label>::New(manager);
				overlay->backgroundColor = MakeVector4(0.0F, 0.0F, 0.0F, 0.65F);
				overlay->SetBounds(AABB2(0.0F, 0.0F, sw, sh));
				AddChild(overlay.GetPointerOrNull());
			}

			// modal menu card background
			{
				Handle<Label> label = Handle<Label>::New(manager);
				label->backgroundColor = MakeVector4(0.03F, 0.06F, 0.10F, 0.85F);
				label->outlineColor = MakeVector4(0.25F, 0.45F, 0.70F, 0.40F);
				label->SetBounds(AABB2(winX - 10.0F, winY - 10.0F, winW + 20.0F, winH + 20.0F));
				AddChild(label.GetPointerOrNull());
			}

			// menu title header
			{
				Handle<Label> title = Handle<Label>::New(manager);
				title->text = _Tr("Client", "ySpades - Menu");
				title->textColor = MakeVector4(0.40F, 0.85F, 1.0F, 0.95F);
				title->alignment = MakeVector2(0.5F, 0.5F);
				title->SetBounds(AABB2(winX, winY, winW, headerH));
				AddChild(title.GetPointerOrNull());
			}

			float curBtnY = winY + headerH + 2.0F;

			// 1. Back to Game (Connect/Play icon)
			{
				Handle<InGameMenuButton> button = Handle<InGameMenuButton>::New(
				    manager, gui::GuiIconType::Connect, MakeVector4(0.35F, 0.85F, 1.0F, 1.0F));
				button->caption = _Tr("Client", "Back to Game");
				button->SetBounds(AABB2(winX, curBtnY, winW, btnH));
				button->activated = [this](UIElement& s) { OnBackToGame(s); };
				AddChild(button.GetPointerOrNull());
			}
			curBtnY += btnH + spacing;

			// 2. Chat Log (Chat bubble icon)
			{
				Handle<InGameMenuButton> button = Handle<InGameMenuButton>::New(
				    manager, gui::GuiIconType::Chat, MakeVector4(0.40F, 0.75F, 1.0F, 1.0F));
				button->caption = _Tr("Client", "Chat Log");
				button->SetBounds(AABB2(winX, curBtnY, winW, btnH));
				button->activated = [this](UIElement& s) { OnChatLog(s); };
				AddChild(button.GetPointerOrNull());
			}
			curBtnY += btnH + spacing;

			// 3. Setup / Settings (Gear icon)
			{
				Handle<InGameMenuButton> button = Handle<InGameMenuButton>::New(
				    manager, gui::GuiIconType::Settings, MakeVector4(0.85F, 0.90F, 0.95F, 1.0F));
				button->caption = _Tr("Client", "Setup");
				button->SetBounds(AABB2(winX, curBtnY, winW, btnH));
				button->activated = [this](UIElement& s) { OnSetup(s); };
				AddChild(button.GetPointerOrNull());
			}
			curBtnY += btnH + spacing;

			// 4. Disconnect (Exit Door icon)
			{
				Handle<InGameMenuButton> button = Handle<InGameMenuButton>::New(
				    manager, gui::GuiIconType::Disconnect, MakeVector4(0.95F, 0.35F, 0.35F, 1.0F));
				button->caption = helper->IsDemoMode() ? _Tr("Client", "Exit Demo")
				                                       : _Tr("Client", "Disconnect");
				button->SetBounds(AABB2(winX, curBtnY, winW, btnH));
				button->activated = [this](UIElement& s) { OnDisconnect(s); };
				AddChild(button.GetPointerOrNull());
			}

			// add resume game hint with badge
			{
				Handle<Label> escHint = Handle<Label>::New(manager);
				escHint->text = _Tr("Client", "[Esc] Back to Game");
				escHint->textColor = MakeVector4(0.80F, 0.85F, 0.90F, 0.80F);
				escHint->alignment = MakeVector2(0.5F, 0.5F);
				escHint->SetBounds(AABB2(winX - 10.0F, winY + winH + 16.0F, winW + 20.0F, 20.0F));
				AddChild(escHint.GetPointerOrNull());
				this->escHint = escHint.GetPointerOrNull();
			}
		}

		void ClientMenu::OnBackToGame(UIElement&) { ui->SetActiveUI(nullptr); }

		void ClientMenu::OnSetup(UIElement&) {
			PreferenceViewOptions opt;
			opt.gameActive = true;
			opt.persistedState = preferenceState;

			if (escHint)
				escHint->visible = false; // hide while in settings

			Handle<PreferenceView> al =
				Handle<PreferenceView>::New(this, opt, &ui->GetFontManager());
			al->closed = [this](UIElement&) {
				if (escHint)
					escHint->visible = true; // restore on close
			};
			al->Run();
		}

		void ClientMenu::OnChatLog(UIElement&) { ui->EnterChatLogWindow(); }

		void ClientMenu::OnDisconnect(UIElement&) { ui->shouldExit = true; }

		void ClientMenu::HotKey(const std::string& key) {
			if (IsEnabled() && key == "Escape") {
				ui->SetActiveUI(nullptr);
			} else {
				UIElement::HotKey(key);
			}
		}

		// IModalMenu implementation
		bool ClientMenu::IsActive() const { return IsEnabled(); }
		void ClientMenu::Open() { /* Managed by ui->SetActiveUI(this) */ }
		void ClientMenu::Close() { if (ui) ui->SetActiveUI(nullptr); }
		bool ClientMenu::KeyEvent(const std::string& key, bool down) {
			if (down) KeyDown(key);
			else KeyUp(key);
			return true;
		}
		void ClientMenu::TextInputEvent(const std::string& text) { KeyPress(text); }
		bool ClientMenu::AcceptsTextInput() const { return IsEnabled(); }
		void ClientMenu::Draw() { Render(); }

		void ClientMenu::Render() {
			UIElement::Render();

			client::IRenderer& r = GetManager().GetRenderer();
			IFont& guiFont = ui->GetFontManager().GetGuiFont();
			IFont& headingFont = ui->GetFontManager().GetHeadingFont();

			Vector4 white = MakeVector4(1, 1, 1, 1);
			Vector4 shadow = MakeVector4(0, 0, 0, 0.6F);
			Vector4 cyan = MakeVector4(0.35F, 0.85F, 1.0F, 1.0F);
			Vector4 green = MakeVector4(0.30F, 0.85F, 0.45F, 1.0F);
			Vector4 amber = MakeVector4(1.0F, 0.82F, 0.25F, 1.0F);

			// Format playing time
			int now = static_cast<int>(helper->GetClientTime());
			int hrs = now / 3600;
			int mins = (now % 3600) / 60;
			int secs = now % 60;

			std::string time;
			if (hrs > 0)
				time = _Tr("Client", "{0}h {1}m {2}s", ToString(hrs), ToString(mins), ToString(secs));
			else if (mins > 0)
				time = _Tr("Client", "{0}m {1}s", ToString(mins), ToString(secs));
			else
				time = _Tr("Client", "{0}s", ToString(secs));

			std::string timeStr = _Tr("Client", "Session time: {0}", time);
			std::string serverName = helper->GetServerName();
			std::string serverStr = serverName.empty() ? "" : _Tr("Client", "Connected to: {0}", serverName);
			const auto& extensions = helper->GetServerExtensions();

			// Calculate card dimensions
			float cardX = 14.0F;
			float cardY = 14.0F;
			float cardW = 320.0F;

			float tW = headingFont.Measure(timeStr).x + 40.0F;
			if (tW > cardW) cardW = tW;
			if (!serverStr.empty()) {
				float sW = guiFont.Measure(serverStr).x + 40.0F;
				if (sW > cardW) cardW = sW;
			}

			float cardH = 36.0F;
			if (!serverStr.empty()) cardH += 26.0F;
			if (!extensions.empty()) cardH += 24.0F + static_cast<float>(extensions.size()) * 18.0F;

			// Floating glass card background & border
			gui::ui::SetColorNP(r, MakeVector4(0.04F, 0.07F, 0.12F, 0.82F));
			r.DrawFilledRect(cardX, cardY, cardX + cardW, cardY + cardH);

			gui::ui::SetColorNP(r, MakeVector4(0.25F, 0.45F, 0.70F, 0.45F));
			r.DrawOutlinedRect(cardX, cardY, cardX + cardW, cardY + cardH);

			// Subtle top accent line
			gui::ui::SetColorNP(r, MakeVector4(0.35F, 0.75F, 1.0F, 0.55F));
			r.DrawLine(MakeVector2(cardX + 2.0F, cardY + 1.0F), MakeVector2(cardX + cardW - 2.0F, cardY + 1.0F), 2.0F);

			float curY = cardY + 9.0F;

			// 1. Session Time with Clock icon
			gui::DrawGuiIcon(r, gui::GuiIconType::Clock, MakeVector2(cardX + 18.0F, curY + 8.0F), cyan, 1.25F);
			headingFont.DrawShadow(timeStr, MakeVector2(cardX + 34.0F, curY), 1.0F, white, shadow);
			curY += headingFont.Measure(timeStr).y + 11.0F;

			// 2. Server Name with Server icon
			if (!serverStr.empty()) {
				gui::DrawGuiIcon(r, gui::GuiIconType::Server, MakeVector2(cardX + 18.0F, curY + 7.0F), green, 1.15F);
				guiFont.DrawShadow(serverStr, MakeVector2(cardX + 34.0F, curY), 1.0F, MakeVector4(0.92F, 0.95F, 0.98F, 1.0F), shadow);
				curY += guiFont.Measure(serverStr).y + 7.0F;
			}

			// 3. Server Extensions with Extension icon
			if (!extensions.empty()) {
				gui::DrawGuiIcon(r, gui::GuiIconType::Extension, MakeVector2(cardX + 18.0F, curY + 7.0F), amber, 1.15F);
				std::string extHeader = _Tr("Client", "Server extensions:");
				guiFont.DrawShadow(extHeader, MakeVector2(cardX + 34.0F, curY), 1.0F, amber, shadow);
				curY += guiFont.Measure(extHeader).y + 3.0F;

				for (const auto& ext : extensions) {
					std::string line = "  * " + GetExtensionDisplayName(ext.id) + " v" + ToString(ext.version);
					Vector4 color = ext.supportedByClient
						? MakeVector4(0.75F, 0.85F, 0.95F, 0.90F)
						: MakeVector4(0.50F, 0.50F, 0.55F, 0.70F);
					guiFont.Draw(line, MakeVector2(cardX + 34.0F, curY), 1.0F, color);
					curY += guiFont.Measure(line).y + 2.0F;
				}
			}
		}
	} // namespace client
} // namespace spades
