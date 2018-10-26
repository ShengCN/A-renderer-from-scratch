#pragma once
#include "TM.h"

class BillBoard
{
public:
	shared_ptr<TM> mesh;

	BillBoard();
	~BillBoard();

	void SetBillboard(V3 O, V3 n, V3 up, float sz, float s = 1.0f, float t = 1.0f);
	bool Intersect(V3 p, V3 d, float & t);
	bool InsideBillboard(V3 p);
	V3 GetColor(FrameBuffer *fb, V3 p);
	V3 GetColor(FrameBuffer *fb, V3 p, float &alpha);

private:
	void GetST(V3 p, float &s, float &t);
};

