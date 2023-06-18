#version 440 core

uniform mat4 gM;
uniform mat4 gMVP;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNor;
layout (location = 3) in vec2 aTexCoord;

out vec3 inTsPosWorld;
out vec4 inTsColor;
out vec3 inTsNor;
out vec2 inTsTexCoord;

void main() {
    inTsPosWorld = (gM * aPos).xyz;
    // inTsNor = normalize(aNor);
    inTsNor = normalize(mat3(transpose(inverse(gM))) * aNor);
    inTsColor = aColor;
    inTsTexCoord = aTexCoord;

    gl_Position = gMVP * aPos;
}
