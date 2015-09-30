#include <stdio.h>
#include <stdlib.h>
#include <gl/glut.h>
#include <fmod.h>
#include <fmod_errors.h>

#include "svl/svl.h"
#include "gluit.h"

#pragma comment (lib, "svl-vc9.lib")
#pragma comment (lib, "gluit-vc9.lib")
#pragma comment (lib, "fmodvc.lib")

using namespace std;

#define HAND "./hand.wav"
#define WALL "./wall.wav"
#define NEWGAME "./newgame.wav"
#define BGM "./bgm.wav"
#define TRUE 1
#define FALSE 0

static int reachable = 1;

int gw = 800, gh = 800, playing = 0, sound = 1;
double r = 2.0;
float puck_m = 10.0, f = 500.0, a;
Vec3 target(0, 0, 0), puck(0, 0, 30), hand, v(0.0, 0.0, 0.0), hv(0.0, 0.0, 0.0), hl;

FSOUND_SAMPLE *samp_hand, *samp_wall, *samp_new, *samp_war, *samp_bgm;

/*			fmod_SE, BGM           */
void init_fmod()
{
	if(FSOUND_Init(11025,8,0)){
		printf("%s\n",FMOD_ErrorString(FSOUND_GetError()));
		//exit(1);
	}
	samp_hand = FSOUND_Sample_Load (FSOUND_UNMANAGED, HAND, FSOUND_NORMAL,0,0);
	samp_wall = FSOUND_Sample_Load (FSOUND_UNMANAGED, WALL, FSOUND_NORMAL,0,0);
	samp_new = FSOUND_Sample_Load (FSOUND_UNMANAGED, NEWGAME, FSOUND_NORMAL,0,0);
	samp_bgm = FSOUND_Sample_Load (FSOUND_UNMANAGED, BGM, FSOUND_NORMAL,0,0);
	FSOUND_Sample_SetMode(samp_hand, FSOUND_LOOP_OFF);
	FSOUND_Sample_SetMode(samp_wall, FSOUND_LOOP_OFF);
	FSOUND_Sample_SetMode(samp_new, FSOUND_LOOP_OFF);
	FSOUND_Sample_SetMode(samp_bgm, FSOUND_LOOP_NORMAL);
}

static int channel, ch_bgm;

void SE_hand()
{
	channel = FSOUND_PlaySound(FSOUND_FREE, samp_hand);
}

void SE_wall()
{
	channel = FSOUND_PlaySound(FSOUND_FREE, samp_wall);
}

void SE_new()
{
	channel = FSOUND_PlaySound(FSOUND_FREE, samp_new);
}

void bgm()
{
	ch_bgm = FSOUND_PlaySound(FSOUND_FREE, samp_bgm);
}
/*			 fmod_SE, BGM_end           */

float D(float x1, float z1, float x2, float z2)
{
	return sqrt((x1 - x2) * (x1 - x2) + (z1 - z2) * (z1 - z2));					//count distance
}

void display()
{
	glViewport(0, 0, gw, gh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, 1, 1, 400);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear (GL_COLOR_BUFFER_BIT);

	gluLookAt(0, 60, -40, 0, 0, 60, 0, 1, 0);
	
	BEGIN_2D_OVERLAY(100, 100);													//text
	glColor3ub(126, 241, 135);
	drawstr(1, 95, "Control:");
	drawstr(1, 90, "LEFT_BUTTON : restart");
	drawstr(1, 85, "F / f : f (friction) up / down");
	drawstr(1, 80, "M / m : puck_m (mass) up / down");
	drawstr(1, 75, "Z / z : turn on / off the music");
	drawstr(1, 70, "target : (%.2f, %.2f)", target[0], target[2]);
	drawstr(1, 65, "hand : (%.2f, %.2f)", hand[0], hand[2]);
	drawstr(1, 60, "hand_v : (%.2f, %.2f)", hv[0], hv[2]);
	drawstr(51, 95, "State:");
	drawstr(51, 90, (playing)? "Now Playing..." : "New Game");
	drawstr(51, 85, "f : %.2f", f);
	drawstr(51, 80, "puck_m : %.2f", puck_m);
	drawstr(51, 75, (sound)? "Normal Mode" : "Soundless Mode");
	drawstr(51, 65, "puck : (%.2f, %.2f)", puck[0], puck[2]);
	drawstr(51, 60, "puck_v : (%.2f, %.2f)", v[0], v[2]);
	END_2D_OVERLAY();


	int i;
	glColor3ub(255, 255, 255);													//grid
	glLineWidth (1.0);
	glBegin(GL_LINES);
	int min = -50, max = 50;
	for(i = min; i <= max; i += 5){
		glVertex3i(i, 0, min + max);
		glVertex3i(i, 0, max + max);
		glVertex3i(min, 0, i + max);
		glVertex3i(max, 0, i + max);
	}
	glEnd();

	glColor3ub (255,255,0);														//puck
	glBegin (GL_POLYGON);
	double theta = 2 * 3.1416 / 100;
	for(int i = 0; i < 100; i++)
	    glVertex3f (puck[0] + r * cos(theta * i), 0, puck[2] + r * sin(theta * i));
	glEnd();

	extern void drawarm(Vec3 &);
	drawarm(hand);

	glutSwapBuffers();
}

void timer(int dummy)
{
	glutTimerFunc(30,timer,0);

	static int last;
	int now;
	float dt;

	if(last == 0){
		last = glutGet(GLUT_ELAPSED_TIME);
		hl[0] = 0.0;
		hl[1] = 0.0;
		hl[2] = 0.0;
		return;
	}
	now = glutGet(GLUT_ELAPSED_TIME);
	dt = (now - last) / 1000.0;
	hv[0] = (hand[0] - hl[0]) / dt;
	hv[2] = (hand[2] - hl[2]) / dt;
	
	//printf("%f %f\n", v[0], v[2]);

	last = now;
	hl[0] = hand[0];
	hl[2] = hand[2];
	
	float d = D(puck[0], puck[2], hand[0], hand[2]);
	if(d <= r * 2.5){																			//collision with hand
		if(sound)
			SE_hand();
		if(playing == 0)
			playing = 1;
		v = v - (2 * (dot((v - hv), (puck - hand))) / pow(d, 2) * (puck - hand));
	}
	puck[0] += v[0] * dt;
	puck[2] += v[2] * dt;

	if(puck[0] - r <= -50){																		//collision with wall
		if(sound)
			SE_wall();
		v[0] *= -1;
		puck[0] = -50 + r + 0.1;
	}
	else if(puck[0] + r >= 50){
		if(sound)		
			SE_wall();
		v[0] *= -1;
		puck[0] = 50 - r - 0.1;
	}
	if(puck[2] - r <= 0){
		if(sound)
			SE_wall();
		v[2] *= -1;
		puck[2] = 0 + r + 0.1;
	}
	else if(puck[2] + r >= 100){
		if(sound)	
			SE_wall();
		v[2] *= -1;
		puck[2] = 100 - r - 0.1;
	}

	a = f / puck_m;									//friction
	a *= dt;
	if(v[0] != 0){
		if(v[0] > 0){
			v[0] -= a;
			if(v[0] < 0)
				v[0] = 0;
		}
		else{
			v[0] += a;
			if(v[0] > 0)
				v[0] = 0;
		}
	}
	if(v[2] != 0){
		if(v[2] > 0){
			v[2] -= a;
			if(v[2] < 0)
				v[2] = 0;
		}
		else{
			v[2] += a;
			if(v[2] > 0)
				v[2] = 0;
		}
	}

	glutPostRedisplay();
}

void reshape(int w, int h)
{
	gw = w;
	gh = h;
}

void screen2object (int x, int y, float* xz)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint realy;  /*  OpenGL y coordinate position  */
	GLdouble wx0, wy0, wz0, wx1, wy1, wz1;  /*  returned world x, y, z coords  */
    GLdouble t ;

	glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

    realy = viewport[3] - (GLint) y - 1;
		
    gluUnProject ((GLdouble) x, (GLdouble) realy, 0.0, 
            mvmatrix, projmatrix, viewport, &wx0, &wy0, &wz0);
    gluUnProject ((GLdouble) x, (GLdouble) realy, 1.0, 
            mvmatrix, projmatrix, viewport, &wx1, &wy1, &wz1); 

	if(wy0 == wy1)										//avoid -1.#IND00
		t = 0;
	else
		t = wy0 / (wy0 - wy1);
	xz[0] = (1-t) * wx0 + t * wx1;
	xz[2] = (1-t) * wz0 + t * wz1;
    xz[1] = 0.0;
}

/*			Control			*/
void passive (int x, int y)
{
	float tmp[3];

	screen2object(x, y, tmp);
	target[0] = tmp[0];
	target[1] = 0;
	target[2] = tmp[2];

	extern int myik (const Vec3&);
	reachable = myik (target);

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){			//New Game
		if(sound)
			SE_new();
		v[0] = 0.0;
		v[2] = 0.0;
		puck[0] = 0.0;
		puck[2] = 30.0;
		playing = 0;
	}	
}

void keyboard(unsigned char key, int  x, int y)
{
	if(key == 'f' && f > 250)
		f -= 50;
	if(key == 'F' && f < 1000)
		f += 50;
	if(key == 'm' && puck_m > 1)
		puck_m--;
	if(key == 'M' && puck_m < 20)
		puck_m++;
	if(key == 'z' && sound == 1){
		sound = 0;
		FSOUND_SetPaused(ch_bgm, TRUE);
	}
	if(key == 'Z' && sound == 0){
		sound = 1;
		FSOUND_SetPaused(ch_bgm, FALSE);
	}
}
/*			Control_end			*/

void init()
{
	SE_new();
	bgm();
}

void main(int argc, char **argv)
{
	init_fmod();

	glutInit(&argc, argv);
	glutInitWindowSize(800, 800);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutCreateWindow("HW3 | LEFT_BUTTON, Ff, Mm, Zz");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(passive);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);

	init();

	glutTimerFunc(0,timer,0);

	extern void setarm();
	setarm();

	glutMainLoop();
}