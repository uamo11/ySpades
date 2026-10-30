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

uniform vec3 fogColor;
varying float hemisphereLighting;

vec3 EvaluateRadiosity(float detailAmbientOcclusion, float ssao) {
	// Micro-contrast curve on voxel ambient occlusion for deeper crevices and contact shadows
	float ao = detailAmbientOcclusion * (0.75 + 0.25 * detailAmbientOcclusion);

	// Ambient skylight with natural atmosphere tone
	vec3 skyAmb = mix(fogColor * 1.08, vec3(0.92, 0.95, 1.0), 0.38);

	return skyAmb * (0.52 * ao * hemisphereLighting * ssao);
}

vec3 EvaluateSoftReflections(float detailAmbientOcclusion, vec3 direction, float ssao) {
    return fogColor * ((direction.z * -0.5 + 0.5) * detailAmbientOcclusion * ssao);
}