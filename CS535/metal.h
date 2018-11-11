#pragma once
#include "material.h"
#include "MathTool.h"

class metal : public material
{
public:
	V3 albedo;
	float fuzz;

	metal(const V3 &a, float f) :albedo(a) { fuzz = min(1.0f, f); };
	virtual bool scatter(ray r_in, hit_record& rec, V3& attenuation, ray& scattered) const override
	{
		V3 reflected = (V3(0.0f) - r_in.direction().UnitVector()).Reflect(rec.n.UnitVector());
		scattered = ray(rec.p, reflected + random_in_unit_shpere() * fuzz);
		attenuation = albedo;
		return (scattered.direction() * rec.n > 0.0f);
	}
};
