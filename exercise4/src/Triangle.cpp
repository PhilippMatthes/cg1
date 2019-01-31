// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Triangle.h"
#include "GridUtils.h"
#include <tuple>
#include <iostream>


/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float boxhalfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

#include <math.h>

#include <stdio.h>

#define X 0

#define Y 1

#define Z 2



#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];



#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])



#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];



#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;



int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3])
{
    int q;
    float vmin[3],vmax[3],v;
    for(q=X;q<=Z;q++)
    {
        v=vert[q];
        if(normal[q]>0.0f)
        {
            vmin[q]=-maxbox[q] - v;	// -NJMP-
            vmax[q]= maxbox[q] - v;	// -NJMP-
        }
        else
        {
            vmin[q]= maxbox[q] - v;	// -NJMP-
            vmax[q]=-maxbox[q] - v;	// -NJMP-
        }
    }
    if(DOT(normal,vmin)>0.0f) return 0;	// -NJMP-
    if(DOT(normal,vmax)>=0.0f) return 1;	// -NJMP-
    return 0;

}


/*======================== X-tests ========================*/

#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;



#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;



/*======================== Y-tests ========================*/

#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;



#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;



/*======================== Z-tests ========================*/



#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;



#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;



int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3])
{



    /*    use separating axis theorem to test overlap between triangle and box */
    /*    need to test for overlap in these directions: */
    /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
    /*       we do not even need to test these) */
    /*    2) normal of the triangle */
    /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
    /*       this gives 3x3=9 more tests */

    float v0[3],v1[3],v2[3];
//   float axis[3];

    float min,max,p0,p1,p2,rad,fex,fey,fez;		// -NJMP- "d" local variable removed
    float normal[3],e0[3],e1[3],e2[3];

    /* This is the fastest branch on Sun */
    /* move everything so that the boxcenter is in (0,0,0) */
    SUB(v0,triverts[0],boxcenter);
    SUB(v1,triverts[1],boxcenter);
    SUB(v2,triverts[2],boxcenter);

    /* compute triangle edges */
    SUB(e0,v1,v0);      /* tri edge 0 */
    SUB(e1,v2,v1);      /* tri edge 1 */
    SUB(e2,v0,v2);      /* tri edge 2 */

    /* Bullet 3:  */
    /*  test the 9 tests first (this was faster) */
    fex = fabsf(e0[X]);
    fey = fabsf(e0[Y]);
    fez = fabsf(e0[Z]);
    AXISTEST_X01(e0[Z], e0[Y], fez, fey);
    AXISTEST_Y02(e0[Z], e0[X], fez, fex);
    AXISTEST_Z12(e0[Y], e0[X], fey, fex);
    fex = fabsf(e1[X]);
    fey = fabsf(e1[Y]);
    fez = fabsf(e1[Z]);
    AXISTEST_X01(e1[Z], e1[Y], fez, fey);
    AXISTEST_Y02(e1[Z], e1[X], fez, fex);
    AXISTEST_Z0(e1[Y], e1[X], fey, fex);
    fex = fabsf(e2[X]);
    fey = fabsf(e2[Y]);
    fez = fabsf(e2[Z]);
    AXISTEST_X2(e2[Z], e2[Y], fez, fey);
    AXISTEST_Y1(e2[Z], e2[X], fez, fex);
    AXISTEST_Z12(e2[Y], e2[X], fey, fex);

    /* Bullet 1: */
    /*  first test overlap in the {x,y,z}-directions */
    /*  find min, max of the triangle each direction, and test for overlap in */
    /*  that direction -- this is equivalent to testing a minimal AABB around */
    /*  the triangle against the AABB */

    /* test in X-direction */
    FINDMINMAX(v0[X],v1[X],v2[X],min,max);
    if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

    /* test in Y-direction */
    FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);

    if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

    /* test in Z-direction */
    FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
    if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

    /* Bullet 2: */
    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    CROSS(normal,e0,e1);
    if(!planeBoxOverlap(normal,v0,boxhalfsize)) return 0;

    return 1;   /* box and triangle overlaps */

}






//default constructor
Triangle::Triangle()
{
}
//constructs a triangle using the vertex positions v0,v1 and v2
Triangle::Triangle(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1,const Eigen::Vector3f& v2): v0(v0),v1(v1),v2(v2)
{
}
//constructs a triangle from  the face f of the given halfedge mesh m
Triangle::Triangle(const HEMesh&m, const OpenMesh::FaceHandle& f):h(f)
{
	OpenMesh::HalfedgeHandle he = m.halfedge_handle(f);
	v0 = ToEigenVector(m.point(m.from_vertex_handle(he)));
	he = m.next_halfedge_handle(he);
	v1 = ToEigenVector(m.point(m.from_vertex_handle(he)));
	he = m.next_halfedge_handle(he);
	v2 = ToEigenVector(m.point(m.from_vertex_handle(he)));
}
//returns the smallest axis aligned bounding box of the triangle
Box Triangle::ComputeBounds() const
{
	/* Task 3.2.2 */
	Box b;
	b.Insert(v0);
	b.Insert(v1);
	b.Insert(v2);
	return b;
}


//returns true if the triangle overlaps the given box b
bool Triangle::Overlaps(const Box& b) const
{
    float boxcenter[3], boxhalfsize[3], triverts[3][3];
    boxcenter[0] = b.Center()(0);
    boxcenter[1] = b.Center()(1);
    boxcenter[2] = b.Center()(2);
    boxhalfsize[0] = b.HalfExtents()(0);
    boxhalfsize[1] = b.HalfExtents()(1);
    boxhalfsize[2] = b.HalfExtents()(2);
    triverts[0][0] = v0(0);
    triverts[0][1] = v0(1);
    triverts[0][2] = v0(2);
    triverts[1][0] = v1(0);
    triverts[1][1] = v1(1);
    triverts[1][2] = v1(2);
    triverts[2][0] = v2(0);
    triverts[2][1] = v2(1);
    triverts[2][2] = v2(2);
    return static_cast<bool>(triBoxOverlap(boxcenter, boxhalfsize, triverts));
}
//returns the barycentric coordinates of the point with the smallest distance to point p which lies on the triangle
void Triangle::ClosestPointBarycentric(const Eigen::Vector3f& p, float& l0, float& l1, float& l2) const
{
	Eigen::Vector3f edge0 = v1 - v0;
	Eigen::Vector3f edge1 = v2 - v0;
	Eigen::Vector3f v = v0 - p;

	float a = edge0.dot( edge0 );
	float b = edge0.dot( edge1 );
	float c = edge1.dot( edge1 );
	float d = edge0.dot( v );
	float e = edge1.dot( v );

	float det = a*c - b*b;
	float s = b*e - c*d;
	float t = b*d - a*e;

	if ( s + t < det )
	{
		if ( s < 0.f )
		{
			if ( t < 0.f )
			{
				if ( d < 0.f )
				{
					s=-d/a;
					s=std::min(std::max(s,0.0f),1.0f);
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = -e/c;
					t = std::min(std::max(t,0.0f),1.0f);
                 
				}
			}
			else
			{
				s = 0.f;
				t = -e/c;
				t = std::min(std::max(t,0.0f),1.0f);
			}
		}
		else if ( t < 0.f )
		{
			s =  -d/a;
			s=std::min(std::max(s,0.0f),1.0f);
			t = 0.f;
		}
		else
		{
			float invDet = 1.f / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if ( s < 0.f )
		{
			float tmp0 = b+d;
			float tmp1 = c+e;
			if ( tmp1 > tmp0 )
			{
				float numer = tmp1 - tmp0;
				float denom = a-2*b+c;
				s = numer/denom;
				s=std::min(std::max(s,0.0f),1.0f);
				t = 1-s;
			}
			else
			{
				t = -e/c;
				t=std::min(std::max(t,0.0f),1.0f);
				s = 0.f;
			}
		}
		else if ( t < 0.f )
		{
			if ( a+d > b+e )
			{
				float numer = c+e-b-d;
				float denom = a-2*b+c;
				s = numer/denom;
				s=std::min(std::max(s,0.0f),1.0f);
               
				t = 1-s;
			}
			else
			{
				s =  -e/c;
				s=std::min(std::max(s,0.0f),1.0f);
				t = 0.f;
			}
		}
		else
		{
			float numer = c+e-b-d;
			float denom = a-2*b+c;

			s =  numer/denom;
			s=std::min(std::max(s,0.0f),1.0f);
			t = 1.f - s;
		}
	}
	l0 = 1-s-t;
	l1 = s;
	l2 = t;
}
//returns the point with smallest distance to point p which lies on the triangle
Eigen::Vector3f Triangle::ClosestPoint(const Eigen::Vector3f& p) const
{
	float l0,l1,l2;
	ClosestPointBarycentric(p,l0,l1,l2);
	return l0*v0 + l1*v1 +l2* v2;

}
//returns the squared distance between point p and the triangle
float Triangle::SqrDistance(const Eigen::Vector3f& p) const
{
	Eigen::Vector3f d = p-ClosestPoint(p);
	return d.squaredNorm();
}
//returns the euclidean distance between point p and the triangle
float Triangle::Distance(const Eigen::Vector3f& p) const
{
	return sqrt(SqrDistance(p));
}
//returns a reference point  which is on the triangle and is used to sort the primitive in the AABB tree construction
Eigen::Vector3f Triangle::ReferencePoint() const
{
	return (v0+v1+v2)/3.0f;
}



