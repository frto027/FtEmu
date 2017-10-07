#include "TestDisplay.h"
#include "GL/gl.h"

void TestDisplay() {
	glBegin(GL_LINE_LOOP);
	glVertex2d(1, 1);
	glVertex2d(ORI_WIDTH - 1, 1);
	glVertex2d(ORI_WIDTH - 1, ORI_HEIGHT - 1);
	glEnd();
}
