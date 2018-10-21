// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
#version 130

in vec4 fragment_color;
in vec4 untransformed_position;

out vec4 color;

void main(void)
{

    color = fragment_color * 0.5 + vec4(0.5);
    /*** Begin of tasks ***
    - 1.2.5
    Implement the pseudo-code for calculating the julia fractal at a point.
    To get the point you can declare a new "in" variable which contains the
    position and just use the X- and Y- value. */

    // Seed vector
    vec2 c = vec2(0.5, 0.5);

    // Zoom factor
    float m = 1;

    vec2 p = untransformed_position.xy;

    vec2 z = p * m;

    int i;
    int i_max = 200;
    for (i; i < i_max; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (z.x * z.y + z.x * z.y) + c.y;
        if ((x * x + y * y) > 10.0) break;
        z.x = x;
        z.y = y;
    }

    float a = 0;
    if (i < i_max) a = float(i)/float(i_max);

    vec4 fractal = vec4(a, a, a, a);
    color = fractal * 10.0 * fragment_color;

    /*** End of tasks ***/
	
}