// Use 'w' to move forward, 'a' to turn left, 'd' to turn right. The house hase several beds,
// doors that open and close, stairs that sometimes work, and a tennis court. The camera uses
// the z buffer to detect collisions. The color on the lower portion on the screen is used to detect
// stairs. 
//
// Jordan White
// Date Completed: 12/01/2014

#include <cassert>
#include <iostream>
#include "cs432.h"
#include "materials.h"
#include "matStack.h"
#include "picking.h"
#include "objParse.h"
#include "ppm.h"

using namespace std;

//====================================================================================

#define WINDOW_HEIGHT 760
#define WINDOW_WIDTH 1080
#define TICK_INTERVAL 50
#define TURN_ANGLE .05
#define MOVE_SPEED .1
#define NUM_TEX 2



typedef vec4 point4;
typedef vec4 color4;

const int NumVertices = 100000;

// Textures
const int TextureWidth = 1024;
const int TextureHeight = 1024;

// Texture objects and storage for texture image
static GLuint textures[NUM_TEX];

static GLubyte image[TextureHeight][TextureWidth][3];

// Vertex data arrays
static point4 points[NumVertices];
static color4 colors[NumVertices];
static vec3	normals[NumVertices];
static vec2 tex_coords[NumVertices];


// Lighting
static lightStruct light = {
	color4(0.5, 0.5, 0.5, 1.0),
	color4(1.0, 1.0, 1.0, 1.0),
	color4(0.2, 0.2, 02, 1.2),
	color4(0.0, 10.0, 0.0, 1.0),
};


static materialStruct houseMaterials = {
	color4(0.5, 0.5, 0.5, 1.0),
	color4(0.5, 0.5, 0.5, 1.0),
	color4(0.2, 0.2, 02, 1.2),
	color4(0.0, 0.0, 0.0, 1.0),
	0.0
};



// Camera set up
static vec4 eye = vec4(0.0, 2.0, 15.0, 1.0);
static vec4 at = vec4(0.0, 1.75, 14.0, 1.0);
static vec4 up = vec4(0.0, 1.0, 0.0, 1.0);
static GLfloat theta = 0.0;

const color4 BKGD(0.5, 0.75, 0.75, 1.0); // background, light grey

static MatrixStack mvstack;
static mat4         model_view, ctm, projection, camera;
static GLuint       ModelView, Projection, buffers[2];
static GLint		ColorMap;

static GLuint		program;

//Objects
static ObjRef house;
static ObjRef door1;
static ObjRef door2;

// Doors and Lights
static bool door1Open = false;
static bool door2Open = false;

static bool light1On = true;
static bool light2On = true;


static int Index[] = { 0, NumVertices };

static vec4 product(vec4 a, vec4 b)
{
	return vec4(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

static void drawObject(ObjRef objRef) {
	int start = objRef.getStartIdx();
	glDrawArrays(GL_TRIANGLES, start, objRef.getCount());
}

// For picking
static int pickedID = 0;


// Draws scene
static void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	model_view = LookAt(eye, at, up);

	mvstack.push(model_view);

	// Draw house
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	drawObject(house);
	
	// door1 texture
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	mvstack.push(model_view);
	if(door1Open) model_view *= Translate(1.2, 0.0, 0.0);
	
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	drawObject(door1);
	model_view = mvstack.pop();

	// door2 texture
	glBindTexture(GL_TEXTURE_2D, textures[2]);

	mvstack.push(model_view);
	if (door2Open) model_view *= Translate(0.0, 0.0, 1.2);

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	drawObject(door2);
	model_view = mvstack.pop();

	model_view = mvstack.pop();

	// Clean up
	glBindTexture(GL_TEXTURE_2D, 0);

	// Finished with the current picking id
	clearPickId();

	// Finish picking or draw the most recently drawn frame
	if (inPickingMode()) {
		endPicking();
	}
	else {
		glutSwapBuffers();
	}
}



static void picking(int code) {
	cout << "code: " << code << endl;
	pickedID = code;
	if (pickedID == 1142911) {
		door1Open = !door1Open;
		pickedID = 0;
	} 
	else if (pickedID == 2433657) {
		door2Open = !door2Open;
		pickedID = 0;
	}
}

//----------------------------------------------------------------------------
static void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		cout << "mouse: " << x << ", " << y << endl;
		startPicking(picking, x, y);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

	}
}

//----------------------------------------------------------------------------

static void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat left = -200, right = 200.0;
	GLfloat bottom = -200.0, top = 200.0;
	GLfloat zNear = 1.0, zFar = 10000.0;

	GLfloat aspect = GLfloat(width) / height;

	if (aspect > 1.0) {
		left *= aspect;
		right *= aspect;
	}
	else {
		bottom /= aspect;
		top /= aspect;
	}

	projection = Perspective(55, 1.0, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

	//model_view = mat4(1.0);
}

//----------------------------------------------------------------------------

static void init(void) {

	house = genObject("house.obj", Index, points, colors, normals, tex_coords);
	door1 = genObject("door1.obj", Index, points, colors, normals, tex_coords);
	door2 = genObject("door2.obj", Index, points, colors, normals, tex_coords);


	const int ImageWidth = 1024;
	const int ImageHeight = 1024;

	// images
	static GLfloat pic[ImageHeight][ImageWidth][3];
	static GLfloat pic2[ImageHeight][ImageWidth][3];
	static GLfloat pic3[ImageHeight][ImageWidth][3];

	// Load house texture
	readPpmImage("HouseTexture.ppm", (GLfloat*)pic, 0, 0, ImageWidth, ImageHeight);
	gluScaleImage(
		GL_RGB, // as in GL_RGB
		ImageWidth, // width of existing image, in pixels
		ImageHeight, // height of existing image, in pixels
		GL_FLOAT, // type of data in existing image, as in GL_FLOAT
		pic, // pointer to first element of existing image
		TextureWidth, // width of new image
		TextureHeight, // height of new image
		GL_BYTE, // type of new image
		image); // pointer to buffer for holding new image


	// Initialize texture objects
	glGenTextures(NUM_TEX, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);
	
	// door1 texture
	readPpmImage("door1.ppm", (GLfloat*)pic2, 0, 0, ImageWidth, ImageHeight);
	gluScaleImage(
		GL_RGB, // as in GL_RGB
		ImageWidth, // width of existing image, in pixels
		ImageHeight, // height of existing image, in pixels
		GL_FLOAT, // type of data in existing image, as in GL_FLOAT
		pic2, // pointer to first element of existing image
		TextureWidth, // width of new image
		TextureHeight, // height of new image
		GL_BYTE, // type of new image
		image); // pointer to buffer for holding new image


	// Initialize texture objects
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// door2 texture
	readPpmImage("door2.ppm", (GLfloat*)pic3, 0, 0, ImageWidth, ImageHeight);
	gluScaleImage(
		GL_RGB, // as in GL_RGB
		ImageWidth, // width of existing image, in pixels
		ImageHeight, // height of existing image, in pixels
		GL_FLOAT, // type of data in existing image, as in GL_FLOAT
		pic3, // pointer to first element of existing image
		TextureWidth, // width of new image
		TextureHeight, // height of new image
		GL_BYTE, // type of new image
		image); // pointer to buffer for holding new image


	// Initialize texture objects
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);


	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/// Create and initialize a buffer object
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(points) + sizeof(colors) + sizeof(tex_coords) + sizeof(normals),
		NULL, GL_STATIC_DRAW);

	// Specify an offset to keep track of where we're placing data in our
	//   vertex array buffer.  We'll use the same technique when we
	//   associate the offsets with vertex attribute pointers.
	GLintptr offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points), points);
	offset += sizeof(points);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors), colors);
	offset += sizeof(colors);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(tex_coords), tex_coords);
	offset += sizeof(tex_coords);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals), normals);
	offset += sizeof(normals);

	// Load shaders and use the resulting shader program
	const GLchar* vShaderCode =
		"attribute vec4 vPosition; "
		"attribute in vec4 vNormal; "
		"attribute  vec4 vColor; "
		"attribute  vec2 vTexCoord; "
		" "
		"uniform mat4 ModelView; "
		"uniform vec4 LightPosition; "
		"uniform mat4 Projection; "
		"uniform vec4 PickColor; "
		" "
		"varying  vec4 color; "
		"varying  vec2 texCoord; "
		"varying vec4 normal; "
		"varying out vec3 N; "
		"varying out vec3 L; "
		"varying out vec3 E; "
		"varying vec3 V; "
		" "
		"void main()  "
		"{ "
		"color = vColor; "
		"texCoord = vTexCoord; "
		"normal = vNormal; "
		" "
		"vec3 eyePosition = vec3(ModelView*vPosition); "
		"vec3 eyeLightPos = (ModelView*LightPosition).xyz; "
		" "
		"if (PickColor.a >= 0.0) { "
		"color = PickColor; "
		"} "
		"gl_Position = Projection*ModelView*vPosition; "
		//"normal = normalize(gl_NormalMatrix * gl_Normal); "
		"}  "
		;
	const GLchar* fShaderCode =
		"uniform sampler2D texture; "
		" "
		"uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct; "
		"uniform mat4 ModelView; "
		"uniform vec4 LightPosition; "
		"uniform float Shininess; "
		" "
		"varying  vec4 color; "
		"varying  vec2 texCoord; "
		"varying vec4 normal; "
		" "
		"varying vec3 N; "
		"varying vec3 L; "
		"varying vec3 E; "
		"varying vec3 V; "
		" "
		"void main()  "
		"{  "
		"gl_FragColor = texture2D( texture, texCoord ); "
		"}  "
		;

	program = InitShader2(vShaderCode, fShaderCode);
	glUseProgram(program);

	offset = 0;

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset += sizeof(points);
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset += sizeof(colors);

	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset += sizeof(tex_coords);

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset += sizeof(normals);

	glClearColor(BKGD[0], BKGD[1], BKGD[2], BKGD[3]); //  background
	//showPickColors(true);

	ModelView = glGetUniformLocation(program, "ModelView");
	ctm = LookAt(eye, at, up);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, ctm);

	Projection = glGetUniformLocation(program, "Projection");

	ColorMap = glGetUniformLocation(program, "ColorMap");

	GLint LightPosition = glGetUniformLocation(program, "LightPosition");
	glUniform4fv(LightPosition, 1, light.position);

	GLint DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	glUniform4fv(DiffuseProduct, 1, product(light.diffuse, houseMaterials.diffuse));

	GLint AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	glUniform4fv(AmbientProduct, 1, houseMaterials.ambient);

	GLint SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	glUniform4fv(SpecularProduct, 1, product(light.specular, houseMaterials.specular));

	setGpuPickColorId(glGetUniformLocation(program, "PickColor"));


	// Set the value of the fragment shader texture sampler variable
	//   ("texture") to the the appropriate texture unit. In this case,
	//   zero, for GL_TEXTURE0 which was previously set by calling
	//   glActiveTexture().
	glUniform1i(glGetUniformLocation(program, "texture"), 0);

	//showPickColors(true);
	glEnable(GL_DEPTH_TEST);
}

//----------------------------------------------------------------------------

static void
keyboard(unsigned char key, int x, int y)
{
	GLubyte stair[3] = { 66, 37, 28 };
	GLubyte footPixel[4];
	GLubyte stairCheck[4];
	float center, feet, left, right;
	glReadPixels(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &center);
	glReadPixels(WINDOW_WIDTH / 2, 0.0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &feet);
	glReadPixels(WINDOW_WIDTH / 3, WINDOW_HEIGHT / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &left);
	glReadPixels((WINDOW_WIDTH*2)/3, WINDOW_HEIGHT / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &right);
	
	glReadPixels(WINDOW_WIDTH / 2, 0.0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, footPixel);
	glReadPixels(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, stairCheck);

	cout << "feet: " << (float)feet << endl;
	cout << "center: " << (float)center << endl;
	cout << "left: " << (float)left << endl;
	cout << "right: " << (float)right << endl;
	

	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'd':
		theta -= TURN_ANGLE;
		break;
	case 'a':
		theta += TURN_ANGLE;;
		break;
	case 'w':
		if (center > .45 && left > .45 && right > .45) {
			eye.x -= MOVE_SPEED*sin(theta);
			eye.z -= MOVE_SPEED*cos(theta);
		}
		at.y = eye.y - .25;
		if (stair[0] == footPixel[0]) {
			if (stairCheck[0] == stair[0] || center == 1) {
				eye.y += .8*MOVE_SPEED;
				at.y = eye.y + .2;
			}
			else if (center != 1) {
				eye.y -= .8*MOVE_SPEED;
				at.y = eye.y - .2;
			}
		}
	case ' ':
		break;
	}
	
	at.x = -sin(theta) + eye.x;
	at.z = -cos(theta) + eye.z;
}



//----------------------------------------------------------------------------
// callback function: occurs every time the clock ticks: update the angles and redraws the scene
static void tick(int n) {
	// set up next "tick"
	glutTimerFunc(n, tick, n);

	// draw the new scene
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("House");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutTimerFunc(TICK_INTERVAL, tick, TICK_INTERVAL); // timer callback
	glutMainLoop();
	delete[] textures;
	return 0;
}

//------------------------------------------------------------------------------


