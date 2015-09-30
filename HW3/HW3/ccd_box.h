////////////////////////////////
// ccd
// ccd data structure
// [jointid, rotation axis]
//

#include "svl/svl.h"
#include <iostream>
#include <vector>
using namespace std;


typedef struct {
	int jointid;  // the joint the axis is attached to, from 0
	Vec3 axis;
	Vec2 limits;  // joint limits (radian) [lo, hi]
} CCD_axis;

extern vector <CCD_axis> axes;

#if 0	
struct ARM
{
	vector <CCD_axis> axes;
	vector <double> linklens;  // ......
};
#endif
