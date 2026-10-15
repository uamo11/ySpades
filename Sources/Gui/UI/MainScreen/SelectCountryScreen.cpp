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

#include "SelectCountryScreen.h"

#include <algorithm>
#include <cctype>

#include <Client/Fonts.h>
#include <Client/IFont.h>
#include <Client/IRenderer.h>
#include <Core/Settings.h>
#include <Core/Strings.h>
#include <Gui/UI/Framework/UIManager.h>
#include <Gui/UI/Widgets/DrawUtils.h>
#include <Gui/UI/Widgets/Label.h>

SPADES_SETTING(cg_country);
SPADES_SETTING(cg_countryIsSet);

namespace spades {
	namespace gui {
		using ui::Button;
		using ui::Field;
		using ui::Label;
		using ui::ListView;
		using ui::ListViewModel;
		using ui::SetColorNP;
		using ui::UIElement;
		using ui::UIManager;

		class SelectCountryItem : public ui::ButtonBase {
			SelectCountryScreen* screen;
			CountryInfo info;
			FlagIconRenderer* flagRenderer;

		public:
			SelectCountryItem(UIManager* manager, SelectCountryScreen* screen,
			                  CountryInfo info, FlagIconRenderer* flagRenderer)
			    : ui::ButtonBase(manager),
			      screen(screen),
			      info(std::move(info)),
			      flagRenderer(flagRenderer) {}

			const std::string& GetCode() const { return info.code; }

			void Render() override {
				client::IRenderer& r = GetManager().GetRenderer();
				client::IFont* font = GetFont();
				Vector2 pos = GetScreenPosition();
				Vector2 sz = size;

				bool isSelected = (screen != nullptr && info.code == cg_country);

				Vector4 bgcolor(0.0F, 0.0F, 0.0F, 0.0F);
				if (isSelected) {
					bgcolor = MakeVector4(0.18F, 0.42F, 0.78F, 0.55F);
				} else if (pressed && hover) {
					bgcolor = MakeVector4(1.0F, 1.0F, 1.0F, 0.25F);
				} else if (hover) {
					bgcolor = MakeVector4(1.0F, 1.0F, 1.0F, 0.12F);
				}

				if (bgcolor.w > 0.0F) {
					SetColorNP(r, bgcolor);
					r.DrawImage(nullptr, AABB2(pos.x + 1.0F, pos.y + 1.0F, sz.x - 2.0F, sz.y - 2.0F));
				}

				// Draw country flag icon
				if (flagRenderer) {
					flagRenderer->DrawIcon(info.code,
					                       MakeVector2(pos.x + 18.0F, pos.y + sz.y * 0.5F));
				}

				if (font) {
					Vector4 textColor = isSelected ? MakeVector4(1.0F, 1.0F, 1.0F, 1.0F)
					                               : MakeVector4(0.92F, 0.92F, 0.92F, 0.9F);
					font->Draw(info.name, MakeVector2(pos.x + 36.0F, pos.y + (sz.y - 14.0F) * 0.5F),
					           1.0F, textColor);

					std::string codeTag = "[" + info.code + "]";
					float codeW = font->Measure(codeTag).x;
					font->Draw(codeTag,
					           MakeVector2(pos.x + sz.x - codeW - 12.0F, pos.y + (sz.y - 14.0F) * 0.5F),
					           1.0F, MakeVector4(0.65F, 0.65F, 0.65F, 0.75F));
				}
			}
		};

		class SelectCountryListModel : public ui::ListViewModel {
			UIManager* manager;
			SelectCountryScreen* screen;
			const std::vector<CountryInfo>& items;
			FlagIconRenderer* flagRenderer;

		public:
			SelectCountryListModel(UIManager* manager, SelectCountryScreen* screen,
			                       const std::vector<CountryInfo>& items,
			                       FlagIconRenderer* flagRenderer)
			    : manager(manager),
			      screen(screen),
			      items(items),
			      flagRenderer(flagRenderer) {}

			int GetNumRows() override { return static_cast<int>(items.size()); }

			Handle<UIElement> CreateElement(int row) override {
				if (row < 0 || row >= static_cast<int>(items.size()))
					return Handle<UIElement>();

				Handle<SelectCountryItem> item =
				    Handle<SelectCountryItem>::New(manager, screen, items[row], flagRenderer);

				std::string code = items[row].code;
				item->activated = [this, code](UIElement&) {
					screen->SelectCode(code);
				};
				item->doubleClicked = [this, code](UIElement&) {
					screen->ConfirmCode(code);
				};

				return item.Cast<UIElement>();
			}
		};

		static std::string ToLowerString(const std::string& str) {
			std::string out = str;
			for (char& c : out)
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			return out;
		}

		SelectCountryScreen::SelectCountryScreen(UIElement* owner,
		                                         client::FontManager* fontManager,
		                                         std::function<void(const std::string&)> onSelected,
		                                         bool canCancel)
		    : UIElement(&owner->GetManager()),
		      owner(owner),
		      fontManager(fontManager),
		      onSelected(std::move(onSelected)),
		      canCancel(canCancel) {
			SetBounds(owner->GetBounds());

			UIManager* manager = &GetManager();
			flagIconRenderer = Handle<FlagIconRenderer>::New(&manager->GetRenderer());

			selectedCode = cg_country;
			if (selectedCode.empty())
				selectedCode = "US";

			allCountries = FlagIconRenderer::GetSupportedCountries();
			filteredCountries = allCountries;

			float sw = manager->screenWidth;
			float sh = manager->screenHeight;
			contentsWidth = 480.0F;
			contentsLeft = (sw - contentsWidth) * 0.5F;
			contentsHeight = 440.0F;
			contentsTop = (sh - contentsHeight) * 0.5F;

			// Dim background overlay
			{
				Handle<Label> overlay = Handle<Label>::New(manager);
				overlay->backgroundColor = MakeVector4(0.0F, 0.0F, 0.0F, 0.75F);
				overlay->SetBounds(AABB2(0.0F, 0.0F, sw, sh));
				AddChild(overlay.GetPointerOrNull());
			}

			// Modal background box
			{
				Handle<Label> bg = Handle<Label>::New(manager);
				bg->backgroundColor = MakeVector4(0.05F, 0.07F, 0.10F, 0.95F);
				bg->SetBounds(AABB2(0.0F, contentsTop - 12.0F, sw, contentsHeight + 24.0F));
				AddChild(bg.GetPointerOrNull());
			}

			// Title
			{
				Handle<Label> title = Handle<Label>::New(manager);
				title->text = _Tr("SelectCountryScreen", "Selecciona tu país / Choose Country");
				title->SetFont(&fontManager->GetHeadingFont());
				title->SetBounds(AABB2(contentsLeft, contentsTop + 6.0F, contentsWidth, 28.0F));
				title->alignment = MakeVector2(0.0F, 0.5F);
				AddChild(title.GetPointerOrNull());
			}

			// Subtitle
			{
				Handle<Label> sub = Handle<Label>::New(manager);
				sub->text = _Tr("SelectCountryScreen", "Tu personaje sostendrá la bandera en el lobby.");
				sub->SetBounds(AABB2(contentsLeft, contentsTop + 34.0F, contentsWidth, 20.0F));
				sub->alignment = MakeVector2(0.0F, 0.5F);
				AddChild(sub.GetPointerOrNull());
			}

			// Search / Filter field
			{
				Handle<Field> field = Handle<Field>::New(manager);
				field->SetBounds(AABB2(contentsLeft, contentsTop + 58.0F, contentsWidth, 28.0F));
				field->placeholder = _Tr("SelectCountryScreen", "Buscar país o código...");
				field->changed = [this](UIElement& s) { OnFilterChanged(s); };
				filterField = field.GetPointerOrNull();
				AddChild(filterField);
			}

			// Country list
			{
				Handle<ListView> lv = Handle<ListView>::New(manager);
				lv->rowHeight = 28.0F;
				lv->SetBounds(AABB2(contentsLeft, contentsTop + 92.0F, contentsWidth,
				                    contentsHeight - 142.0F));
				countryList = lv.GetPointerOrNull();
				AddChild(countryList);
			}

			// Cancel / Decide later button
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				btn->caption = canCancel ? _Tr("SelectCountryScreen", "Cancelar")
				                         : _Tr("SelectCountryScreen", "Decidir después");
				btn->SetBounds(AABB2(contentsLeft, contentsTop + contentsHeight - 38.0F,
				                     140.0F, 30.0F));
				btn->activated = [this](UIElement& s) { OnCancelPressed(s); };
				cancelButton = btn.GetPointerOrNull();
				AddChild(cancelButton);
			}

			// Confirm button
			{
				Handle<Button> btn = Handle<Button>::New(manager);
				btn->caption = _Tr("SelectCountryScreen", "Confirmar");
				btn->SetBounds(AABB2(contentsLeft + contentsWidth - 140.0F,
				                     contentsTop + contentsHeight - 38.0F, 140.0F, 30.0F));
				btn->activated = [this](UIElement& s) { OnConfirmPressed(s); };
				confirmButton = btn.GetPointerOrNull();
				AddChild(confirmButton);
			}

			ApplyFilter("");
		}

		void SelectCountryScreen::ApplyFilter(const std::string& query) {
			std::string q = ToLowerString(query);
			filteredCountries.clear();

			for (const auto& c : allCountries) {
				if (q.empty() || ToLowerString(c.name).find(q) != std::string::npos ||
				    ToLowerString(c.code).find(q) != std::string::npos) {
					filteredCountries.push_back(c);
				}
			}

			listModel = Handle<SelectCountryListModel>::New(&GetManager(), this,
			                                                filteredCountries,
			                                                flagIconRenderer.GetPointerOrNull());
			countryList->SetModel(listModel.Cast<ListViewModel>().GetPointerOrNull());
		}

		void SelectCountryScreen::OnFilterChanged(UIElement&) {
			if (filterField)
				ApplyFilter(filterField->GetText());
		}

		void SelectCountryScreen::SelectCode(const std::string& code) {
			selectedCode = code;
			cg_country = code;
			// Refresh list visual
			if (countryList && listModel)
				countryList->SetModel(listModel.Cast<ListViewModel>().GetPointerOrNull());
		}

		void SelectCountryScreen::ConfirmCode(const std::string& code) {
			SelectCode(code);
			cg_countryIsSet = 1;
			Settings::GetInstance()->Flush();
			if (onSelected)
				onSelected(selectedCode);
			Close();
		}

		void SelectCountryScreen::OnConfirmPressed(UIElement&) {
			ConfirmCode(selectedCode);
		}

		void SelectCountryScreen::OnCancelPressed(UIElement&) {
			if (!canCancel) {
				// First run: default to US or current selection
				if (selectedCode.empty())
					selectedCode = "US";
				ConfirmCode(selectedCode);
			} else {
				Close();
			}
		}

		void SelectCountryScreen::Run() {
			owner->enable = false;
			owner->GetParent()->AddChild(this);
			if (filterField)
				GetManager().SetActiveElement(filterField);
		}

		void SelectCountryScreen::Close() {
			owner->enable = true;
			if (GetParent())
				GetParent()->RemoveChild(this);
			if (closed)
				closed(*this);
		}

		void SelectCountryScreen::HotKey(const std::string& key) {
			if (IsEnabled() && (key == "Enter" || key == "Escape")) {
				if (key == "Enter") {
					OnConfirmPressed(*this);
				} else if (key == "Escape") {
					OnCancelPressed(*this);
				}
			} else {
				UIElement::HotKey(key);
			}
		}

		void SelectCountryScreen::Render() {
			client::IRenderer& r = GetManager().GetRenderer();
			Vector2 pos = GetScreenPosition();
			float sw = GetManager().screenWidth;

			// Top & bottom accent border lines
			SetColorNP(r, MakeVector4(0.25F, 0.55F, 0.95F, 0.4F));
			r.DrawImage(nullptr, AABB2(pos.x, pos.y + contentsTop - 12.0F, sw, 2.0F));
			r.DrawImage(nullptr,
			            AABB2(pos.x, pos.y + contentsTop + contentsHeight + 10.0F, sw, 2.0F));

			UIElement::Render();
		}
	} // namespace gui
} // namespace spades
