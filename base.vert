#version 440 core

uniform mat4 gM;
uniform mat4 gMVP;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNor;
layout (location = 3) in vec2 aTexCoord;

out vec3 inFsPosWorld;
out vec4 inFsColor;
out vec3 inFsNor;
out vec2 inFsTexCoord;

void main() {
    inFsPosWorld = (gM * aPos).xyz;
    // inTsNor = normalize(aNor);
    inFsNor = normalize(mat3(transpose(inverse(gM))) * aNor);
    inFsColor = aColor;
    inFsTexCoord = aTexCoord;

    gl_Position = gMVP * aPos;
}
