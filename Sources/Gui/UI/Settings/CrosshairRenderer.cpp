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

#include "CrosshairRenderer.h"
#include <Client/IRenderer.h>
#include <Gui/UI/Widgets/DrawUtils.h>

namespace spades {
	namespace gui {
		namespace {
			inline void FillRectI(client::IRenderer& r, int x0, int y0, int x1, int y1) {
				r.DrawFilledRect(static_cast<float>(x0), static_cast<float>(y0),
								 static_cast<float>(x1), static_cast<float>(y1));
			}
			
			void DrawTargetOutlineRect(client::IRenderer& r, int x, int y, int w, int h,
			                           const TargetParam& param) {
				int thickness = static_cast<int>(param.outlineThickness);
				ui::SetColorNP(r, param.outlineColor);
				if (param.useRoundedStyle) {
					FillRectI(r, x, y - thickness, w, y);
					FillRectI(r, x, h, w, h + thickness);
				} else {
					FillRectI(r, x - thickness, y - thickness, w + thickness, y);
					FillRectI(r, x - thickness, h, w + thickness, h + thickness);
				}
				FillRectI(r, x - thickness, y, x, h);
				FillRectI(r, w, y, w + thickness, h);
			}

			void DrawTargetRect(client::IRenderer& r, int x, int y, int w, int h,
			                    const TargetParam& param) {
				if (param.drawOutline)
					DrawTargetOutlineRect(r, x, y, w, h, param);
				ui::SetColorNP(r, param.lineColor);
				FillRectI(r, x, y, w, h);
			}
		} // namespace

		void DrawTarget(client::IRenderer& r, Vector2 pos, const TargetParam& param) {
			int thickness = static_cast<int>(param.lineThickness);
			int x = static_cast<int>(pos.x) - thickness / 2;
			int y = static_cast<int>(pos.y) - thickness / 2;
			int w = x + thickness;
			int h = y + thickness;

			// draw target lines
			if (param.drawLines) {
				int lineGap = static_cast<int>(param.lineGap);

				// horizontal lines
				int lineLengthHorizontal = static_cast<int>(param.lineLength.x);
				int innerLeft = x - lineGap;
				int innerRight = w + lineGap;
				int outerLeft = innerLeft - lineLengthHorizontal;
				int outerRight = innerRight + lineLengthHorizontal;
				DrawTargetRect(r, outerLeft, y, innerLeft, h, param); // left
				DrawTargetRect(r, innerRight, y, outerRight, h, param); // right

				// vertical lines
				int lineLengthVertical = static_cast<int>(param.lineLength.y);
				int innerTop = y - lineGap;
				int innerBottom = h + lineGap;
				int outerTop = innerTop - lineLengthVertical;
				int outerBottom = innerBottom + lineLengthVertical;
				if (!param.useTStyle)
					DrawTargetRect(r, x, outerTop, w, innerTop, param); // top
				DrawTargetRect(r, x, innerBottom, w, outerBottom, param); // bottom
			}

			// draw center dot
			if (param.drawDot) {
				thickness = static_cast<int>(param.dotThickness);
				x = static_cast<int>(pos.x) - thickness / 2;
				y = static_cast<int>(pos.y) - thickness / 2;
				w = x + thickness;
				h = y + thickness;

				if (param.drawOutline)
					DrawTargetOutlineRect(r, x, y, w, h, param);
				ui::SetColorNP(r, param.dotColor);
				FillRectI(r, x, y, w, h);
			}
		}

		void DrawCrosshairPreset(client::IRenderer& r, Vector2 pos, int preset, const Vector4& color) {
			Vector4 black = MakeVector4(0.0F, 0.0F, 0.0F, color.w * 0.9F);
			int cx = static_cast<int>(pos.x);
			int cy = static_cast<int>(pos.y);

			switch (preset) {
				case 3: {
					// Clásico HD-Plus (Ace of Spades crosshair in crisp HD vector)
					int gap = 3;
					int len = 7;
					int thick = 2; // 2px width
					int half = thick / 2;

					// Outlines (1px black border around every arm)
					ui::SetColorNP(r, black);
					FillRectI(r, cx - gap - len - 1, cy - half - 1, cx - gap + 1, cy + half + 1);
					FillRectI(r, cx + gap - 1, cy - half - 1, cx + gap + len + 1, cy + half + 1);
					FillRectI(r, cx - half - 1, cy - gap - len - 1, cx + half + 1, cy - gap + 1);
					FillRectI(r, cx - half - 1, cy + gap - 1, cx + half + 1, cy + gap + len + 1);

					// Core bars in color
					ui::SetColorNP(r, color);
					FillRectI(r, cx - gap - len, cy - half, cx - gap, cy + half);
					FillRectI(r, cx + gap, cy - half, cx + gap + len, cy + half);
					FillRectI(r, cx - half, cy - gap - len, cx + half, cy - gap);
					FillRectI(r, cx - half, cy + gap, cx + half, cy + gap + len);
					break;
				}
				case 4: {
					// Punto HD (Precision Dot)
					ui::SetColorNP(r, black);
					FillRectI(r, cx - 2, cy - 2, cx + 3, cy + 3);
					ui::SetColorNP(r, color);
					FillRectI(r, cx - 1, cy - 1, cx + 2, cy + 2);
					break;
				}
				case 5: {
					// Cruz Pro (Competitive Esports Cross)
					int gap = 2;
					int len = 5;

					ui::SetColorNP(r, black);
					FillRectI(r, cx - gap - len - 1, cy - 1, cx - gap + 1, cy + 2);
					FillRectI(r, cx + gap - 1, cy - 1, cx + gap + len + 1, cy + 2);
					FillRectI(r, cx - 1, cy - gap - len - 1, cx + 2, cy - gap + 1);
					FillRectI(r, cx - 1, cy + gap - 1, cx + 2, cy + gap + len + 1);

					ui::SetColorNP(r, color);
					FillRectI(r, cx - gap - len, cy, cx - gap, cy + 1);
					FillRectI(r, cx + gap, cy, cx + gap + len, cy + 1);
					FillRectI(r, cx, cy - gap - len, cx + 1, cy - gap);
					FillRectI(r, cx, cy + gap, cx + 1, cy + gap + len);
					break;
				}
				case 6: {
					// Cruz + Punto HD (Pro Cross with Center Dot)
					int gap = 4;
					int len = 5;

					ui::SetColorNP(r, black);
					FillRectI(r, cx - gap - len - 1, cy - 1, cx - gap + 1, cy + 2);
					FillRectI(r, cx + gap - 1, cy - 1, cx + gap + len + 1, cy + 2);
					FillRectI(r, cx - 1, cy - gap - len - 1, cx + 2, cy - gap + 1);
					FillRectI(r, cx - 1, cy + gap - 1, cx + 2, cy + gap + len + 1);
					FillRectI(r, cx - 2, cy - 2, cx + 3, cy + 3);

					ui::SetColorNP(r, color);
					FillRectI(r, cx - gap - len, cy, cx - gap, cy + 1);
					FillRectI(r, cx + gap, cy, cx + gap + len, cy + 1);
					FillRectI(r, cx, cy - gap - len, cx + 1, cy - gap);
					FillRectI(r, cx, cy + gap, cx + 1, cy + gap + len);
					FillRectI(r, cx - 1, cy - 1, cx + 2, cy + 2);
					break;
				}
				case 7: {
					// Círculo Táctico (Tactical CQB Ring with Center Dot & 4 Ticks)
					int r0 = 6;
					ui::SetColorNP(r, black);
					FillRectI(r, cx - r0 - 1, cy - 1, cx - r0 + 2, cy + 2);
					FillRectI(r, cx + r0 - 1, cy - 1, cx + r0 + 2, cy + 2);
					FillRectI(r, cx - 1, cy - r0 - 1, cx + 2, cy - r0 + 2);
					FillRectI(r, cx - 1, cy + r0 - 1, cx + 2, cy + r0 + 2);

					FillRectI(r, cx - r0 - 5, cy, cx - r0 - 1, cy + 1);
					FillRectI(r, cx + r0 + 1, cy, cx + r0 + 5, cy + 1);
					FillRectI(r, cx, cy - r0 - 5, cx + 1, cy - r0 - 1);
					FillRectI(r, cx, cy + r0 + 1, cx + 1, cy + r0 + 5);

					FillRectI(r, cx - 2, cy - 2, cx + 3, cy + 3);
					ui::SetColorNP(r, color);
					FillRectI(r, cx - 1, cy - 1, cx + 2, cy + 2);

					FillRectI(r, cx - r0, cy, cx - r0 + 1, cy + 1);
					FillRectI(r, cx + r0, cy, cx + r0 + 1, cy + 1);
					FillRectI(r, cx, cy - r0, cx + 1, cy - r0 + 1);
					FillRectI(r, cx, cy + r0, cx + 1, cy + r0 + 1);
					FillRectI(r, cx - r0 + 2, cy - r0 + 2, cx - r0 + 3, cy - r0 + 3);
					FillRectI(r, cx + r0 - 2, cy - r0 + 2, cx + r0 - 1, cy - r0 + 3);
					FillRectI(r, cx - r0 + 2, cy + r0 - 2, cx - r0 + 3, cy + r0 - 1);
					FillRectI(r, cx + r0 - 2, cy + r0 - 2, cx + r0 - 1, cy + r0 - 1);
					break;
				}
				case 8: {
					// Chevron / Flecha (Military Arrow ^)
					ui::SetColorNP(r, black);
					for (int i = 0; i <= 5; ++i) {
						FillRectI(r, cx - i - 1, cy + i - 1, cx - i + 2, cy + i + 2);
						FillRectI(r, cx + i - 1, cy + i - 1, cx + i + 2, cy + i + 2);
					}
					FillRectI(r, cx - 8, cy + 5, cx - 4, cy + 7);
					FillRectI(r, cx + 5, cy + 5, cx + 9, cy + 7);

					ui::SetColorNP(r, color);
					for (int i = 0; i <= 5; ++i) {
						FillRectI(r, cx - i, cy + i, cx - i + 1, cy + i + 1);
						FillRectI(r, cx + i, cy + i, cx + i + 1, cy + i + 1);
					}
					FillRectI(r, cx - 7, cy + 5, cx - 4, cy + 6);
					FillRectI(r, cx + 5, cy + 5, cx + 8, cy + 6);
					break;
				}
				default: break;
			}
		}
	} // namespace gui
} // namespace spades
