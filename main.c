/*******************************************************************
		   CPS 511 Assignment 2
		   Michael Gorokhovsky 
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"
#include "CubeMesh.h"

const int meshSize = 100;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

static int numBuildings = 0;
static double camXPos = 0.0;
static double camYPos = 6.0;
int prevMouseXPos, prevMouseYPos;
static CubeMesh cube;
//static Building building;
bool transl = false;
bool scaleY = false;
bool scaleXZ = false;
bool extrusion = false;
bool polygonScaleMode = false;
bool justChanged = false;
static double builPosX = 0;
static double builPosZ = 0.0;


static double builScaleX = 1.0;
double builScaleXArchive[100];
static double builScaleY = 1.0;
double builScaleYArchive[100];
static double builScaleZ = 1.0;
double builScaleZArchive[100];


#define FLOORHEIGHT 4;
int numfloors[100];
float scaleXValL[100][100];
float scaleXValR[100][100];
float scaleZValT[100][100];
float scaleZValB[100][100];

static int numQuadBuildings = 0;
int floorBeingScaled = 0;
int numFinalBuildings = 0;

// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat drone_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat drone_mat_shininess[] = { 0.0F };


//Building after you are done editing
static GLfloat buil_mat_ambient[] = { 0.0F, 0.05F, 0.0F, 1.0F };
static GLfloat buil_mat_specular[] = { 0.0F, 0.0F, 0.004F, 1.0F };
static GLfloat buil_mat_diffuse[] = { 0.5F, 0.5F, 0.5F, 1.0F };
static GLfloat buil_shininess[] = { 0.0F };

// A quad mesh representing the ground
static QuadMesh groundMesh;

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
Vector3D ScreenToWorld(int x, int y);


int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(-50.0f, 0.0f, 50.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 100.0, 100.0, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);
}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
float leftSideX[100];
float rightSideX[100];
float topSideZ[100];
float bottomSideZ[100];
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camXPos, camYPos + 50, 100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// Draw Drone

	// Set drone material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);


	if (numBuildings > 0) {
		glPushMatrix();
		
		glTranslatef(builPosX,0,builPosZ);
		glScalef(builScaleX, builScaleY, builScaleZ);
		glTranslatef(0,1,0);
		cube = newCube();
		drawCube(&cube);
		glPopMatrix();
	}

	numfloors[0] = builScaleY / FLOORHEIGHT;
	int floorHeight = 2 * FLOORHEIGHT;
	
	leftSideX[0] = builPosX - (builScaleX);
	
	rightSideX[0]= builPosX + 2 * builScaleX - (builScaleX);
	
	topSideZ[0] = builPosZ + 2 * builScaleZ - (builScaleZ);
	
	bottomSideZ[0] = builPosZ - (builScaleZ);

	if (numQuadBuildings > 0) {
		

		

		//making the base quad polygon
		//glPushMatrix();
		//glBegin(GL_QUADS);	// Draw A Quad
		//		glVertex3f(leftSideX[0], 0, topSideZ[0]);	// Top Left
		//		glVertex3f(rightSideX[0], 0, topSideZ[0]);	// Top Right
		//		glVertex3f(rightSideX[0], 0, bottomSideZ[0]);	// Bottom Right
		//		glVertex3f(leftSideX[0], 0, bottomSideZ[0]);	// Bottom Left
		//		glEnd();
		//glPopMatrix();
	
		if (numfloors[0] == 0) {
			printf("Not enough height for any floors! \n");
		}
		else {
			for (int i = 1; i <= numfloors[0]; i++) {
				//Making a platform one floor up
				glPushMatrix();
				glBegin(GL_QUADS);	// Draw A Quad
				glNormal3f(0.0, 1.0, 0.0);
				glVertex3f(leftSideX[0] + scaleXValL[0][i], floorHeight * i, topSideZ[0] + scaleZValT[0][i]);	// Top Left
				glVertex3f(rightSideX[0] + scaleXValR[0][i], floorHeight * i, topSideZ[0] + scaleZValT[0][i]);	// Top Right
				glVertex3f(rightSideX[0] + scaleXValR[0][i], floorHeight * i, bottomSideZ[0] + scaleZValB[0][i]);	// Bottom Right
				glVertex3f(leftSideX[0] + scaleXValL[0][i], floorHeight * i, bottomSideZ[0] + scaleZValB[0][i]);	// Bottom Left
				glEnd();
				glPopMatrix();
				//Now we need 4 walls
					//wall1 (left)
				glPushMatrix();
				glBegin(GL_QUADS);	// Draw A Quad
				glNormal3f(-1.0, 0.0, 0.0);
				glVertex3f(leftSideX[0] + scaleXValL[0][i], floorHeight * i, bottomSideZ[0] + scaleZValB[0][i]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i - 1], floorHeight * (i - 1), bottomSideZ[0] + scaleZValB[0][i - 1]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i-1], floorHeight * (i - 1), topSideZ[0] + scaleZValT[0][i-1]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i], floorHeight * i, topSideZ[0] + scaleZValT[0][i]);
				glEnd();
				glPopMatrix();
				//wall2 (front)
				glPushMatrix();
				glBegin(GL_QUADS);	// Draw A Quad
				glNormal3f(0.0, 0.0, 1.0);
				glVertex3f(rightSideX[0] + scaleXValR[0][i], floorHeight * i, topSideZ[0] + scaleZValT[0][i]);
				glVertex3f(rightSideX[0] + scaleXValR[0][i-1], floorHeight * (i - 1), topSideZ[0] + scaleZValT[0][i-1]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i-1], floorHeight * (i - 1), topSideZ[0] + scaleZValT[0][i-1]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i], floorHeight * i, topSideZ[0] + scaleZValT[0][i]);
				glEnd();
				glPopMatrix();
				//wall3 (back)
				glPushMatrix();
				glBegin(GL_QUADS);	// Draw A Quad
				glNormal3f(0.0, 0.0, -1.0);
				glVertex3f(rightSideX[0] + scaleXValR[0][i], floorHeight * i, bottomSideZ[0] + scaleZValB[0][i]);
				glVertex3f(rightSideX[0] + scaleXValR[0][i - 1], floorHeight * (i - 1), bottomSideZ[0] + scaleZValB[0][i - 1]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i-1], floorHeight * (i - 1), bottomSideZ[0] + scaleZValB[0][i-1]);
				glVertex3f(leftSideX[0] + scaleXValL[0][i], floorHeight * i, bottomSideZ[0] + scaleZValB[0][i]);
				glEnd();
				glPopMatrix();
				//wall4(right)
				glPushMatrix();
				glBegin(GL_QUADS);	// Draw A Quad
				glNormal3f(1.0, 0.0, 0.0);
				glVertex3f(rightSideX[0] + scaleXValR[0][i], floorHeight * i, bottomSideZ[0] + scaleZValB[0][i]);
				glVertex3f(rightSideX[0] + scaleXValR[0][i - 1], floorHeight * (i - 1), bottomSideZ[0] + scaleZValB[0][i - 1]);
				glVertex3f(rightSideX[0] + scaleXValR[0][i-1], floorHeight * (i - 1), topSideZ[0] + scaleZValT[0][i-1]);
				glVertex3f(rightSideX[0] + scaleXValR[0][i], floorHeight * i, topSideZ[0] + scaleZValT[0][i]);
				glEnd();
				glPopMatrix();
			}
			}
		
		}

	if (numFinalBuildings == 1 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[1][i] = scaleXValL[0][i];
			scaleXValR[1][i] = scaleXValR[0][i];
			scaleZValT[1][i] = scaleZValT[0][i];
			scaleZValB[1][i] = scaleZValB[0][i];
		}
		numfloors[1] = numfloors[0];
		leftSideX[1] = leftSideX[0];
		rightSideX[1] = rightSideX[0];
		topSideZ[1] = topSideZ[0];
		bottomSideZ[1] = bottomSideZ[0];
	}
	else if (numFinalBuildings == 2 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[2][i] = scaleXValL[0][i];
			scaleXValR[2][i] = scaleXValR[0][i];
			scaleZValT[2][i] = scaleZValT[0][i];
			scaleZValB[2][i] = scaleZValB[0][i];
		}
		numfloors[2] = numfloors[0];
		leftSideX[2] = leftSideX[0];
		rightSideX[2] = rightSideX[0];
		topSideZ[2] = topSideZ[0];
		bottomSideZ[2] = bottomSideZ[0];
	}	else if (numFinalBuildings == 3 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[3][i] = scaleXValL[0][i];
			scaleXValR[3][i] = scaleXValR[0][i];
			scaleZValT[3][i] = scaleZValT[0][i];
			scaleZValB[3][i] = scaleZValB[0][i];
		}
		numfloors[3] = numfloors[0];
		leftSideX[3] = leftSideX[0];
		rightSideX[3] = rightSideX[0];
		topSideZ[3] = topSideZ[0];
		bottomSideZ[3] = bottomSideZ[0];
	}	else if (numFinalBuildings == 4 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[4][i] = scaleXValL[0][i];
			scaleXValR[4][i] = scaleXValR[0][i];
			scaleZValT[4][i] = scaleZValT[0][i];
			scaleZValB[4][i] = scaleZValB[0][i];
		}
		numfloors[4] = numfloors[0];
		leftSideX[4] = leftSideX[0];
		rightSideX[4] = rightSideX[0];
		topSideZ[4] = topSideZ[0];
		bottomSideZ[4] = bottomSideZ[0];
	}	else if (numFinalBuildings == 5 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[5][i] = scaleXValL[0][i];
			scaleXValR[5][i] = scaleXValR[0][i];
			scaleZValT[5][i] = scaleZValT[0][i];
			scaleZValB[5][i] = scaleZValB[0][i];
		}
		numfloors[5] = numfloors[0];
		leftSideX[5] = leftSideX[0];
		rightSideX[5] = rightSideX[0];
		topSideZ[5] = topSideZ[0];
		bottomSideZ[5] = bottomSideZ[0];
	}	else if (numFinalBuildings == 6 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[6][i] = scaleXValL[0][i];
			scaleXValR[6][i] = scaleXValR[0][i];
			scaleZValT[6][i] = scaleZValT[0][i];
			scaleZValB[6][i] = scaleZValB[0][i];
		}
		numfloors[6] = numfloors[0];
		leftSideX[6] = leftSideX[0];
		rightSideX[6] = rightSideX[0];
		topSideZ[6] = topSideZ[0];
		bottomSideZ[6] = bottomSideZ[0];
	}	else if (numFinalBuildings == 7 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[7][i] = scaleXValL[0][i];
			scaleXValR[7][i] = scaleXValR[0][i];
			scaleZValT[7][i] = scaleZValT[0][i];
			scaleZValB[7][i] = scaleZValB[0][i];
		}
		numfloors[7] = numfloors[0];
		leftSideX[7] = leftSideX[0];
		rightSideX[7] = rightSideX[0];
		topSideZ[7] = topSideZ[0];
		bottomSideZ[7] = bottomSideZ[0];
	}	else if (numFinalBuildings == 8 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[8][i] = scaleXValL[0][i];
			scaleXValR[8][i] = scaleXValR[0][i];
			scaleZValT[8][i] = scaleZValT[0][i];
			scaleZValB[8][i] = scaleZValB[0][i];
		}
		numfloors[8] = numfloors[0];
		leftSideX[8] = leftSideX[0];
		rightSideX[8] = rightSideX[0];
		topSideZ[8] = topSideZ[0];
		bottomSideZ[8] = bottomSideZ[0];
	}	else if (numFinalBuildings == 9 && justChanged == true) {
		for (int i = 0; i < 10; i++) {
			scaleXValL[9][i] = scaleXValL[0][i];
			scaleXValR[9][i] = scaleXValR[0][i];
			scaleZValT[9][i] = scaleZValT[0][i];
			scaleZValB[9][i] = scaleZValB[0][i];
		}
		numfloors[9] = numfloors[0];
		leftSideX[9] = leftSideX[0];
		rightSideX[9] = rightSideX[0];
		topSideZ[9] = topSideZ[0];
		bottomSideZ[9] = bottomSideZ[0];
	}
	else if (numFinalBuildings == 10 && justChanged == true) {
	for (int i = 0; i < 10; i++) {
		scaleXValL[10][i] = scaleXValL[0][i];
		scaleXValR[10][i] = scaleXValR[0][i];
		scaleZValT[10][i] = scaleZValT[0][i];
		scaleZValB[10][i] = scaleZValB[0][i];
	}
	numfloors[10] = numfloors[0];
	leftSideX[10] = leftSideX[0];
	rightSideX[10] = rightSideX[0];
	topSideZ[10] = topSideZ[0];
	bottomSideZ[10] = bottomSideZ[0];
	}

	glMaterialfv(GL_FRONT, GL_AMBIENT, buil_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, buil_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, buil_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, buil_shininess);
	for (int j = 1; j <= numFinalBuildings; j++) {
		for (int i = 1; i <= numfloors[j]; i++) {

			//Making a platform one floor up
			glPushMatrix();
			glBegin(GL_QUADS);	// Draw A Quad
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(leftSideX[j] + scaleXValL[j][i], floorHeight * i, topSideZ[j] + scaleZValT[j][i]);	// Top Left
			glVertex3f(rightSideX[j] + scaleXValR[j][i], floorHeight * i, topSideZ[j] + scaleZValT[j][i]);	// Top Right
			glVertex3f(rightSideX[j] + scaleXValR[j][i], floorHeight * i, bottomSideZ[j] + scaleZValB[j][i]);	// Bottom Right
			glVertex3f(leftSideX[j] + scaleXValL[j][i], floorHeight * i, bottomSideZ[j] + scaleZValB[j][i]);	// Bottom Left
			glEnd();
			glPopMatrix();
			//Now we need 4 walls
				//wall1 (left)
			glPushMatrix();
			glBegin(GL_QUADS);	// Draw A Quad
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(leftSideX[j] + scaleXValL[j][i], floorHeight * i, bottomSideZ[j] + scaleZValB[j][i]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i - 1], floorHeight * (i - 1), bottomSideZ[j] + scaleZValB[j][i - 1]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i - 1], floorHeight * (i - 1), topSideZ[j] + scaleZValT[j][i - 1]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i], floorHeight * i, topSideZ[j] + scaleZValT[j][i]);
			glEnd();
			glPopMatrix();
			//wall2 (front)
			glPushMatrix();
			glBegin(GL_QUADS);	// Draw A Quad
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(rightSideX[j] + scaleXValR[j][i], floorHeight * i, topSideZ[j] + scaleZValT[j][i]);
			glVertex3f(rightSideX[j] + scaleXValR[j][i - 1], floorHeight * (i - 1), topSideZ[j] + scaleZValT[j][i - 1]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i - 1], floorHeight * (i - 1), topSideZ[j] + scaleZValT[j][i - 1]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i], floorHeight * i, topSideZ[j] + scaleZValT[j][i]);
			glEnd();
			glPopMatrix();
			//wall3 (back)
			glPushMatrix();
			glBegin(GL_QUADS);	// Draw A Quad
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(rightSideX[j] + scaleXValR[j][i], floorHeight * i, bottomSideZ[j] + scaleZValB[j][i]);
			glVertex3f(rightSideX[j] + scaleXValR[j][i - 1], floorHeight * (i - 1), bottomSideZ[j] + scaleZValB[j][i - 1]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i - 1], floorHeight * (i - 1), bottomSideZ[j] + scaleZValB[j][i - 1]);
			glVertex3f(leftSideX[j] + scaleXValL[j][i], floorHeight * i, bottomSideZ[j] + scaleZValB[j][i]);
			glEnd();
			glPopMatrix();
			//wall4(right)
			glPushMatrix();
			glBegin(GL_QUADS);	// Draw A Quad
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(rightSideX[j] + scaleXValR[j][i], floorHeight * i, bottomSideZ[j] + scaleZValB[j][i]);
			glVertex3f(rightSideX[j] + scaleXValR[j][i - 1], floorHeight * (i - 1), bottomSideZ[j] + scaleZValB[j][i - 1]);
			glVertex3f(rightSideX[j] + scaleXValR[j][i - 1], floorHeight * (i - 1), topSideZ[j] + scaleZValT[j][i - 1]);
			glVertex3f(rightSideX[j] + scaleXValR[j][i], floorHeight * i, topSideZ[j] + scaleZValT[j][i]);
			glEnd();
			glPopMatrix();
		}
		justChanged = false;
	}

	
	
	// Draw ground mesh
	DrawMeshQM(&groundMesh, meshSize);

	glutSwapBuffers();   // Double buffering, swap buffers
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 400.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis

}

void makeBuilding() {
	if (numBuildings == 0 && numQuadBuildings == 0 && builScaleY == 1)
	{
		numBuildings = 1;

	}
	
	//glutPostRedisplay();

}
void makeQuadBuilding() {
	if (numQuadBuildings == 0 && numBuildings == 1) {
		numBuildings = 0;
		numQuadBuildings = 1;
	}
}
void makeFinalBuilding() {
	if (numQuadBuildings == 1) {
		numQuadBuildings = 0;
		numFinalBuildings += 1;
	}
}
void resetBuilding() {
	numBuildings = 0;
	numQuadBuildings = 0;
	extrusion = false;
	transl = false;
	scaleY = false;
	scaleXZ = false;
	polygonScaleMode = false;
	builPosX = 0;
	builPosZ = 0.0;
	numfloors[0] = 0;
	for (int i = 0; i < 10; i++) {
		scaleXValL[0][i] = 0;
		scaleXValR[0][i] = 0;
		scaleZValT[0][i] = 0;
		scaleZValB[0][i] = 0;
	}
	builScaleX = 1;
	builScaleY = 1;
	builScaleZ = 1;
}
// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't':
		if (extrusion != true) {
			transl = true;
			scaleY = false;
			scaleXZ = false;
		}
		
		break;

	case 's':
		if (extrusion != true) {
			scaleXZ = true;
			scaleY = false;
			transl = false;
		}
		
		break;

	case 'h':
		if (extrusion != true) {
			scaleY = true;
			transl = false;
		}
		
		break;

	case 'e':

		extrusion = true;
		transl = false;
		scaleY = false;
		scaleXZ = false;
		makeQuadBuilding();
		break;

	case 'i':
		if (extrusion == true) {
			polygonScaleMode = true;
			printf("Please select which floor you want to scale with your number keys. \n");
			printf("Select '1' to scale floor 1, '2' for floor 2 and so on \n");
			printf("Once you have selected a floor, use the arrow keys to scale it \n");

		}
		break;

	case '0':
		floorBeingScaled = 0;
		break;
	case '1':
		
		floorBeingScaled = 1;
		break;

	case '2':
		floorBeingScaled = 2;
		break;

	case '3':
		floorBeingScaled = 3;
		break;
	case '4':
		floorBeingScaled = 4;
		break;
	case '5':
		floorBeingScaled = 5;
		break;
	case '6':
		floorBeingScaled = 6;
		break;
	case '7':
		floorBeingScaled = 7;
		break;
	case '8':
		floorBeingScaled = 8;
		break;
	case '9':
		floorBeingScaled = 9;
		break;

	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{
		makeBuilding();
	}
	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}
	else if (key == GLUT_KEY_F2) {
		if (extrusion == true) {
			justChanged = true;
			makeFinalBuilding();
		}
	}
	else if (key == GLUT_KEY_F3) {
		resetBuilding();
	}
	else if (key == GLUT_KEY_LEFT)
	{
		if (transl == true) {
			builPosX -= 1;
		}
		else if (scaleXZ == true) {
			if (builScaleX > 0.5)
			{
				builScaleX -= 0.5;
			}
		}
		else if (polygonScaleMode == true) {
			scaleXValL[0][floorBeingScaled] += 0.3;
			scaleXValR[0][floorBeingScaled] -= 0.3;
		}
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		if (transl == true) {
			builPosX += 1;
		}
		else if (scaleXZ == true) {
			builScaleX += 0.5;
		}
		else if (polygonScaleMode == true) {
			scaleXValL[0][floorBeingScaled] -= 0.3;
			scaleXValR[0][floorBeingScaled] += 0.3;
		}
	}
	else if (key == GLUT_KEY_UP)
	{
		if (transl == true) {
			builPosZ -= 1;
		}
		else if (scaleY == true) {
			builScaleY += 0.5;
		}
		else if (scaleXZ == true) {
			builScaleZ += 0.5;
		}
		else if (polygonScaleMode == true) {
			scaleZValT[0][floorBeingScaled] -= 0.3;
			scaleZValB[0][floorBeingScaled] += 0.3;
		}
	}
	else if (key == GLUT_KEY_DOWN)
	{
		if (transl == true) {
			builPosZ += 1;
		}
		else if (scaleY == true) {
			if (builScaleY > 0.5)
			{
				builScaleY -= 0.5;
			}
		}
		else if (scaleXZ == true) {
			if (builScaleZ > 0.5)
			{
				builScaleZ -= 0.5;
			}
		}
		else if (polygonScaleMode == true) {
			scaleZValT[0][floorBeingScaled] += 0.3;
			scaleZValB[0][floorBeingScaled] -= 0.3;
		}
	}
	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	POINT mouse;
	GetCursorPos(&mouse);
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		if (prevMouseXPos < xMouse) {
			camXPos -= 0.5;
			if (camXPos < -56) {
				camXPos = -56;
			}
		}
		else if (prevMouseXPos > xMouse ){
			camXPos += 0.5;
			if (camXPos > 75) {
				camXPos = 75;
			}
		}
		else if (prevMouseYPos < yMouse) {
			camYPos -= 0.5;
			if (camYPos < -49.5) {
				camYPos = -49.5;
			}
		}
		else if (prevMouseYPos > yMouse) {
			camYPos += 0.5;
			if (camYPos > 33) {
				camYPos = 33;
			}
		}

	}
	prevMouseXPos = xMouse;
	prevMouseYPos = yMouse;
	glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
	// you will need to finish this if you use the mouse
	return NewVector3D(0, 0, 0);
}



