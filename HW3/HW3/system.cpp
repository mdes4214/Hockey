#include <stdlib.h>
#include <gl/glut.h>
#include <limits.h>

#include "ccd_box.h"

static Vec2 myq(0,0); // keep the current joint angle
double l1 = 16.0;
double l2 = 15.0;

#define drawOneLine(x1,y1,z1,x2,y2,z2) \
{glBegin (GL_LINES); glVertex3f(x1,y1,z1);glVertex3f(x2,y2,z2); glEnd();}

vector <CCD_axis> axes;

void setarm()
{
	CCD_axis link;
	link.axis = Vec3 (0,1,0); link.jointid = 0; link.limits = Vec2(-1e10,1e10); axes.push_back(link);
	link.axis = Vec3 (0,1,0); link.jointid = 1; link.limits = Vec2(-3, -.05); axes.push_back(link);	
}

void drawarm(Vec3 &hand)
{
	Vec3 p1, p2;

	p1 = proj (HRot4 (Vec3(0,1,0),myq[0]) * HTrans4 (Vec3(l1,0,0)) * Vec4 (0,0,0,1)); 
	p2 = proj (HRot4 (Vec3(0,1,0),myq[0]) * HTrans4 (Vec3(l1,0,0)) * HRot4 (Vec3(0,1,0),myq[1]) * HTrans4 (Vec3(l2,0,0)) * Vec4 (0,0,0,1)); 
	hand = p2;

	glLineWidth(3.0);																		//arm and hand
	glColor3ub(255, 0, 0);
	drawOneLine(0, 0, 0, p1[0], p1[1], p1[2]);
	glColor3ub(0, 255, 255);
	drawOneLine(p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
	glBegin (GL_POLYGON);
	double theta = 2 * 3.1416 / 100;
	double r = 2.0;
	for(int i = 0; i < 100; i++)
	    glVertex3f (p2[0] + r * cos(theta * i), p2[1] + r * sin(theta * i), p2[2]);
	glEnd();
}
 
void fk (const Vec& q, vector<Vec3>& joints)  // joints[0] as base
{
	// assume base is origin
	joints[0] = Vec3(0,0,0);
	joints[1] = proj (HRot4 (Vec3(0,1,0),q[0]) * HTrans4 (Vec3(l1,0,0)) * Vec4 (0,0,0,1)); 
	joints[2] = proj (HRot4 (Vec3(0,1,0),q[0]) * HTrans4 (Vec3(l1,0,0)) * HRot4 (Vec3(0,1,0),q[1]) * HTrans4 (Vec3(l2,0,0)) * Vec4 (0,0,0,1)); 
}

int myik (const Vec3& mytarget)
{
	Vec q (2, vl_0);
	
	int i;
	for (i = 0; i < q.Elts(); i++)
		q[i] = myq[i];

	extern int ik_ccd (const Vec3&, Vec&);
	
	int reachable = ik_ccd (mytarget, q);
	for (i = 0; i < 2; i++)
		myq[i] = q[i];
	return reachable;
}
