#pragma once
#include "v3.h"

class M33 {
public:
	V3 rows[3];
	M33() {};
	M33(V3 v0, V3 v1, V3 v2) { rows[0] = v0; rows[1] = v1; rows[2] = v2; };
	V3 GetColumn(int c);
	void SetColumn(int c, V3 v);
	float Det();
	M33 Inverse();
	M33 Transpose();
	V3& operator[](int r);
	V3 operator*(V3 v);
	M33 operator*(M33 m1);
	M33 operator*(float scf);
	M33 operator/(float scf);
	bool operator==(M33 m1);
	bool operator!=(M33 m1);
	friend ostream& operator<<(ostream& ost, M33 m);
};