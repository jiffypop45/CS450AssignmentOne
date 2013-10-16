// SimpleProgram.cpp
//
// A simple 2D OpenGL program


#define GL3_PROTOTYPES

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include "../include/Angel.h"

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

// A global constant for the number of points that will be in our object.
const int NumPoints = 5;

// bluuuuuh these are gross and shouldn't be here, let's figure out a better way to do this later
GLint m = 0;
GLint n = 0;

std::string DATA_DIRECTORY_PATH = "Data\\";
GLdouble NO_DATA = 0.000000;

int discretize_data(GLdouble data_value, GLdouble smallest_data_value, GLdouble largest_data_value, GLint num_buckets) {
	if(data_value == NO_DATA) {
		return -1;
	}             
	return int( (data_value-smallest_data_value)/(largest_data_value-smallest_data_value) * (num_buckets - 1)  + 0.5 );
}

int read_data_from_file(std::string filename, std::vector<GLdouble> & buffer) {
	std::ifstream data_file;
	std::string line;
	std::string filepath;
	int curr_index = 0;
	
	filepath = DATA_DIRECTORY_PATH + filename;
	std::cout << "data_filepath: " << filepath << "\n\n";
	data_file.open(DATA_DIRECTORY_PATH + filename);
	if(data_file.is_open()) {
		getline(data_file, line);
		sscanf(line.c_str(), "# %i %i", &m, &n);
		buffer.resize(m * n);
		std::cout << "Reading " << m * n << " data points from file '" << filename << "'" << std::endl;
		while(getline(data_file, line, ' ')) {
			buffer[curr_index] = stof(line);
			curr_index++;
			if(curr_index >= (m * n) ) {
				break; // TODO: This is not an elegant way to see if more data is available but recieve invalid memory argument exception without.
			}
		}
		data_file.close();
	} else {
	  return 1;
	}
	return 0;
}
//----------------------------------------------------------------------------
void init(void)
{
    // Specifiy the vertices for a rectangle.  The first and last vertex are
    // duplicated to close the box.
    vec2 vertices[] = {
        vec2(-0.5, -0.5),
        vec2(-0.5, 0.5),
        vec2(0.5, 0.5),
        vec2(0.5, -0.5),
        vec2(-0.5, -0.5)
    };

    // Create a vertex array object---OpenGL needs this to manage the Vertex
    // Buffer Object
    GLuint vao[1];

    // Generate the vertex array and then bind it to make make it active.
    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);

    // Create and initialize a buffer object---that's the memory buffer that
    // will be on the card!
    GLuint buffer;

    // We only need one for this example.
    glGenBuffers(1, &buffer);

    // Bind makes it the active VBO
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    // Here we copy the vertex data into our buffer on the card.  The parameters
    // tell it the type of buffer object, the size of the data in bytes, the
    // pointer for the data itself, and a hint for how we intend to use it.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Load the shaders.  Note that this function is not offered by OpenGL
    // directly, but provided as a convenience.
    GLuint program = InitShader("../SimpleProgram/src/vshader32.glsl", 
								"../SimpleProgram/src/fshader32.glsl");

    // Make that shader program active.
    glUseProgram(program);

    // Initialize the vertex position attribute from the vertex shader.  When
    // the shader and the program are linked, a table is created for the shader
    // variables.  Here, we get the index of the vPosition variable.
    GLuint loc = glGetAttribLocation(program, "vPosition");

    // We want to set this with an array!
    glEnableVertexAttribArray(loc);

    // We map it to this offset in our current buffer (VBO) So, our position
    // data is going into loc and contains 2 floats.  The parameters to this
    // function are the index for the shader variable, the number of components,
    // the type of the data, a boolean for whether or not this data is
    // normalized (0--1), stride (or byte offset between consective attributes),
    // and a pointer to the first component.  Note that BUFFER_OFFSET is a macro
    // defined in the Angel.h file.
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // Make the background white
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------
void
display(void)
{
    // clear the window
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the points.  The parameters to the function are: the mode, the first
    // index, and the count.
    glDrawArrays(GL_LINE_STRIP, 0, NumPoints);
    glFlush();
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {

    // Quit when ESC is pressed
    case 27:
        exit(EXIT_SUCCESS);
        break;
    }
}

//------------------------------------------------------------------------------
// This program draws a red rectangle on a white background, but it's still
// missing the machinery to move to 3D.
int main(int argc, char** argv)
{
	// data input
	GLint num_buckets = 20; // default value
	if(argc < 2) {
		std::cerr << "\n\nUsage: " << argv[0] << "DATA_FILENAME" << " NUM_BUCKETS" << std::endl;
		std::cerr << "Default NUM_BUCKETS value is 20" << std::endl;
		return 1;
	}
	if(argc == 3) {
		num_buckets = atoi(argv[2]);
	}
	std::string data_filename = argv[1];
	
	std::vector<GLdouble> data;

	int data_read_status = read_data_from_file(data_filename, data);
	if(data_read_status) {
		std::cerr << "\n\nError: Attempted to read data file '" << data_filename << "'" << std::endl;
		return 1;
	}
	std::cout << "DATA_FILENAME: '" << data_filename << "'\n";
	std::cout << "ROW_DIMENSION: '" << m << "'\n";
	std::cout << "COLUMN_DIMENSION: '" << n << "'\n";

	// discretize and determine colors
	std::vector<int> buckets;
	for(auto point : data) {
		buckets.push_back(discretize_data(point, *std::min_element(data.begin(), data.end()), *std::max_element(data.begin(), data.end()), num_buckets));
	}




	// graphics setup
    glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(500, 300);
    glutCreateWindow("Simple Open GL Program");
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

	glewExperimental = GL_TRUE;

#ifndef __APPLE__
	glewInit();
#endif

    init();

    //NOTE:  callbacks must go after window is created!!!
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();

    return(0);
}
