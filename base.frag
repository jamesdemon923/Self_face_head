#version 440 core

in vec3 inFsPosWorld;
in vec4 inFsColor;
in vec3 inFsNor;
in vec2 inFsTexCoord;

uniform bool gEnableTexture;
//// texture sampler
uniform sampler2D tex1;

out vec4 fColor;

void main() {
    if (gEnableTexture) {
        fColor = texture(tex1, inFsTexCoord);
    } else {
        fColor = inFsColor;
    }
}
