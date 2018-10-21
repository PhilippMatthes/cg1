﻿// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
#version 130

in vec4 in_position;

/* - 1.2.2 (b)
 * Declare a new "in" variable with the name "in_color". Instead of setting
 * "fragment_color" to the position, set it to "in_color. */
in vec4 in_color;

out vec4 fragment_color;

/* - 1.2.4 (a)
 * Declare two new "uniform" variables with the type "mat4" (above the main function)
 * that store the modelview and projection matrix. To apply the transformations
 * multiply the value of "in_position" before setting "gl_Position". */

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;


void main(void)
{

    fragment_color = in_color;

    vec4 transformed_position = in_position * modelview_matrix * projection_matrix;

    gl_Position = transformed_position;

	/* - 1.2.5
	 * The algorithm to calculate the julia fractal needs a position as input.
	 * Declare another "out" variable and set it to the untransformed input
	 * position. */
}