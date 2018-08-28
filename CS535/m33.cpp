#include "m33.h"

V3 M33::GetColumn(int c)
{
	M33 m = *this;
	return V3(m[0][c], m[1][c], m[2][c]);
}

void M33::SetColumn(int c, V3 v)
{
	rows[0][c] = v[0];
	rows[1][c] = v[1];
	rows[2][c] = v[2];
}

V3& M33::operator[](int r)
{
	return rows[r];
}

V3 M33::operator*(V3 v)
{
	V3 r0 = rows[0];
	V3 r1 = rows[1];
	V3 r2 = rows[2];
	return V3(r0*v, r1*v, r2*v);
}

M33 M33::operator*(M33 m1)
{
	M33 m0 = *this, res;
	V3 c0 = m1.GetColumn(0);
	V3 c1 = m1.GetColumn(1);
	V3 c2 = m1.GetColumn(2);
	res.SetColumn(0, m0*c0);
	res.SetColumn(1, m0*c1);
	res.SetColumn(2, m0*c2);

	return res;
}

bool M33::operator==(M33 m1)
{
	M33 m0 = *this;
	return m0[0] == m1[0]
		&& m0[1] == m1[1]
		&& m0[2] == m1[2];
}

bool M33::operator!=(M33 m1)
{
	M33 m0 = *this;
	return !(m0 == m1);
}

ostream& operator<<(ostream& ost, M33 m)
{
	return ost << m[0] << m[1] << m[2] << endl;
}
