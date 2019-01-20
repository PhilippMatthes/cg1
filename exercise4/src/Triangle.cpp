// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Triangle.h"
#include "GridUtils.h"
#include <tuple>
#include <iostream>


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
	/* Task 3.2.2 */
	/*
	 * 	[3 tests] e0 = (1, 0, 0), e1 = (0, 1, 0), e2 = (0, 0, 1) (the normals of the
		AABB). Test the AABB against the minimal AABB around the triangle
	 */
	std::vector<Eigen::Vector3f> e;
	Eigen::Vector3f e0 (1, 0, 0);
	Eigen::Vector3f e1 (0, 1, 0);
	Eigen::Vector3f e2 (0, 0, 1);
	e.push_back(e0);
	e.push_back(e1);
	e.push_back(e2);
	/*
	 * 	[1 test] n, the normal of ∆. We use a fast plane/AABB overlap test [5, 6],
		which only tests the two diagonal vertices, whose direction is most closely
		aligned to the normal of the triangle.
	 */
	std::vector<Eigen::Vector3f> f;
	Eigen::Vector3f f0 = v1 - v0;
	Eigen::Vector3f f1 = v2 - v1;
	Eigen::Vector3f f2 = v0 - v2;
	f.push_back(f0);
	f.push_back(f1);
	f.push_back(f2);
	Eigen::Vector3f n = f0.cross(f1);
	/*
	 * 	[9 tests] aij = ei × fj , i, j ∈ {0, 1, 2}, where f0 = v1 − v0, f1 = v2 − v1,
		and f2 = v0 − v2. These tests are very similar and we will only show the
		derivation of the case where i = 0 and j = 0 (see below).
	 */
	std::vector<Eigen::Vector3f> a;
	for (int i = 0; i < 3; i += 1) {
		for (int j = 0; j < 3; j += 1) {
			Eigen::Vector3f ei = e[i];
			Eigen::Vector3f fj = f[j];
			Eigen::Vector3f aij = ei.cross(fj);
			a.push_back(aij);
		}
	}

	std::vector<Eigen::Vector3f> v;
	v.insert(v.end(), e.begin(), e.end());
	v.push_back(n);
	v.insert(v.end(), a.begin(), a.end());

	for (auto const& vi : v) {
		float p0 = vi.dot(v0);
		float p1 = vi.dot(v1);
		float p2 = vi.dot(v2);

		Eigen::Vector3f h = b.HalfExtents();
		float r = h(0) * abs(vi(0)) + h(1) * abs(vi(1)) + h(2) * abs(vi(2));
		if (std::min(std::min(p0, p1), p2) > r || std::max(std::max(p0, p1), p2) < -r) return false;
	}

	return true;
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



