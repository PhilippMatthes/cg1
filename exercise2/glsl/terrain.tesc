#version 430

// Tessellation Control Shader

// Sources:
// http://codeflow.org/entries/2010/nov/07/opengl-4-tessellation/
// https://prideout.net/blog/old/blog/index.html@p=48.html
// https://github.com/fynnfluegge/Lwjgl3-Game-Engine-Programming-Series/blob/tutorial_3/oreon.engine/res/shaders/terrain/Terrain_TC.glsl

layout(vertices = 16) out;


in vec3 TCS_position[];
out vec3 TEC_position[];

uniform vec3 cameraPos;

const int AB = 2;
const int BC = 3;
const int CD = 0;
const int DA = 1;

float lod(float dist) {
    return clamp((-200.0/1200.0) * dist + 64.0, 1.0, 64.0);
}

void main(){
    #define id gl_InvocationID

    // The TES is invoked on every generated vertex.
    // Subsequently, we set our tessellation levels
    // on invocation of the first generated vertex.
    if(id == 0){
        /*
         * Because we want to tessellate quads, we need 6
         * tessellation levels in total:
         * - 2 for the inner tessellation
         * - 4 for the outer tessellation
         * Note: We only need to invoke 2 inner tesellation levels,
         * because the tessellation of one inner vertex is mirrored on the
         * opposite one.
         */

        vec3 abMid = vec3(TCS_position[0] + TCS_position[3])/2.0;
        vec3 bcMid = vec3(TCS_position[3] + TCS_position[15])/2.0;
        vec3 cdMid = vec3(TCS_position[15] + TCS_position[12])/2.0;
        vec3 daMid = vec3(TCS_position[12] + TCS_position[0])/2.0;

        float distanceAB = distance(abMid, cameraPos);
        float distanceBC = distance(bcMid, cameraPos);
        float distanceCD = distance(cdMid, cameraPos);
        float distanceDA = distance(daMid, cameraPos);

        gl_TessLevelOuter[AB] = mix(1.0, 64.0, lod(distanceAB));
        gl_TessLevelOuter[BC] = mix(1.0, 64.0, lod(distanceBC));
        gl_TessLevelOuter[CD] = mix(1.0, 64.0, lod(distanceCD));
        gl_TessLevelOuter[DA] = mix(1.0, 64.0, lod(distanceDA));

        gl_TessLevelInner[0] = (gl_TessLevelOuter[BC] + gl_TessLevelOuter[DA])/4;
        gl_TessLevelInner[1] = (gl_TessLevelOuter[AB] + gl_TessLevelOuter[CD])/4;
    }
    // On invocation of any other vertex than the first,
    // pass through the generated vertex to the Fragment Shader.
    TEC_position[id] = TCS_position[id];
 }
