#version 330

#define MAX_LIGHTS 8

#pragma include "pbrVert.glsl"

void main() {
    SendPBRVaryings();
}