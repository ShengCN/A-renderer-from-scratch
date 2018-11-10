#pragma once
#include "ray.h"
#include "hitable.h"

class material
{
public:
	virtual bool scatter(ray r_in, hit_record &rec, V3 &attenuation, ray &scattered) const = 0;
};
