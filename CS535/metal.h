#pragma once
#include "material.h"
#include "MathTool.h"

class metal : public material
{
public:
	V3 albedo;

	metal(const V3 &a) :albedo(a) {};
	virtual bool scatter(ray r_in, hit_record& rec, V3& attenuation, ray& scattered) const override
	{
		V3 reflected = (V3(0.0f) - r_in.direction().UnitVector()).Reflect(rec.n.UnitVector());
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (scattered.direction() * rec.n > 0.0f);
	}
};
