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

#include <Core/Math.h>
#include <Core/RefCountedObject.h>

namespace spades {
	namespace client {
		class IImage;
		class IRenderer;
	}
	namespace gui {
		struct CountryInfo {
			std::string code;
			std::string name;
		};

		/** Draws 16x16 country flag icons from the flag atlas. */
		class FlagIconRenderer : public RefCountedObject {
			Handle<client::IImage> atlas;
			client::IRenderer* renderer; // weak; owned by the screen

		protected:
			~FlagIconRenderer();

		public:
			FlagIconRenderer(client::IRenderer* renderer);

			/** Draws the flag for a two-letter country code centered at `pos`. */
			void DrawIcon(const std::string& name, Vector2 pos);

			/** Draws the flag for a two-letter country code in `dest`. */
			void DrawIcon(const std::string& name, AABB2 dest);

			/** Gets the tile coordinates (x: 0..15, y: 0..15) in the flags atlas. */
			static bool GetFlagCoordinates(const std::string& name, int& x, int& y);

			/** Gets the list of supported countries with their names. */
			static const std::vector<CountryInfo>& GetSupportedCountries();

			/** Gets the human-readable country name for a code. */
			static std::string GetCountryName(const std::string& code);
		};
	} // namespace gui
} // namespace spades
