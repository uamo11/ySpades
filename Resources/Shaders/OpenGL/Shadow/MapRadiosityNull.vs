/*
 Copyright (c) 2013 yvt

 This file is part of OpenSpades.

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

varying float hemisphereLighting;

void PrepareForMapRadiosity(vec3 vertexCoord, vec3 normal) {
	// Smooth sky-to-ground ambient hemisphere lighting (-Z is UP in OpenSpades)
	float skyFactor = clamp(-normal.z * 0.5 + 0.5, 0.0, 1.0);
	hemisphereLighting = mix(0.74, 1.18, skyFactor);
}

void PrepareForMapRadiosityForMap(vec3 vertexCoord, vec3 centerCoord, vec3 normal) {
	float skyFactor = clamp(-normal.z * 0.5 + 0.5, 0.0, 1.0);
	hemisphereLighting = mix(0.74, 1.18, skyFactor);
}