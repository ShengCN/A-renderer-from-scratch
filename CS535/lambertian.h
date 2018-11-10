#pragma once
#include "material.h"
#include "MathTool.h"

class lambertian: public material
{
public:
	V3 albedo;

	lambertian(V3 &a) : albedo(a) {};
	virtual  bool scatter(ray r_in, hit_record& rec, V3& attenuation, ray& scattered) const override
	{
		V3 target = rec.p + rec.n + random_in_unit_shpere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}
};