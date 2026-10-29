#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h> // handle the window-managing operations
#else
#include <GL/freeglut.h> // handle the window-managing operations
#endif
#include <iostream>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

GLint win_width = 500,
win_hight = 500;
GLdouble PI = 3.14159265359;
int scene = 1;

// =======================================
// GLOBAL VARIABLES
// =======================================
float sunAngle = 0.0f;
float cloudX = -1.0;
float rainY = 0.0;
float girlX = 0.0f;     // position
bool walkGirl = false;  // start walking
float appleTranslateY = 0.0f;
bool appleFalling = false;
GLuint houseTexture;
GLuint roofTexture;
bool rainSoundPlaying = false;

// =======================================
// TEXTURE FUNCTIONS
// Texture Loading Function (for BMP files)
// =======================================
GLuint LoadTexture(const char* filename)
{
    GLuint texture;
    int width, height;
    unsigned char* data;

    FILE* file;
    fopen_s(&file, filename, "rb");
    if (file == NULL) {
        std::cout << "Unable to open texture file: " << filename << std::endl;
        return 0;
    }

    unsigned short bfType;
    fread(&bfType, 1, sizeof(unsigned short), file);
    if (bfType != 0x4D42) {
        std::cout << "Not a valid BMP file!" << std::endl;
        fclose(file);
        return 0;
    }

    fseek(file, 18, SEEK_SET);
    fread(&width, 4, 1, file);
    fread(&height, 4, 1, file);

    fseek(file, 54, SEEK_SET);

    int row_padded = (width * 3 + 3) & (~3);
    data = new unsigned char[row_padded * height];

    fread(data, 1, row_padded * height, file);
    fclose(file);

    // BMP stores colors as BGR → convert to RGB
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = y * row_padded + x * 3;
            std::swap(data[i], data[i + 2]); // BGR → RGB
        }
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
        0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;

    return texture;
}

// =======================================
// DRAWING PRIMITIVES
// circles, clouds, shapes
// =======================================
void drawText(float x, float y, const char* text)
{
    glColor3f(0, 0, 0);
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void drawCloud(float x, float y) {
    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++) {
        float angle = i * 2 * 3.1416 / 20;
        glVertex2f(x + 0.08 * cos(angle), y + 0.05 * sin(angle));
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++) {
        float angle = i * 2 * 3.1416 / 20;
        glVertex2f(x + 0.08 + 0.08 * cos(angle), y + 0.05 * sin(angle));
    }
    glEnd();
}

void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;
    int triangleAmount = 20; //# of triangles used to draw circle
    //GLfloat radius = 0.8f; //radius
    GLfloat twicePi = 2.0f * PI;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // center of circle
    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(
            x + (radius * cos(i * twicePi / triangleAmount)),
            y + (radius * sin(i * twicePi / triangleAmount))
        );
    }
    glEnd();
}

void drawHollowCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;
    int lineAmount = 100; //# of triangles used to draw circle
    //GLfloat radius = 0.8f; //radius
    GLfloat twicePi = 2.0f * PI;
    glBegin(GL_LINE_LOOP);
    for (i = 0; i <= lineAmount; i++) {
        glVertex2f(
            x + (radius * cos(i * twicePi / lineAmount)),
            y + (radius * sin(i * twicePi / lineAmount))
        );
    }
    glEnd();
}
void drawSmile(float x, float y, float r)
{
    glColor3f(1.0f, 0.4f, 0.6f); // pink smile

    glBegin(GL_LINE_STRIP);

    for (int i = 200; i <= 340; i++) // bottom half circle
    {
        float angle = i * PI / 180.0f;
        glVertex2f(
            x + cos(angle) * r,
            y + sin(angle) * r
        );
    }

    glEnd();
}

void drawHalfCircleFilled(float x, float y, float r)
{
    glColor3f(0.55f, 0.27f, 0.07f); // Brown (Hair color)

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // center

    for (int i = 0; i <= 180; i++) // half circle
    {
        float angle = i * PI / 180.0f;
        glVertex2f(
            x + cos(angle) * r,
            y + sin(angle) * r
        );
    }

    glEnd();
}

// =======================================
// CHARACTER DRAWING
// =======================================
void drawCharacter()

{
    glPushMatrix();
    glTranslatef(girlX, 0.0f, 0.0f);
    //Brown Hair
    glColor3f(0.55f, 0.27f, 0.07f);
    glBegin(GL_QUADS);
    glVertex2f(0.41f, -0.40f);
    glVertex2f(0.59f, -0.40f);
    glVertex2f(0.66f, -0.57f);
    glVertex2f(0.36f, -0.57f);
    glEnd();
    //Right hand
    glColor3f(1.0f, 0.85f, 0.7f);
    drawFilledCircle(0.6f, -0.64f, 0.03f);
    //Left hand
    glColor3f(1.0f, 0.85f, 0.7f);
    drawFilledCircle(0.4f, -0.64f, 0.03f);
    //shoes
    glColor3f(0.0f, 0.0f, 0.0f);
    drawFilledCircle(0.47f, -0.80f, 0.03f);
    //Right shoes
    glColor3f(0.0f, 0.0f, 0.0f);
    drawFilledCircle(0.54f, -0.80f, 0.03f);
    //body
    glColor3f(1.0f, 0.71f, 0.76f); //Pink
    glBegin(GL_QUADS);
    glVertex2f(0.44f, -0.57f);
    glVertex2f(0.56f, -0.57f);
    glVertex2f(0.67f, -0.80f);
    glVertex2f(0.36f, -0.80f);
    glEnd();
    //Right Hand (pink)
    glColor3f(1.0f, 0.71f, 0.76f);
    glBegin(GL_QUADS);
    glVertex2f(0.56f, -0.57f);
    glVertex2f(0.45f, -0.57f);
    glVertex2f(0.60f, -0.63f);
    glVertex2f(0.65f, -0.64f);
    glEnd();
    //Left Hand (pink)
    glColor3f(1.0f, 0.71f, 0.76f);
    glBegin(GL_QUADS);
    glVertex2f(0.56f, -0.57f);
    glVertex2f(0.44f, -0.57f);
    glVertex2f(0.60f, -0.61f);
    glVertex2f(0.34f, -0.64f);
    glEnd();
    // Face
    glColor3f(1.0f, 0.85f, 0.7f); //Natural Human Tone
    drawFilledCircle(0.5f, -0.49f, 0.1f);
    glLineWidth(3.0f);
    // Left Eye
    glColor3f(1.0f, 1.0f, 1.0f);
    drawFilledCircle(0.45f, -0.48f, 0.03f);
    glColor3f(0.0f, 0.0f, 0.0f);
    drawFilledCircle(0.45f, -0.48f, 0.02f);
    // Right Eye
    glColor3f(1.0f, 1.0f, 1.0f);
    drawFilledCircle(0.53f, -0.48f, 0.03f);
    glColor3f(0.0f, 0.0f, 0.0f);
    drawFilledCircle(0.53f, -0.48f, 0.02f);
    // Bangs (hair fringe)
    drawHalfCircleFilled(0.5f, -0.46f, 0.11f); //Brown
    // Pink smile
    drawSmile(0.5f, -0.52f, 0.04f);

    glPopMatrix();
}


// =======================================
// SCENE OBJECTS
// Scene - Tree - House - Apples
// =======================================
void reshapeFun(GLint newWidth, GLint newHight)
{
    glViewport(0, 0, newWidth, newHight);
    win_width = newWidth;
    win_hight = newHight;
}

// Apples
void drawApple(float x, float y) {

    glColor3f(1.0f, 0.0f, 0.0f); //Red 
    drawFilledCircle(x, y, 0.038);

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.038);
    glVertex2f(x, y + 0.055);
    glEnd();
}
// Right Leaf
void drawRLeaf(float x, float y) {

    glColor3f(0.0, 0.5, 0.0); //Green 
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 0.05, y - 0.005);
    glVertex2f(x - 0.05, y + 0.02);
    glVertex2f(x, y + 0.06);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(4.0);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x - 0.04, y - 0.04);
    glEnd();
}
// Left Leaf
void drawLLeaf(float x, float y) {

    glColor3f(0.0, 0.5, 0.0); //Green 
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 0.05, y + 0.005);
    glVertex2f(x - 0.05, y - 0.02);
    glVertex2f(x, y + 0.06);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(4.0);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x + 0.04, y - 0.04);
    glEnd();
}

// Tree
void drawTree(int stage) {
    if (stage <= 1) return;

    if (stage == 2) {
        glColor3f(0.4, 0.2, 0.0); // Tree Trunk Color (Brown Wood)
        glLineWidth(6.0);
        glBegin(GL_LINES);
        glVertex2f(-0.5, -0.8);
        glVertex2f(-0.5, -0.5);
        glEnd();

        drawRLeaf(-0.45, -0.5);
        drawRLeaf(-0.45, -0.6);

        drawLLeaf(-0.55, -0.55);
        drawLLeaf(-0.55, -0.65);
    }

    if (stage >= 3) {
        glPushMatrix();

        glTranslatef(-0.5f, -0.8f, 0.0f);

        float scaleFactor = (stage == 3) ? 0.8f : 1.2f;
        glScalef(scaleFactor, scaleFactor, 1.0f);

        glTranslatef(0.5f, 0.8f, 0.0f);

        glColor3f(0.4, 0.2, 0.0);
        glBegin(GL_QUADS);
        glVertex2f(-0.4, -0.8);
        glVertex2f(-0.6, -0.8);
        glVertex2f(-0.56, -0.2);
        glVertex2f(-0.44, -0.2);
        glEnd();

        glColor3f(0.0, 0.5, 0.0);
        drawFilledCircle(-0.5, -0.1, 0.3);

        glPopMatrix();
    }

    if (scene == 4) {
        //falling an apple
        glPushMatrix();
        glTranslatef(0.0f, appleTranslateY, 0.0f);
        drawApple(-0.35, 0.15);
        glPopMatrix();

        drawApple(-0.5, 0.05);
        drawApple(-0.65, -0.05);
        drawApple(-0.35, -0.05);
        drawApple(-0.65, 0.15);
        drawApple(-0.5, -0.15);
        drawApple(-0.5, 0.2);
    }
}

void drawBackground(int scene) {

    // color of the sky
    if (scene == 1)       glColor3f(0.5, 0.8, 1.0); //Light Blue → Clear Weather
    else if (scene == 2)  glColor3f(0.6, 0.6, 0.6); //Gray → Rain Scene Mood
    else if (scene == 3)  glColor3f(0.5, 0.8, 1.0);
    else if (scene == 4)  glColor3f(0.5, 0.8, 1.0);

    // the sky
    glBegin(GL_QUADS);
    glVertex2f(-1, 1);
    glVertex2f(1, 1);
    glVertex2f(1, -1);
    glVertex2f(-1, -1);
    glEnd();

    // the ground
    glColor3f(0.3, 0.8, 0.3); //Green → Nature Environment
    glBegin(GL_QUADS);
    glVertex2f(-1, -0.3);
    glVertex2f(1, -0.3);
    glVertex2f(1, -1);
    glVertex2f(-1, -1);
    glEnd();


    // the sun disappears wihle  the rain
    if (scene != 2) {
        
            glPushMatrix();

            glTranslatef(0.7f, 0.7f, 0.0f);

            glRotatef(sunAngle, 0.0, 0.0, 1.0);

            // Sun center
            glColor3f(1.0, 1.0, 0.0); //Yellow → Daylight Source
            drawFilledCircle(0.0, 0.0, 0.08);

            // Sun rays
            glColor3f(1.0, 0.8, 0.0); //Warm Orange Tone

            for (int i = 0; i < 8; i++)
            {
                glRotatef(45, 0.0, 0.0, 1.0);

                glBegin(GL_LINES);
                glVertex2f(0.10, 0.0);
                glVertex2f(0.16, 0.0);
                glEnd();
            }

            glPopMatrix();
        }

    if (scene == 2 || scene == 3) {
        drawCloud(cloudX, 0.7);
        drawCloud(cloudX - 0.5, 0.8);
    }

    drawTree(scene);
}

void drawRain() {
    glColor3f(0.7, 0.7, 1.0); //Soft Blue → Water Effect

    for (int i = 0; i < 30; i++) {
        float x = -1 + (i * 0.07);

        glBegin(GL_LINES);
        glVertex2f(x, 1 - rainY);
        glVertex2f(x, 0.8 - rainY);
        glEnd();
    }
}

// House
void drawHouse() {

    glEnable(GL_TEXTURE_2D);
    //Body (Square)
    glBindTexture(GL_TEXTURE_2D, houseTexture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0.4f, -0.3f);
    glTexCoord2f(1, 0); glVertex2f(0.9f, -0.3f);
    glTexCoord2f(1, 1); glVertex2f(0.9f, 0.2f);
    glTexCoord2f(0, 1); glVertex2f(0.4f, 0.2f);

    glEnd();

    //Roof (Triangle)
    glBindTexture(GL_TEXTURE_2D, roofTexture);
    glBegin(GL_TRIANGLES);

    glTexCoord2f(0, 0); glVertex2f(0.35f, 0.2f);
    glTexCoord2f(1, 0); glVertex2f(0.95f, 0.2f);
    glTexCoord2f(0.5f, 1); glVertex2f(0.65f, 0.45f);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    //Door
    glColor3f(0.4f, 0.2f, 0.1f); //Dark Wood
    glBegin(GL_QUADS);
    glVertex2f(0.6f, -0.3f);
    glVertex2f(0.7f, -0.3f);
    glVertex2f(0.7f, -0.05f);
    glVertex2f(0.6f, -0.05f);
    glEnd();

    //Window
    glColor3f(0.7f, 0.9f, 1.0f); //Light Blue Glass
    glBegin(GL_QUADS);
    glVertex2f(0.45f, -0.05f);
    glVertex2f(0.55f, -0.05f);
    glVertex2f(0.55f, 0.05f);
    glVertex2f(0.45f, 0.05f);
    glEnd();
}

void drawScene()
{
    drawBackground(scene);
    drawHouse();

    if (scene == 2) drawRain();
}

// Text
void drawDialogueBox()
{
    // ===== Transparent Box =====
    glColor4f(1, 1, 1, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(-0.9, -0.9);
    glVertex2f(0.9, -0.9);
    glVertex2f(0.9, -0.7);
    glVertex2f(-0.9, -0.7);
    glEnd();

    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.9, -0.9);
    glVertex2f(0.9, -0.9);
    glVertex2f(0.9, -0.7);
    glVertex2f(-0.9, -0.7);
    glEnd();


    if (scene == 1)
    {
        drawText(-0.82, -0.79, "Today I will plant this seed, I hope it grows into an apple tree.");

    }


    else if (scene == 2)
    {
        drawText(-0.82, -0.81, "Rain helps the plant grow stronger.");
    }

    else if (scene == 3)
    {

        drawText(-0.82, -0.81, "The tree is growing!");
    }

    else if (scene == 4)
    {
        drawText(-0.82, -0.79,
            "The tree gives fruits!Now I can eat apples");
    }
}

// =======================================
// ANIMATION & UPDATE
// =======================================
void update()
{
    sunAngle += 0.2f;

    if (sunAngle > 360)
        sunAngle = 0;

    cloudX += 0.002;
    if (cloudX > 1.2) cloudX = -1.2;

    rainY += 0.01;
    if (rainY > 1) rainY = 0;

    // ===== RAIN SOUND CONTROL =====
    if (scene == 2 && !rainSoundPlaying)
    {
#ifdef _WIN32
        PlaySound(L"Rain.wav",
            NULL,
            SND_FILENAME | SND_ASYNC | SND_LOOP);
#endif
        rainSoundPlaying = true;
    }
    else if (scene != 2 && rainSoundPlaying)
    {
#ifdef _WIN32
        PlaySound(NULL, 0, 0); // stop sound
#endif
        rainSoundPlaying = false;
    }

    // girl walking animation
    if (walkGirl && girlX > -0.5f)
    {
        girlX -= 0.003f;
    }

    if (appleFalling && appleTranslateY > -0.8f)
    {
        appleTranslateY -= 0.015f;
    }

    glutPostRedisplay();
}
// Display
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // ===== Scene =====
    drawScene();    

    // ===== Character =====
    if (scene == 1 || walkGirl)
        drawCharacter();

    // ===== UI Layer =====
    glEnable(GL_BLEND);
    drawDialogueBox();
    glDisable(GL_BLEND);
    glutSwapBuffers();
}

// ======================
// RESET PROJECT STATE
// ======================
void resetProject()
{
    // scenes animation
    sunAngle = 0.0f;
    cloudX = -1.0f;
    rainY = 0.0f;

    // girl
    walkGirl = false;
    girlX = 0.0f;

    // apple
    appleFalling = false;
    appleTranslateY = 0.0f;

    // sound
    rainSoundPlaying = false;
}

// =======================================
// INTERACTION (KEYBOARD)
// =======================================
void keyboard(unsigned char key, int x, int y) {

    if (key == 'n')
    {
        scene++;

        if (scene > 4)
        {
            scene = 1;
            resetProject();
        }
    }

    if (key == 'r') {
        scene = 4;
    }

    // Girl movement key
    if (key == 'w')
    {
        scene = 4;        // go to the final scene
        girlX = 1.2f;     // start outside the screen on the right
        walkGirl = true;  // start walking
    }


    if (key == 'f') {
        if (scene == 4) {
            appleFalling = true;
        }
    }

}
void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    houseTexture = LoadTexture("Brick.bmp");
    roofTexture = LoadTexture("roof.bmp");
}

// main
int main(int argc, char** argv) {

    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Project");

    init();

    glutDisplayFunc(display); // Register display callback handler for window re-paint
    glutKeyboardFunc(keyboard);
    glutIdleFunc(update);
    glutReshapeFunc(reshapeFun);

    glutMainLoop();
    return 0;
}