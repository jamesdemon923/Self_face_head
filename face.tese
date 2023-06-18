#version 440 core

layout (triangles, equal_spacing, ccw) in;

uniform mat4 gVP;

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

in patch CurvedPNTriangePatch tPatch;

out vec3 inFsWorldPos;
out vec4 inFsColor;
out vec3 inFsNor;
out vec2 inFsTexCoord;

vec2 interpolate_bary_2d(vec3 bary_centric, vec2 v0, vec2 v1, vec2 v2) {
    return vec2(bary_centric.x) * v0 + vec2(bary_centric.y) * v1 + vec2(bary_centric.z) * v2;
}

vec3 interpolate_bary_3d(vec3 bary_centric, vec3 v0, vec3 v1, vec3 v2) {
    return vec3(bary_centric.x) * v0 + vec3(bary_centric.y) * v1 + vec3(bary_centric.z) * v2;
}

vec4 interpolate_bary_4d(vec3 bary_centric, vec4 v0, vec4 v1, vec4 v2) {
    return vec4(bary_centric.x) * v0 + vec4(bary_centric.y) * v1 + vec4(bary_centric.z) * v2;
}

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    float uu = u * u;
    float uuu = uu * u;
    float vv = v * v;
    float vvv = vv * v;
    float ww = w * w;
    float www = ww * w;

    float uu3 = 3.f * uu;
    float vv3 = 3.f * vv;
    float ww3 = 3.f * ww;


    // Position: Cubic
    vec3 pos = tPatch.b300 * www + tPatch.b030 * uuu + tPatch.b003 * vvv + tPatch.b210 * ww3 * u +
    tPatch.b120 * uu3 * w + tPatch.b201 * ww3 * v + tPatch.b021 * uu3 * v +
    tPatch.b102 * vv3 * w + tPatch.b012 * vv3 * u + tPatch.b111 * 6.f * w * u * v;
    // Normal: Quadratic
    vec3 nor = tPatch.n200 * ww + tPatch.n020 * uu + tPatch.n020 * vv + tPatch.n110 * w * u + tPatch.n011 * u * v +
    tPatch.n101 * w * v;

    inFsWorldPos = pos;
    inFsNor = nor;

    vec3 bary_centric = vec3(w, u, v);
    inFsColor = interpolate_bary_4d(bary_centric, tPatch.aColor[0], tPatch.aColor[1], tPatch.aColor[2]);
    inFsTexCoord = interpolate_bary_2d(bary_centric, tPatch.aTexCoord[0], tPatch.aTexCoord[1], tPatch.aTexCoord[2]);

    gl_Position = gVP * vec4(pos, 1.0);
}
