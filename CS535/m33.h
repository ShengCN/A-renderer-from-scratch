#pragma once
#include "v3.h"

class M33 {
public:
	V3 rows[3];
	M33() {};
	V3 GetColumn(int c);
	void SetColumn(int c, V3 v);
	V3& operator[](int r);
	V3 operator*(V3 v);
	M33 operator*(M33 m1);
	bool operator==(M33 m1);
	bool operator!=(M33 m1);
	friend ostream& operator<<(ostream& ost, M33 m);
};