#pragma once
#include "ray.h"
#include "hitable.h"
#include "MathTool.h"

class material
{
public:
	virtual bool scatter(ray r_in, hit_record &rec, V3 &attenuation, ray &scattered) const = 0;
};

class lambertian : public material
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

class dielectric : public material
{
public:
	float ref_idx;

	dielectric(float ri) : ref_idx(ri)
	{
	}


	bool scatter(ray r_in, hit_record& rec, V3& attenuation, ray& scattered) const override
	{
		V3 outward_normal;
		V3 reflected = (V3(0.0f) - r_in.direction().UnitVector()).Reflect(rec.n);
		float ni_over_nt = 0.0f;
		attenuation = V3(1.0f);
		V3 refracted(0.0f);
		float reflect_prob = 0.0f, cosin = 0.0f;

		if ((r_in.direction() * rec.n) > 0.0f)
		{
			outward_normal = V3(0.0f) - rec.n;
			ni_over_nt = ref_idx;
			cosin = ref_idx * (r_in.direction() * rec.n) / r_in.direction().Length();
		}
		else
		{
			outward_normal = rec.n;
			ni_over_nt = 1.0f / ref_idx;
			cosin = -(r_in.direction() * rec.n) / r_in.direction().Length();
		}

		reflect_prob = refract(r_in.direction(), outward_normal, ni_over_nt, refracted)
			? schlick(cosin, ref_idx)
			: reflect_prob = 1.0f;


		scattered = Random(0.0f, 1.0f) < reflect_prob ? ray(rec.p, reflected) : ray(rec.p, refracted);

		return true;
	}
};

class metal : public material
{
public:
	V3 albedo;
	float fuzz;

	metal(const V3 &a, float f = 0.0f) :albedo(a) { fuzz = min(1.0f, f); };
	virtual bool scatter(ray r_in, hit_record& rec, V3& attenuation, ray& scattered) const override
	{
		V3 reflected = (V3(0.0f) - r_in.direction().UnitVector()).Reflect(rec.n.UnitVector());
		scattered = ray(rec.p, reflected + random_in_unit_shpere() * fuzz);
		attenuation = albedo;
		return (scattered.direction() * rec.n > 0.0f);
	}
};
