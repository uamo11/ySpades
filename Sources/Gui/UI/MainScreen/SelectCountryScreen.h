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

#include <functional>
#include <string>
#include <vector>

#include "CountryFlags.h"
#include <Gui/UI/Widgets/ButtonBase.h>
#include <Gui/UI/Widgets/Field.h>
#include <Gui/UI/Widgets/ListView.h>

namespace spades {
	namespace client {
		class FontManager;
	}
	namespace gui {
		class SelectCountryListModel;

		/** Modal dialog for selecting player country / flag. */
		class SelectCountryScreen : public ui::UIElement {
			ui::UIElement* owner; // weak
			client::FontManager* fontManager; // weak
			Handle<FlagIconRenderer> flagIconRenderer;

			ui::Field* filterField = nullptr;
			ui::ListView* countryList = nullptr;

			std::string selectedCode;
			std::vector<CountryInfo> allCountries;
			std::vector<CountryInfo> filteredCountries;
			Handle<SelectCountryListModel> listModel;

			std::function<void(const std::string&)> onSelected;
			bool canCancel;

			void ApplyFilter(const std::string& query);
			void OnFilterChanged(ui::UIElement& sender);
			void RebuildModel();

		public:
			ui::EventHandler closed;

			SelectCountryScreen(ui::UIElement* owner, client::FontManager* fontManager,
			                    std::function<void(const std::string&)> onSelected,
			                    bool canCancel = true);

			void Run();
			void Close();

			void ConfirmCode(const std::string& code);

			void HotKey(const std::string& key) override;
			void Render() override;
		};
	} // namespace gui
} // namespace spades
