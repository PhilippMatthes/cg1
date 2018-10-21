// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/checkbox.h>

#include <gui/SliderHelper.h>

#include <iostream>

#include "glsl.h"

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 1")
{ 
	SetupGUI();

	CreateShaders();
	CreateVertexBuffers();

	modelViewMatrix.setIdentity();
	projectionMatrix.setIdentity();

	camera().FocusOnBBox(nse::math::BoundingBox<float, 3>(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1)));
}

void Viewer::SetupGUI()
{
	auto mainWindow = SetupMainWindow();

	//Create GUI elements for the various options
	chkHasDepthTesting = new nanogui::CheckBox(mainWindow, "Perform Depth Testing");
	chkHasDepthTesting->setChecked(true);

	chkHasFaceCulling = new nanogui::CheckBox(mainWindow, "Perform backface Culling");
	chkHasFaceCulling->setChecked(true);

	sldJuliaCX = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "JuliaC.X", std::make_pair(-1.0f, 1.0f), 0.45f, 2);
	sldJuliaCY = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "JuliaC.Y", std::make_pair(-1.0f, 1.0f), -0.3f, 2);
	sldJuliaZoom = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Julia Zoom", std::make_pair(0.01f, 10.0f), 1.0f, 2);

	performLayout();
}

// Create and define the vertex array and add a number of vertex buffers
void Viewer::CreateVertexBuffers()
{
	/*** Begin of task 1.2.3 ***
	Fill the positions-array and your color array with 12 rows, each
	containing 4 entries, to define a tetrahedron. */

	/*
	 *
	 *    H---G		Tetrahedron: AFC, FHC, ACH, FAH
	 *   /|  /|		A = -1, -1, -1, 1,	E = -1, -1, 1, 1,
	 * 	D-E-C F		B = 1, -1 , -1, 1,	F = 1, -1, 1, 1,
	 *  |   |/		C = 1, 1, -1, 1,	G = 1, 1, 1, 1,
	 *  A---B		D = -1, 1, -1, 1,	H = -1, 1, 1, 1,
	 */

	GLfloat positions[] = {
            // Front triangle
            0, 1, 0, 1,
            -1, -1, 0, 1,
            1, -1, 0, 1,

            // Right side triangle
            1, -1, 0, 1,
            0, 1, 0, 1,
            0,  -1, -1, 1,

            // Left side triangle
            -1, -1, 0, 1,
            0, 1, 0, 1,
            0, -1, -1, 1,

            // Bottom triangle
            -1, -1, 0, 1,
            1, -1, 0, 1,
            0, -1, -1, 1,
    };

	GLfloat colors[] = {
            1, 0, 0, 1,
            1, 0, 0, 1,
            1, 0, 0, 1,
            0, 1, 0, 1,
            0, 1, 0, 1,
            0, 1, 0, 1,
            0, 0, 1, 1,
            0, 0, 1, 1,
            0, 0, 1, 1,
            1, 1, 0, 1,
            1, 1, 0, 1,
            1, 1, 0, 1,
	};

	// Generate the vertex array 
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	// Generate a position buffer to be appended to the vertex array
	glGenBuffers(1, &position_buffer_id);
	// Bind the buffer for subsequent settings
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer_id);
	// Supply the position data
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	// The buffer shall now be linked to the shader attribute
	// "in_position". First, get the location of this attribute in 
	// the shader program
	GLuint vid = glGetAttribLocation(program_id, "in_position");
	
	// Enable this vertex attribute array
	glEnableVertexAttribArray(vid);
	// Set the format of the data to match the type of "in_position"
	glVertexAttribPointer(vid, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/*** Begin of task 1.2.2 (a)
	Create another buffer that will store color information. This works nearly
	similar to the code above that creates the position buffer. Store the buffer
	id into the variable "color_buffer_id" and bind the color buffer to the
    shader variable "in_color".
    ***/

	std::cout << "Generating color buffer..." << "\n";
	glGenBuffers(1, &color_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    std::cout << "Getting location of in_color in shader program..." << "\n";
	GLuint cid = glGetAttribLocation(program_id, "in_color");

	std::cout << "Enabling vertex attribute array..." << "\n";
	glEnableVertexAttribArray(cid);

    std::cout << "Setting data format to match the type of in_color..." << "\n";
	glVertexAttribPointer(cid, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/*** End of task 1.2.2 (a) ***/
	
	

	// Unbind the vertex array to leave OpenGL in a clean state
	glBindVertexArray(0);
}

//Checks if the given shader has been compiled successfully. Otherwise, prints an
//error message and throws an exception.
//  shaderId - the id of the shader object
//  name - a human readable name for the shader that is printed together with the error
void CheckShaderCompileStatus(GLuint shaderId, std::string name)
{
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		std::cerr << "Error while compiling shader \"" << name << "\":" << std::endl;
		glGetShaderInfoLog(shaderId, 512, nullptr, buffer);
		std::cerr << "Error: " << std::endl << buffer << std::endl;
		throw std::runtime_error("Shader compilation failed!");
	}
}

// Read, Compile and link the shader codes to a shader program
void Viewer::CreateShaders()
{
	/*** Begin of task 1.2.1 ***
	Use the appropriate OpenGL commands to create a shader object for
    the vertex shader, set the source code and let it compile. Store the
    ID of this shader object in the variable "vertex_shader_id". Repeat
    for the fragment shader. Store the ID in the variable "fragment_shader_id.
	Finally, create a shader program with its handle stored in "program_id",
	attach both shader objects and link them. For error checking, you can
	use the method "CheckShaderCompileStatus()" after the call to glCompileShader().
	*/

	/*
	 *  Use the appropriate OpenGL commands to create a shader object for
	    the vertex shader, set the source code and let it compile. Store the
	    ID of this shader object in the variable "vertex_shader_id". Repeat
	    for the fragment shader. Store the ID in the variable "fragment_shader_id.
	 */

	std::cout << "Creating shaders..." << "\n";
	vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	/*
	 *  Finally, create a shader program with its handle stored in "program_id",
	    attach both shader objects and link them. For error checking, you can
	    use the method "CheckShaderCompileStatus()" after the call to glCompileShader().
	 */
    std::cout << "Creating program..." << "\n";
	program_id = glCreateProgram();

    std::cout << "Creating shader sources..." << "\n";
    std::string vs((char*)shader_vert, shader_vert_size);
    const char *vertex_content = vs.c_str();

    std::string fs((char*)shader_frag, shader_frag_size);
    const char *fragment_content = fs.c_str();

    std::cout << "Linking shader sources..." << "\n";
    glShaderSource(vertex_shader_id, 1, &vertex_content, 0);
    glShaderSource(fragment_shader_id, 1, &fragment_content, 0);

    std::cout << "Compiling shaders..." << "\n";
    glCompileShader(vertex_shader_id);
    glCompileShader(fragment_shader_id);

    std::cout << "Attaching shaders to program..." << "\n";
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);

	std::cout << "Linking program..." << "\n";
	glLinkProgram(program_id);

	std::cout << "Detaching shaders..." << "\n";
	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);

    CheckShaderCompileStatus(vertex_shader_id, "Vertex Shader");
    CheckShaderCompileStatus(fragment_shader_id, "Fragment Shader");

	/*** End of task 1.2.1 ***/
}

void Viewer::drawContents()
{
	Eigen::Vector2f juliaC(sldJuliaCX->value(), sldJuliaCY->value());
	float juliaZoom = sldJuliaZoom->value();

	//Get the transform matrices
	camera().ComputeCameraMatrices(modelViewMatrix, projectionMatrix);

	// If has_faceculling is set then enable backface culling
	// and disable it otherwise
	if (chkHasFaceCulling->checked())
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	// If has_depthtesting is set then enable depth testing
	// and disable it otherwise
	if (chkHasDepthTesting->checked())
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	// Activate the shader program
	glUseProgram(program_id);

	/*** Begin of task 1.2.4 (b) ***
	Set the shader variables for the modelview and projection matrix.
	First, find the location of these variables using glGetUniformLocation and
	then set them with the command glUniformMatrix4fv. 
	*/

	GLint modelview_matrix_id = glGetUniformLocation(program_id, "modelview_matrix");
	GLint projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");

	glUniformMatrix4fv(modelview_matrix_id, 1, GL_TRUE, modelViewMatrix.data());
	glUniformMatrix4fv(projection_matrix_id, 1, GL_TRUE, projectionMatrix.data());

	// Bind the vertex array 
	glBindVertexArray(vertex_array_id);
	// Draw the bound vertex array. Start at element 0 and draw 3 vertices
	glDrawArrays(GL_TRIANGLES, 0, 12);

	/*** End of task 1.2.4 (b) ***/
	
	// Unbind the vertex array
	glBindVertexArray(0);
	// Deactivate the shader program
	glUseProgram(0);
}