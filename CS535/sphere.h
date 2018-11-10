#pragma once
#include "hitable.h"
class sphere :
	public hitable
{
public:
	V3 center;
	float radius;
	shared_ptr<material> mat_ptr;

	sphere() = default; 
	sphere(V3 cen, float r, shared_ptr<material> mat) : center(cen), radius(r), mat_ptr(mat) {};

	bool hit(ray& r, float tmin, float tmax, hit_record &rec) const;
	
};

inline bool sphere::hit(ray& r, float tmin, float tmax, hit_record& rec) const
{
	V3 oc = r.origin() - center;
	float a = r.direction() * r.direction();
	float b = (oc)* r.direction() * 2.0f;
	float c = oc * oc - radius * radius;
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant > 0.0f)
	{
		float t = (-b - sqrt(discriminant)) / (a * 2.0f);

		if(t > tmin && t < tmax)
		{
			rec.t = t;
			rec.p = r.point_at_parameter(t);
			rec.n = (rec.p - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}

		t = -b + sqrt(discriminant) / (a * 2.0f);
		if (t > tmin && t < tmax)
		{
			rec.t = t;
			rec.p = r.point_at_parameter(t);
			rec.n = (rec.p - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}

