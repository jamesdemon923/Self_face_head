#version 440 core

layout (vertices = 3) out;

uniform float gTessellationLevel;

in vec3 inTsPosWorld[];
in vec4 inTsColor[];
in vec3 inTsNor[];
in vec2 inTsTexCoord[];

struct CurvedPNTriangePatch {
// TCS Input
// Vertices
    vec3 b300, b030, b003;
// Normals
    vec3 n200, n020, n002;

// TCS Output
// Tangents
    vec3 b210, b120, b021;
    vec3 b012, b102, b201;
// Center
    vec3 b111;
// Mid-edges normals
    vec3 n110, n011, n101;

    vec4 aColor[3];
    vec3 aNor[3];
     vec2 aTexCoord[3];
};

out patch CurvedPNTriangePatch tPatch;

void main() {
    tPatch.aColor[0] = inTsColor[0];
    tPatch.aColor[1] = inTsColor[1];
    tPatch.aColor[2] = inTsColor[2];
    tPatch.aNor[0] = inTsNor[0];
    tPatch.aNor[1] = inTsNor[1];
    tPatch.aNor[2] = inTsNor[2];
    tPatch.aTexCoord[0] = inTsTexCoord[0];
    tPatch.aTexCoord[1] = inTsTexCoord[1];
    tPatch.aTexCoord[2] = inTsTexCoord[2];

    vec3 p1 = inTsPosWorld[0];
    vec3 p2 = inTsPosWorld[1];
    vec3 p3 = inTsPosWorld[2];
    vec3 n1 = inTsNor[0];
    vec3 n2 = inTsNor[1];
    vec3 n3 = inTsNor[2];

    tPatch.b300 = p1;
    tPatch.b030 = p2;
    tPatch.b003 = p3;

    tPatch.n200 = n1;
    tPatch.n020 = n2;
    tPatch.n002 = n3;

    // Calculate vertices control points
    float w12 = dot ((p2 - p1), n1);
    tPatch.b210 = (2.0f * p1 + p2 - w12 * n1) / 3.0f;

    float w21 = dot ((p1 - p2), n2);
    tPatch.b120 = (2.0f * p2 + p1 - w21 * n2) / 3.0f;

    float w23 = dot ((p3 - p2), n2);
    tPatch.b021 = (2.0f * p2 + p3 - w23 * n2) / 3.0f;

    float w32 = dot ((p2 - p3), n3);
    tPatch.b012 = (2.0f * p3 + p2 - w32 * n3) / 3.0f;

    float w31 = dot ((p1 - p3), n3);
    tPatch.b102 = (2.0f * p3 + p1 - w31 * n3) / 3.0f;

    float w13 = dot ((p3 - p1), n1);
    tPatch.b201 = (2.0f * p1 + p3 - w13 * n1) / 3.0f;

    vec3 e = (tPatch.b210 + tPatch.b120 + tPatch.b021 +
    tPatch.b012 + tPatch.b102 + tPatch.b201) / 6.0f;
    vec3 v = (p1 + p2 + p3) / 3.0f;
    tPatch.b111 = e + ((e - v) / 2.0f);

    // Calculate normals control points
    float v12 = 2.0f * dot ((p2 - p1), (n1 + n2)) / dot ((p2 - p1), (p2 - p1));
    tPatch.n110 = normalize ((n1 + n2 - v12 * (p2 - p1)));

    float v23 = 2.0f * dot ((p3 - p2), (n2 + n3)) / dot ((p3 - p2), (p3 - p2));
    tPatch.n011 = normalize ((n2 + n3 - v23 * (p3 - p2)));

    float v31 = 2.0f * dot ((p1 - p3), (n3 + n1)) / dot ((p1 - p3), (p1 - p3));
    tPatch.n101 = normalize ((n3 + n1 - v31 * (p1 - p3)));

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    gl_TessLevelOuter[0] = gTessellationLevel;
    gl_TessLevelOuter[1] = gTessellationLevel;
    gl_TessLevelOuter[2] = gTessellationLevel;
    gl_TessLevelInner[0] = gTessellationLevel;
}
