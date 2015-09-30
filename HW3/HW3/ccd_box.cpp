#include "ccd_box.h"

static Vec3 proj2plane (const Vec3& p, const Vec3& n)
// n: the normal vector, should be a unit vector
{
	return p - dot(p,n)*n;   // return in-plane component
}

#define CLAMP(x,lo,hi)  ((x) > (hi) ? (hi) : ((x) < (lo) ? (lo) : (x)))

int ik_ccd (const Vec3 &target, Vec& q) 
{
	extern void fk(const Vec &q, vector<Vec3>& joints);
#define MAX_ITER 10

	vector <Vec3> joints;
	// allocate joint positions
	int njoints = axes[axes.size()-1].jointid + 1;
	for (int i = 0; i <= njoints; i++)
		joints.push_back (Vec3(0,0,0));
	
	Vec3 end;
	double eps = 1e-4;

	fk (q, joints);
	end = joints[joints.size()-1];

	// initial fk
	// for each iteration (loop through all axes)
	//   for each axis (world coordinate, updated hierarchically)
	// 	   find its best angle; fk; check error
	
	int iter;
	for (iter = 0; iter < MAX_ITER; iter++) {			
		for (int i = axes.size()-1; i >= 0; i--) {  // distal joint first
			Vec3 base = joints[axes[i].jointid];
			
			// the rotation axis, in WORLD coord
			Vec3 axis; 
			Mat3 rotmat = vl_I;
			for (int j = 0; j < i; j++) 
				rotmat *= Rot3 (axes[j].axis, q[j]);
			axis = rotmat* axes[i].axis;
//	cout << "axis: " << axis << endl;  ok ....
	
			Vec3 t_target = proj2plane (target-base, axis);
			Vec3 t_reach = proj2plane(end-base, axis);

			double angle = acos(CLAMP(dot (norm(t_reach), norm(t_target)), -1,1));
			double sign = dot(cross (t_reach, t_target), axis) > 0 ? 1.0 : -1.0;
			q[i] += sign* angle;
			
			// joint limit
			q[i] = CLAMP (q[i], axes[i].limits[0], axes[i].limits[1]);

			fk (q, joints);
			end = joints[joints.size()-1];

			Vec3 dx = target - end;
			double err = len(dx);
			//cout << "err: " << err << endl;
			if (len(dx) < eps) {
				return 1; // successful ...
			}
		}
		
	} 

	if (iter < MAX_ITER) 
		return 1;
	else {
		//printf ("restore (do not converge)\n");
		return 0;
	}
}		

