#include <stdlib.h>

#define _USE_MATH_DEFINES

#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>
#include <iostream>

#endif

#define TESSELLATION 100

float camX = 0, camY, camZ = 5;
int startX, startY, tracking = 0;

int alpha = 0, beta = 0, r = 5;

#define POINT_COUNT 6
// Points that make up the loop for catmull-rom interpolation
float p[POINT_COUNT][3] = {
        {-1, -1, 0},
        {-1, 1, 0},
        {1, 1, 0},
        {0, 0, 0},
        {1, -1, 0},
        {-1, -1.5f, 0}
};

void buildRotMatrix(float *x, float *y, float *z, float *m) {
    m[0] = x[0];
    m[1] = x[1];
    m[2] = x[2];
    m[3] = 0;
    m[4] = y[0];
    m[5] = y[1];
    m[6] = y[2];
    m[7] = 0;
    m[8] = z[0];
    m[9] = z[1];
    m[10] = z[2];
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}


void cross(float *a, float *b, float *res) {
    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
}


void normalize(float *a) {
    float l = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0] / l;
    a[1] = a[1] / l;
    a[2] = a[2] / l;
}


float length(float *v) {
    float res = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return res;
}

void multMatrixVector(const float m[4][4], const float *v, float *res) {
    for (int j = 0; j < 4; ++j) {
        res[j] = 0;
        for (int k = 0; k < 4; ++k) {
            res[j] += v[k] * m[j][k];
        }
    }
}


void getCatmullRomPoint(float t, const float *p0, const float *p1, const float *p2, const float *p3, float *pos,
                        float *deriv) {
    // catmull-rom matrix
    const float m[4][4] = {{-0.5f, 1.5f,  -1.5f, 0.5f},
                           {1.0f,  -2.5f, 2.0f,  -0.5f},
                           {-0.5f, 0.0f,  0.5f,  0.0f},
                           {0.0f,  1.0f,  0.0f,  0.0f}};

    // T vector
    float T[4] = {powf(t, 3), powf(t, 2), t, 1};

    // T' vector
    float Tl[4] = {powf(t, 3), powf(t, 2), t, 1};

    for (int i = 0; i < 3; i++) {
        // ----- Compute A = M * P
        float A[4];
        const float Pi[4] = {p0[i], p1[i], p2[i], p3[i]}; // Vector P for the given component

        multMatrixVector(m, Pi, A);

        // Compute pos = T * A
        pos[i] = T[0] * A[0] + T[1] * A[1] + T[2] * A[2] + T[3] * A[3];

        // compute deriv = T' * A
        deriv[i] = Tl[0] * A[0] + Tl[1] * A[1] + Tl[2] * A[2] + Tl[3] * A[3];


        //std::cout << "i=" << i << " pos=" << pos[i] << " deriv=" << deriv[i] << std::endl;
    }
}


// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv) {

    float t = gt * POINT_COUNT; // this is the real global t
    int index = floor(t);  // which segment
    t = t - (float) index; // where within  the segment

    // indices store the points
    int indices[4];
    indices[0] = (index + POINT_COUNT - 1) % POINT_COUNT;
    indices[1] = (indices[0] + 1) % POINT_COUNT;
    indices[2] = (indices[1] + 1) % POINT_COUNT;
    indices[3] = (indices[2] + 1) % POINT_COUNT;

    getCatmullRomPoint(t, p[indices[0]], p[indices[1]], p[indices[2]], p[indices[3]], pos, deriv);
}


void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if (h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = w * 1.0 / h;

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


void renderCatmullRomCurve() {
    glBegin(GL_LINE_LOOP);

    float *pos;
    float *deriv;
    pos = (float *) (calloc(3, sizeof(float))); // pos is a point so it has 3 coordinates
    deriv = (float *) (calloc(3, sizeof(float))); // deriv is a point so it has 3 coordinates

    float gt = 0;
    while (gt < 1) {
        getGlobalCatmullRomPoint(gt, pos, deriv);
        glVertex3f(pos[0], pos[1], pos[2]);

        gt += 1.f / TESSELLATION;
    }

    free(pos); // TODO: Free of the pos dynamic allocated memory

    glEnd();
}


void renderScene() {
    static float t = 0;
    static auto *pos = (float *) (calloc(3, sizeof(float))); // pos is a point so it has 3 coordinates
    static auto *deriv = (float *) (calloc(3, sizeof(float))); // deriv is a point so it has 3 coordinates

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
              0.0, 0.0, 0.0,
              0.0f, 1.0f, 0.0f);

    renderCatmullRomCurve();

    // apply transformations here
    getGlobalCatmullRomPoint(t, pos, deriv);
    glTranslatef(pos[0], pos[1], pos[2]);

    glutWireTeapot(0.1);

    glutSwapBuffers();
    t += 1.f / TESSELLATION;
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
    camX = rAux * sinf(alphaAux * 3.14 / 180.0) * cosf(betaAux * 3.14 / 180.0);
    camZ = rAux * cosf(alphaAux * 3.14 / 180.0) * cosf(betaAux * 3.14 / 180.0);
    camY = rAux * sinf(betaAux * 3.14 / 180.0);
}


int main(int argc, char **argv) {

// inicialization
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(320, 320);
    glutCreateWindow("CG@DI-UM");

// callback registration 
    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);

// mouse callbacks
    glutMouseFunc(processMouseButtons);
    glutMotionFunc(processMouseMotion);

// OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

// enter GLUT's main cycle 
    glutMainLoop();

    return 1;
}

