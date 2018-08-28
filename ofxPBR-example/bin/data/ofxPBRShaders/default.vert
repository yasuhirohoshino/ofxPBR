#version 410

const int MAX_LIGHTS = 8;

#pragma include "pbrVert.glsl"

void main() {
	SendPBRVaryings();
}