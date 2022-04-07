#include <cstdio>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>

#endif

#define _USE_MATH_DEFINES

#include <math.h>

float alfa = 0.0f, beta = 0.5f, radius = 200.0f;
float camX, camY, camZ;

const int seed = 0;

const float glade_radius = 50;

const float indianRadius = 15;

const float cowboyRadius = 40;

void spherical2Cartesian() {

    camX = radius * cos(beta) * sin(alfa);
    camY = radius * sin(beta);
    camZ = radius * cos(beta) * cos(alfa);
}


void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if (h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = w * 1.0 / h;

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);
    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

float get_rand_pos() {
    return ((((float) rand() / (float) RAND_MAX)) * 200) - 100;
}

float get_rand_colour() {
    return (((float) (rand() / ((float) RAND_MAX)))) / 5;
}

void drawBonfire() {
    glColor3f(0.9686f, 0.2157f, 0.0941f);

    //glutSolidTorus(10, 2, 5, 10);

    glColor3f(0, 0, 0);
}

void drawCowboy() {
    glColor3f(1.f, 0.f, 0.f);

    glTranslatef(cowboyRadius, 0, 0);

    glutSolidTeapot(5);

    glColor3f(0.f, 0.f, 0.f);
}

void drawCowboys(int n) {
    glPushMatrix();

    float angle = 360.f / (float) n;
    for (int i = 0; i < n; i++) {
        glPushMatrix();

        glRotatef(angle * (float) i, 0, 1, 0);
        drawCowboy();

        glPopMatrix();
    }

    glPopMatrix();
}

void drawIndian() {
    glColor3f(0.f, 0.f, .8f);

    //glRotatef(180, 0, 1, 0);

    glTranslatef(indianRadius, 0, 0);

    glutSolidTeapot(5);

    glColor3f(0, 0, 0);
}

void drawIndians(int n) {
    glPushMatrix();

    float angle = 360.f / (float) n;
    for (int i = 0; i < n; i++) {
        glPushMatrix();

        glRotatef(angle * (float) i + 0, 0, 1, 0);
        drawIndian();

        glPopMatrix();
    }

    glPopMatrix();
}

void drawTree() {
    const int baseRadius = 5;
    const int height = 20;
    const int slices = 20;
    const int stacks = 20;

    // Downside of the tree
    glColor3f(0.6f, 0.4f, 0);

    glutSolidCone(((float) baseRadius) / 10, height, slices, stacks);

    // Upper side of the tree
    glTranslatef(0, 0, 5);

    glColor3f(0, 0.5f + get_rand_colour(), 0);

    glutSolidCone(baseRadius, height, slices, stacks);


    glColor3f(0, 0, 0);
}

void drawTrees(int n) {
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);

    for (int i = 0; i < n; i++) {
        glPushMatrix();

        float x_pos = 0;
        float y_pos = 0;

        while (pow(x_pos, 2) + pow(y_pos, 2) <= pow(glade_radius, 2)) {
            x_pos = get_rand_pos();
            y_pos = -get_rand_pos();
        }


        glTranslatef(x_pos, y_pos, 0);

        drawTree();

        glPopMatrix();
    }
    glPopMatrix();
}

void renderScene() {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              0.0, 0.0, 0.0,
              0.0f, 1.0f, 0.0f);

    glColor3f(0.2f, 0.8f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex3f(100.0f, 0, -100.0f);
    glVertex3f(-100.0f, 0, -100.0f);
    glVertex3f(-100.0f, 0, 100.0f);

    glVertex3f(100.0f, 0, -100.0f);
    glVertex3f(-100.0f, 0, 100.0f);
    glVertex3f(100.0f, 0, 100.0f);
    glEnd();
    // End of frame

    // Create the seed for the random number generator
    srand(seed);

    // Drawing scene

    drawTrees(50);

    drawIndians(8);

    drawCowboys(15);

    drawBonfire();


    glutSwapBuffers();
}


void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here

}


void processSpecialKeys(int key, int xx, int yy) {

    switch (key) {

        case GLUT_KEY_RIGHT:
            alfa -= 0.1;
            break;

        case GLUT_KEY_LEFT:
            alfa += 0.1;
            break;

        case GLUT_KEY_UP:
            beta += 0.1f;
            if (beta > 1.5f)
                beta = 1.5f;
            break;

        case GLUT_KEY_DOWN:
            beta -= 0.1f;
            if (beta < -1.5f)
                beta = -1.5f;
            break;

        case GLUT_KEY_PAGE_DOWN:
            radius -= 1.0f;
            if (radius < 1.0f)
                radius = 1.0f;
            break;

        case GLUT_KEY_PAGE_UP:
            radius += 1.0f;
            break;
    }
    spherical2Cartesian();
    glutPostRedisplay();

}

void printInfo() {

    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));

    printf("\nUse Arrows to move the camera up/down and left/right\n");
    printf("Home and End control the distance from the camera to the origin");
}


int main(int argc, char **argv) {

// init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG@DI-UM");


// Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

// Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);

//  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    spherical2Cartesian();

    printInfo();

// enter GLUT's main cycle
    glutMainLoop();

    return 1;
}
