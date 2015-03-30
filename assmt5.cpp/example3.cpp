	/* sets up flat mesh */
/* sets up elapsed time parameter for use by shaders */

#include "cs432.h"
#include "mat.h"
#include "vec.h"

#define N 256

static GLfloat normals[N][N][3];
static GLuint         program;
static GLuint        texMapLocation;
static GLfloat tangent[3] = {1.0, 0.0, 0.0};

typedef vec4  point4;
typedef vec4  color4;

static point4 points[6];
static vec2 tex_coord[6];
static mat4 ctm, projection;

static vec4 normal = point4(0.0, 1.0, 0.0, 0.0);
static color4 light_diffuse = color4(1.0, 1.0, 1.0, 1.0);
static color4 material_diffuse = color4(0.7, 0.7, 0.7, 1.0);
static point4  light_position = point4(0.0, 10.0, 0.0, 1.0);
static vec4 eye =  vec4(2.0, 2.0, 2.0, 1.0);
static vec4 at = vec4(0.5, 0.0, 0.5, 1.0);
static vec4 up = vec4(0.0, 1.0, 0.0, 1.0);



static GLuint loc, loc2;
static GLuint buffers[2];

static GLuint normal_loc;
static GLuint diffuse_product_loc;
static GLuint light_position_loc;
static GLuint ctm_loc, projection_loc;
static GLuint tangent_loc;


/* standard OpenGL initialization */

static vec4 product(vec4 a, vec4 b)
{
  return vec4(a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3]);
}
static void init()
{
    const float meshColor[]     = {0.7f, 0.7f, 0.7f, 1.0f}; 

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, meshColor);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGB, GL_FLOAT, normals);
    glEnable(GL_TEXTURE_2D);


    glEnable(GL_DEPTH_TEST);
 
   loc = glGetAttribLocation(program, "vPosition");
   glEnableVertexAttribArray(loc);
   glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, points);

   loc2 = glGetAttribLocation(program, "texcoord");
   glEnableVertexAttribArray(loc2);
   glVertexAttribPointer(loc2, 2, GL_FLOAT, GL_FALSE, 0, tex_coord);

   glGenBuffers(2, buffers);
   glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

   tangent_loc = glGetUniformLocation(program, "objTangent");
   glUniform3fv(tangent_loc, 1, tangent);

   normal_loc = glGetUniformLocation(program, "Normal");
   glUniform4fv(normal_loc, 1, normal);

   vec4 diffuse_product = product(light_diffuse, material_diffuse);
   diffuse_product_loc = glGetUniformLocation(program, "DiffuseProduct");
   glUniform4fv(diffuse_product_loc, 1, diffuse_product);

   light_position_loc = glGetUniformLocation(program, "LightPosition");
   glUniform4fv(light_position_loc, 1, light_position);

   ctm_loc = glGetUniformLocation(program, "ModelView");
   ctm = LookAt(eye, at , up);
   glUniformMatrix4fv(ctm_loc, 1, GL_TRUE, ctm);

   mat4 nm;
   GLfloat det;
   det = ctm[0][0]*ctm[1][1]*ctm[2][2]+ctm[0][1]*ctm[1][2]*ctm[2][1]
     -ctm[2][0]*ctm[1][1]*ctm[0][2]-ctm[1][0]*ctm[0][1]*ctm[2][2]-ctm[0][0]*ctm[1][2]*ctm[2][1];
   nm[0][0] = (ctm[1][1]*ctm[2][2]-ctm[1][2]*ctm[2][1])/det;
   nm[0][1] = -(ctm[0][1]*ctm[2][2]-ctm[0][2]*ctm[2][1])/det;
   nm[0][2] = (ctm[0][1]*ctm[2][0]-ctm[2][1]*ctm[2][2])/det;
   nm[1][0] = -(ctm[0][1]*ctm[2][2]-ctm[0][2]*ctm[2][1])/det;
   nm[1][1] = (ctm[0][0]*ctm[2][2]-ctm[0][2]*ctm[2][0])/det;
   nm[1][2] = -(ctm[0][0]*ctm[2][1]-ctm[2][0]*ctm[0][1])/det;
   nm[2][0] = (ctm[0][1]*ctm[1][2]-ctm[1][1]*ctm[0][2])/det;
   nm[2][1] = -(ctm[0][0]*ctm[1][2]-ctm[0][2]*ctm[1][0])/det;
   nm[2][2] = (ctm[0][0]*ctm[1][1]-ctm[1][0]*ctm[0][1])/det;

    GLuint nm_loc;
    nm_loc = glGetUniformLocation(program, "NormalMatrix");
    glUniformMatrix4fv(nm_loc, 1, GL_TRUE, nm);

    projection_loc = glGetUniformLocation(program, "Projection");
    projection = Ortho(-0.75,0.75,-0.75,0.75,-5.5,5.5);
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);

    texMapLocation = glGetUniformLocation(program, "texMap");

}

    /* set up uniform parameter */

static void mesh()
{
      point4 vertices[4] = {point4(0.0, 0.0, 0.0, 1.0), point4(1.0, 0.0, 0.0, 1.0),
         point4(1.0, 0.0, 1.0, 1.0), point4(0.0, 0.0, 1.0, 1.0)};

       points[0] = vertices[0];
       tex_coord[0] = vec2(0.0, 0.0);
       points[1] = vertices[1];
       tex_coord[1] = vec2(1.0, 0.0);
       points[2] = vertices[2];
       tex_coord[2] = vec2(1.0, 1.0);
       points[3] = vertices[2];
       tex_coord[3] = vec2(1.0, 1.0);
       points[4] = vertices[3];
       tex_coord[4] = vec2(0.0, 1.0);
       points[5] = vertices[0];
       tex_coord[5] = vec2(0.0, 0.0);
}

static void draw()
{

    glUniform1i(texMapLocation, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mesh(); 

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glutSwapBuffers();
}

static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
    case 'Q':
    case 'q':
        exit(EXIT_SUCCESS);
    }
}

static void idle()
{
   int t;
   t = glutGet(GLUT_ELAPSED_TIME);
   light_position[0] = 5.5*sin(0.001*t);
   light_position[2] = 5.5*cos(0.001*t);
   glUniform4fv(light_position_loc, 1, light_position);
   glutPostRedisplay();
}

int main3(int argc, char** argv)
{
    int i,j, k;
    float d;
    
    float data[N+1][N+1];
    for(i=0;i<N+1;i++) for(j=0;j<N+1;j++) data[i][j]=0.0;
    for(i=N/4; i< 3*N/4; i++) for(j=N/4;j<3*N/4;j++) data[i][j] = 1.0;

    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
       normals[i][j][0] = data[i][j]-data[i+1][j];
       normals[i][j][2] = data[i][j]-data[i][j+1];
       normals[i][j][1]= 1.0;
    }


    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
       d = 0.0;
       for(k=0;k<3;k++) d+=normals[i][j][k]*normals[i][j][k];
       d=sqrt(d);
       for(k=0;k<3;k++) normals[i][j][k]= 0.5*normals[i][j][k]/d+0.5;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("Simple GLSL example");
	glewInit();
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);


    const GLchar* vShaderCode =
      "/* bump map vertex shader */ "
      " "
      "varying vec3 L; /* light vector in texture-space coordinates */ "
      "varying vec3 V; /* view vector in texture-space coordinates */ "
      " "
      "attribute vec2 texcoord; "
      "attribute vec4 vPosition; "
      " "
      "uniform vec4 Normal; "
      "uniform vec4 LightPosition; "
      "uniform mat4 ModelView; "
      "uniform mat4 Projection; "
      "uniform mat4 NormalMatrix; "
      "uniform vec3 objTangent; /* tangent vector in object coordinates */ "
      " "
      "varying vec2 st; "
      " "
      "void main() "
      "{ "
      "mat3 NM3; "
      " "
      "NM3[0][0] = NormalMatrix[0][0]; "
      "NM3[0][1] = NormalMatrix[0][1]; "
      "NM3[0][2] = NormalMatrix[0][2]; "
      "NM3[1][0] = NormalMatrix[1][0]; "
      "NM3[1][1] = NormalMatrix[1][1]; "
      "NM3[1][2] = NormalMatrix[1][2]; "
      "NM3[2][0] = NormalMatrix[2][0]; "
      "NM3[2][1] = NormalMatrix[2][1]; "
      "NM3[2][2] = NormalMatrix[2][2]; "
      " "
      "gl_Position = Projection*ModelView*vPosition; "
      " "
      " "
      "st = texcoord; "
      " "
      "vec3 eyePosition = vec3(ModelView*vPosition); "
      "vec3 eyeLightPos = (ModelView*LightPosition).xyz; "
      " "
      "/* normal, tangent and binormal in eye coordinates */ "
      " "
      "vec3 N = normalize(NM3*Normal.xyz); "
      "vec3 T  = normalize(NM3*objTangent); "
      "vec3 B = cross(N, T); "
      " "
      "/* light vector in texture space */ "
      " "
      "L.x = dot(T, eyeLightPos-eyePosition); "
      "L.y = dot(B, eyeLightPos-eyePosition); "
      "L.z = dot(N, eyeLightPos-eyePosition); "
      " "
      "L = normalize(L); "
      " "
      "/* view vector in texture space */ "
      " "
      "V.x = dot(T, -eyePosition); "
      "V.y = dot(B, -eyePosition); "
      "V.z = dot(N, -eyePosition); "
      " "
      "V = normalize(V); "
      "} "
      ;
    const GLchar* fShaderCode = 
      "varying vec3 L; "
      "varying vec3 V; "
      "uniform sampler2D texMap; "
      "varying vec2 st; "
      "uniform vec4 DiffuseProduct; "
      " "
      "void main() "
      "{ "
      " "
      "vec4 N = texture2D(texMap, st); "
      "vec3 NN =  normalize(2.0*N.xyz-1.0); "
      "vec3 LL = normalize(L); "
      "float Kd = max(dot(NN.xyz, LL), 0.0); "
      "gl_FragColor = Kd*DiffuseProduct; "
      "} "
      ;
    program = InitShader2(vShaderCode, fShaderCode);

    init();

    glutMainLoop();
    return 0;
}
