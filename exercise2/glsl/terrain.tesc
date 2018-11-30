#version 430

// Tessellation Control Shader
// http://codeflow.org/entries/2010/nov/07/opengl-4-tessellation/

layout(vertices = 4) out;

uniform vec2 screenSize;
uniform mat4 mvp;

const float lod = 4.0;

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
        gl_TessLevelInner[0] = lod;
        gl_TessLevelInner[1] = lod;

        gl_TessLevelOuter[0] = lod;
        gl_TessLevelOuter[1] = lod;
        gl_TessLevelOuter[2] = lod;
        gl_TessLevelOuter[3] = lod;
     }
     // On invocation of any other vertex than the first,
     // pass through the generated vertex to the Fragment Shader.
     gl_out[id].gl_Position = gl_in[id].gl_Position;
 }
