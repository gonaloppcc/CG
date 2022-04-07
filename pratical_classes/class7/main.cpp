

#include <iostream>
#include <cstdlib>

#define _USE_MATH_DEFINES

#include <cmath>
#include <vector>
#include <tuple>

#include <IL/il.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glew.h>
#include <GL/glut.h>

#endif

using namespace std;


/* ------------------------------------------------------------------------------------------------------------------ */
// Structures definitions

struct Point {
    float x;
    float y;
    float z;
};

/* ------------------------------------------------------------------------------------------------------------------ */

/* Camera stuff */
int startX, startY, tracking = 0;

int alpha = 0, beta = 45, r = 50;

Point L = {0.f, 0.f, 1.f};

float speed = 1;

float camX = 0, camY = 30, camZ = 40;

/* ------------------------------------------------------------------------------------------------------------------ */

// VBO stuff

GLuint *fieldVertices; // Vertices of the image to drawn that are going to be part of the field.

// Trees stuff

unsigned int seed = 0;
vector<tuple<float, float>> trees; // Position of the trees generated.

const int baseRadius = 2;
const int height = 5;
const int slices = 10;
const int stacks = 10;

// Image stuff

unsigned int t, tw, th;
unsigned char *imageData;

float max_height = 30; // Max height of a point of the image

int halfW;
int halfH;

/* ------------------------------------------------------------------------------------------------------------------ */

const float glade_radius = 50; // Center of the forest which trees can't be drawn

/* ------------------------------------------------------------------------------------------------------------------ */

float get_rand_pos() {
    return ((((float) rand() / (float) RAND_MAX)) * 255); // TODO: Put in the middle of the referential
}

float get_rand_colour() {
    return (((float) (rand() / ((float) RAND_MAX)))) / 5;
}

void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you can't make a window with zero width).
    if (h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = ((((float) w) * 1.0f)) / (float) h;

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the correct perspective
    gluPerspective(45, ratio, 1, 1000);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

/**
 * Returns the height of a grid cell based on the image.
 */
float h(int i, int j) {
    float height = imageData[i * tw + j];
    return height / 255.0f * max_height;
}

/**
 * Interpolation of the height in an arbitrary point in terrain.
 */
float hf(float x, float z) {
    // Points needed
    int x1 = floor(x);
    int x2 = x1 + 1;
    int z1 = floor(z);
    int z2 = z1 + 1;

    // Interpolation
    float fx = x - (float) x1; // 0 <= fx <= 1
    float fz = z - (float) z1; // 0 <= fz <= 1

    float h_x1_z = h(x1, z1) * (1 - fz) + h(x1, z2) * fz;
    float h_x2_z = h(x2, z1) * (1 - fz) + h(x2, z2) * fz;

    return h_x1_z * (1 - fx) + h_x2_z * fx; // Height of the point
}

// Initializing the terrain and trees

void initTerrain() {
    glEnableClientState(GL_VERTEX_ARRAY);

    fieldVertices = (GLuint *) malloc(sizeof(GLuint) * th);

    glGenBuffers((int) th, fieldVertices);

    halfW = tw / 2;
    halfH = th / 2;

    for (int n_strip = 0; n_strip < th; n_strip++) {
        vector<double> strip;

        for (int j = 0; j < tw; j++) {
            strip.push_back(n_strip - halfW);
            strip.push_back(h(n_strip, j));
            strip.push_back(j - halfH);

            strip.push_back(n_strip + 1 - halfW);
            strip.push_back(h(n_strip, j));
            strip.push_back(j - halfH);
        }

        glBindBuffer(GL_ARRAY_BUFFER, fieldVertices[n_strip]);

        glBufferData(GL_ARRAY_BUFFER, (long) (strip.size() * sizeof(double)), strip.data(), GL_STATIC_DRAW);
    }

}


void generateTrees(int n) {
    srand(seed);

    for (int i = 0; i < n; i++) {
        // Position
        float x = get_rand_pos();
        float y = get_rand_pos();

        // TODO: Restrict the position of the trees

        // cout << "Tree n: " << i << " x=" << x << " y=" << y << endl;
        trees.emplace_back(x, y);
    }

}

// Drawing the terrain and trees

void drawTerrain() {
    glColor3f(1, 1, 1);
    for (int n_strip = 0; n_strip < th; n_strip++) {
        glBindBuffer(GL_ARRAY_BUFFER, fieldVertices[n_strip]);
        glVertexPointer(3, GL_DOUBLE, 0, nullptr);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, (int) (2u * tw));
    }
}


void drawTree() {
    // Downside of the tree
    glColor3f(0.6f, 0.4f, 0);

    glutSolidCone(((float) baseRadius) / 10, height, slices, stacks);

    // Upper side of the tree
    glTranslatef(0, 0, 2);

    glColor3f(0, 0.5f + get_rand_colour(), 0);

    glutSolidCone(baseRadius, height, slices, stacks);

    glColor3f(0, 0, 0);
}

void drawTrees() {
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);

    for (auto tree: trees) {
        float x = get<0>(tree);
        float z = get<1>(tree);

        glPushMatrix();

        glTranslatef(x - halfW, -z + halfH, hf(x, z)); // Because of the upper rotate

        drawTree();

        glPopMatrix();
    }
    glPopMatrix();
}

void renderScene() {

    float pos[4] = {-1.0, 1.0, 1.0, 0.0};

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(camX, hf(camX + halfW, camZ - halfH), camZ,
              camX + L.x, 0, camZ + L.z,
              0.0f, 1.0f, 0.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    drawTerrain();
    drawTrees();

// End of frame
    glutSwapBuffers();
}



// Begin of Input stuff

void processKeys(unsigned char key, int xx, int yy) {
    switch (key) {
        case 'W':
        case 'w':
            camX += L.x * speed;
            camZ += L.z * speed;
            cout << "w pressed!" << endl;
            break;

        case 'D':
        case 'd':
            camX += L.z * speed;
            camZ -= L.x * speed;
            break;
        case 'A':
        case 'a':
            camX += L.z * speed;
            camZ -= L.x * speed;
            break;

        case 'S':
        case 's':
            camX -= L.x * speed;
            camZ -= L.z * speed;
            cout << "s pressed!" << endl;
            break;
    }
    // put code to process regular keys in here
}

void processSpecialKeys(int key, int xx, int yy) {
    switch (key) {
        case GLUT_KEY_UP:
            camX += L.x * speed;
            camY += L.y * speed;
            camZ += L.z * speed;
            break;

    }
}

void processMouseButtons(int button, int state, int xx, int yy) {

    if (state == GLUT_DOWN) {
        startX = xx;
        startY = yy;
        if (button == GLUT_LEFT_BUTTON)
            tracking = 1;
        else if (button == GLUT_RIGHT_BUTTON)
            tracking = 2;
        else
            tracking = 0;
    } else if (state == GLUT_UP) {
        if (tracking == 1) {
            alpha += (xx - startX);
            beta += (yy - startY);
        } else if (tracking == 2) {

            r -= yy - startY;
            if (r < 3)
                r = 3.0;
        }
        tracking = 0;
    }
}


void processMouseMotion(int xx, int yy) {

    int deltaX, deltaY;
    int alphaAux, betaAux;
    int rAux;

    if (!tracking)
        return;

    deltaX = xx - startX;
    deltaY = yy - startY;

    if (tracking == 1) {


        alphaAux = alpha + deltaX;
        betaAux = beta + deltaY;

        if (betaAux > 85.0)
            betaAux = 85.0;
        else if (betaAux < -85.0)
            betaAux = -85.0;

        rAux = r;
    } else if (tracking == 2) {

        alphaAux = alpha;
        betaAux = beta;
        rAux = r - deltaY;
        if (rAux < 3)
            rAux = 3;
    }
    camX = rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
    camZ = rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
    //camY = rAux * sin(betaAux * 3.14 / 180.0);
}

// End of Input stuff



void init() {

    // Initialize glew
    glewInit();

    // 	Loading the height map of the file "terreno.jpg"
    ilInit();
    ilGenImages(1, &t);
    ilBindImage(t);
    // terreno.jpg is the image containing our height map
    ilLoadImage((ILstring) "terreno.jpg");
    // convert the image to single channel per pixel
    // with values ranging between 0 and 255
    ilConvertImage(IL_LUMINANCE, IL_UNSIGNED_BYTE);

    // important: check tw and th values
    // both should be equal to 256
    // if not there was an error loading the image
    // most likely the image could not be found
    tw = ilGetInteger(IL_IMAGE_WIDTH);
    th = ilGetInteger(IL_IMAGE_HEIGHT);
    // imageData is a LINEAR array with the pixel values
    imageData = ilGetData();

// 	Build the vertex arrays
    initTerrain();
    generateTrees(50);

// 	OpenGL settings
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
}


int main(int argc, char **argv) {

// init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CG@DI-UM");


// Required callback registry 
    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);

// Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);
    glutMouseFunc(processMouseButtons);
    glutMotionFunc(processMouseMotion);

    init();

// enter GLUT's main cycle
    glutMainLoop();

    return 0;
}

