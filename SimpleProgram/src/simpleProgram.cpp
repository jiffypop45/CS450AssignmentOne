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

// input data dimensions
GLint m = 0;
GLint n = 0;

std::string DATA_DIRECTORY_PATH = "Data\\";
GLfloat NO_DATA = 0.000000;
GLfloat data_min = 0.;
GLfloat data_max = 0.;

struct node {
	GLfloat position[2];
	GLfloat *rgb;
};

// returns the bucket number from 1 to num_buckets
int discretize_data(GLfloat data_value, GLfloat smallest_data_value, GLfloat largest_data_value, GLint num_buckets) {
	if(data_value == NO_DATA) {
		return -1;
	}             
	return int( (data_value-smallest_data_value)/(largest_data_value-smallest_data_value) * (num_buckets - 1)  + 0.5 );
}

// returns whether file was read
bool read_data_from_file(std::string filename, std::vector<GLfloat>& buffer) {
	std::ifstream data_file;
	std::string line;
	std::string filepath;
	int curr_index = 0;
	GLfloat curr_data_val;

	filepath = DATA_DIRECTORY_PATH + filename;
	data_file.open(DATA_DIRECTORY_PATH + filename);
	if(data_file.is_open()) {
		getline(data_file, line);
		sscanf(line.c_str(), "# %i %i", &m, &n);
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < m; j++) {
				if(!getline(data_file, line, ' ')) break;
				curr_data_val = stof(line);
				if(curr_data_val > data_max) {
					data_max = curr_data_val;
				} else if(curr_data_val < data_min) {
					data_min = curr_data_val;
				}
				buffer.push_back(curr_data_val);
			}
		}
		data_file.close();
	} else {
	  return false;
	}
	return true;
}


// get this here data on that there graphics card
void init(std::vector<node> vertex_data)
{
	vertex_data.shrink_to_fit();
	float *colors_temp = (float*)malloc(sizeof(float[3]) * vertex_data.size());
	GLfloat *vertex_temp = (GLfloat*)malloc(sizeof(GLfloat[2]) * vertex_data.size());

	for(int i = 0; i < vertex_data.size(); i++) {
		vertex_temp[2 * i] = vertex_data[i].position[0];
		vertex_temp[2 * i + 1] = vertex_data[i].position[1];
		colors_temp[3 * i] = vertex_data[i].rgb[0];
		colors_temp[3 * i + 1] = vertex_data[i].rgb[1];
		colors_temp[3 * i + 2] = vertex_data[i].rgb[2];
	}

    // Create a vertex array object---OpenGL needs this to manage the Vertex
    // Buffer Object
    GLuint vao;
    // Generate the vertex array and then bind it to make make it active.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object---that's the memory buffer that
    // will be on the card!
	// 0 is triangle vertices, 1 is colors
    GLuint buffer[2];
    glGenBuffers(2, buffer);

    // Bind makes it the active VBO
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);

    // Here we copy the vertex data into our buffer on the card.  The parameters
    // tell it the type of buffer object, the size of the data in bytes, the
    // pointer for the data itself, and a hint for how we intend to use it.
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[2]) * vertex_data.size(), vertex_temp, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float[3]) * vertex_data.size(), colors_temp, GL_STATIC_DRAW);
    // Load the shaders.  Note that this function is not offered by OpenGL
    // directly, but provided as a convenience.
    GLuint program = InitShader("../SimpleProgram/src/vshader32.glsl", 
								"../SimpleProgram/src/fshader32.glsl");

    // Make that shader program active.
    glUseProgram(program);

    // Initialize the vertex position attribute from the vertex shader.  When
    // the shader and the program are linked, a table is created for the shader
    // variables.  Here, we get the index of the vPosition variable.
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
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
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	loc = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
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
    glDrawArrays(GL_TRIANGLES, 0, m*n*6);
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

//----------------------------------------------------------------------------
// the HSV color model will be as follows
// h : [0 - 360]
// s : [0 - 1]
// v : [0 - 1]
// If you want it differently (in a 2 * pi scale, 256 instead of 1, etc,
// you'll have to change it yourself.
// rgb is returned in 0-1 scale (ready for color3f)
void HSVtoRGB(float hsv[3], float rgb[3]) {
	float tmp1 = hsv[2] * (1-hsv[1]);
	float tmp2 = hsv[2] * (1-hsv[1] * (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) ));
	float tmp3 = hsv[2] * (1-hsv[1] * (1 - (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) )));
	switch((int)(hsv[0] / 60)) {
		case 0:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp3 ;
			rgb[2] = tmp1 ;
			break;
		case 1:
			rgb[0] = tmp2 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp1 ;
			break;
		case 2:
			rgb[0] = tmp1 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp3 ;
			break;
		case 3:
			rgb[0] = tmp1 ;
			rgb[1] = tmp2 ;
			rgb[2] = hsv[2] ;
			break;
		case 4:
			rgb[0] = tmp3 ;
			rgb[1] = tmp1 ;
			rgb[2] = hsv[2] ;
			break;
		case 5:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp1 ;
			rgb[2] = tmp2 ;
			break;
		default:
			std::cout << "Inconceivable!\n";
	}
    
}


int main(int argc, char** argv)
{
	// data input
	GLint num_buckets = 20; // default value
	if(argc < 2) {
		std::cerr << "\n\nUsage: " << argv[0] << "DATA_FILENAME" << " NUM_BUCKETS" << std::endl;
		std::cerr << "File must be in Data/ directory" << std::endl;
		std::cerr << "Default NUM_BUCKETS value is 20" << std::endl;
		return 1;
	}

	// params
	if(argc == 3) {
		num_buckets = atoi(argv[2]);
	}
	std::string data_filename = argv[1];
	
	// file IO
	std::vector<GLfloat> data;
	int data_read_status = read_data_from_file(data_filename, data);
	if(data_read_status == false) {
		std::cerr << "Error: Failed to read data file '" << data_filename << "'" << std::endl;
		std::cerr << "File must be in ./Data directory" << std::endl;
		return 1;
	}

	// discretize
	std::vector<int> buckets;
	for(auto point : data) {
		buckets.push_back(discretize_data(point, data_min, data_max, num_buckets));
	}

	std::vector<GLfloat> hues(num_buckets, 0);
	for(int i = 0; i < num_buckets; i++) {
		hues[i] = (num_buckets - i - 1) * (240.0f / (num_buckets - 1));
	}
	
	// hsv to rgb conversion
	std::vector<float*> rgbs; // aw yiss vector of arrays. access each bucket rgb by rgbs[bucket][0 through 2]
	for(auto hue : hues) {
		rgbs.push_back(new float[3]);
		float hsv[3];
		hsv[0] = hue;
		hsv[1] = hsv[2] = 1.0f;
		HSVtoRGB(hsv, rgbs[rgbs.size() - 1]);
	}

	const GLfloat X_MAX = .95;
	const GLfloat Y_MAX = .95;
	const GLfloat X_MIN = -.95;
	const GLfloat Y_MIN = -.95;
	node a, b, c, d;
	GLfloat curr_data_val;
	float WHITE_RGB[3];
	WHITE_RGB[0] = 1.0f;
	WHITE_RGB[1] = 1.0f;
	WHITE_RGB[2] = 1.0f;

	float *curr_rgb;
	std::vector<node> vertex_data;
	int curr_bucket_val;
	for(int num_y = 0; num_y < n - 1; num_y++) {
		for(int num_x = 0; num_x < m - 1; num_x++) {
			int i = (num_y * m) + num_x;
			curr_bucket_val = buckets[i];
			curr_data_val = data[i];

			// account for no data areas, which should stay white
			if(curr_bucket_val == -1) {
				curr_rgb = WHITE_RGB;
			}
			else {
				curr_rgb = rgbs[curr_bucket_val];
			}

			// figure out screen location based on xy coords
			a.position[0] = X_MIN + (X_MAX - X_MIN) * (GLfloat)num_x / (GLfloat)(m - 1);
			a.position[1] = Y_MIN + (Y_MAX - Y_MIN) * (GLfloat)(num_y + 1) / (GLfloat)(n - 1);
			a.rgb = curr_rgb;
				
			b.position[0] = X_MIN + (X_MAX - X_MIN) * (GLfloat)(num_x + 1)/ (GLfloat)(m - 1);
			b.position[1] = Y_MIN + (Y_MAX - Y_MIN) * (GLfloat)(num_y + 1) / (GLfloat)(n - 1);
			b.rgb = curr_rgb;
				
			c.position[0] = X_MIN + (X_MAX - X_MIN) * (GLfloat)(num_x + 1) / (GLfloat)(m - 1);
			c.position[1] = Y_MIN + (Y_MAX - Y_MIN) * (GLfloat)(num_y) / (GLfloat)(n - 1);
			c.rgb = curr_rgb;
				
			d.position[0] = X_MIN + (X_MAX - X_MIN) * (GLfloat)num_x / (GLfloat)(m - 1);
			d.position[1] = Y_MIN + (Y_MAX - Y_MIN) * (GLfloat)num_y / (GLfloat)(n - 1);
			d.rgb = curr_rgb;

			vertex_data.push_back(a);
			vertex_data.push_back(b);
			vertex_data.push_back(c);
				
			vertex_data.push_back(a);
			vertex_data.push_back(c);
			vertex_data.push_back(d);
		}
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
    glutInitWindowSize(768 * m / max(m, n), 768 * n / max(m, n));
    glutInitWindowPosition(100, 100);
	glutCreateWindow(data_filename.c_str());
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

	glewExperimental = GL_TRUE;

#ifndef __APPLE__
	glewInit();
#endif

    init(vertex_data);

    //NOTE:  callbacks must go after window is created!!!
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();

    return(0);
}
