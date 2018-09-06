// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
#version 130

in vec4 fragment_color;

out vec4 color;



void main(void)
{
	color = fragment_color * 0.5 + vec4(0.5);

	 /**** Begin of tasks ***
	 - 1.2.5
	 Implement the pseudo-code for calculating the julia fractal at a point.
	 To get the point you can declare a new "in" variable which contains the
	 position and just use the X- and Y- value. 

	 *** End of tasks ***/
	
}