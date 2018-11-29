#version 430

// Tessellation Control Shader
// http://codeflow.org/entries/2010/nov/07/opengl-4-tessellation/

layout(vertices = 4) out;

uniform vec2 screenSize;
uniform mat4 mvp;

const float lod = 4.0;

void main(){
     #define id gl_InvocationID
     if(id == 0){
        gl_TessLevelInner[0] = 16;
        gl_TessLevelInner[1] = 16;
        gl_TessLevelOuter[0] = 16;
        gl_TessLevelOuter[1] = 16;
        gl_TessLevelOuter[2] = 16;
        gl_TessLevelOuter[3] = 16;
     }
     gl_out[id].gl_Position = gl_in[id].gl_Position;
 }
