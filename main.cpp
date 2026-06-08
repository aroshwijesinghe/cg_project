#include "Game.h"
#include <GL/freeglut.h>

Game game;

void display() {
    game.draw();
}

void timer(int) {
    game.update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyDown(unsigned char key, int, int) {
    game.handleInput(key, true);
}

void keyUp(unsigned char key, int, int) {
    game.handleInput(key, false);
}

void specialDown(int key, int, int) {
    game.handleSpecialInput(key, true);
}

void specialUp(int key, int, int) {
    game.handleSpecialInput(key, false);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Space Odyssey - Rogue Starship");

    game.init();

    glutDisplayFunc(display);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);

    glutMainLoop();
    return 0;
}
