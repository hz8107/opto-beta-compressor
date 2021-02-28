#include "myheader.h"
#include "tables.h"



__m128 DS4x_FIR::ra0, DS4x_FIR::rb0, DS4x_FIR::ra1, DS4x_FIR::rb1, DS4x_FIR::ra2, DS4x_FIR::rb2, DS4x_FIR::ra3, DS4x_FIR::rb3;
__m128 DS4x_FIR::_h0, DS4x_FIR::_h1, DS4x_FIR::_h2, DS4x_FIR::_h3, DS4x_FIR::_h4, DS4x_FIR::_h5, DS4x_FIR::_h6, DS4x_FIR::_h7;


LPF::LPF()
{
	alpha = 1.0f;
	y0 = 0.0f;
	lfsr = 44100.0f;
	lfc = 1.0f;
}

HPF::HPF()
{
	alpha = 1.0f;
	x0 = 0.0f;
	y0 = 0.0f;
	lfc = 1.0f;
	lfsr = 44100.0f;
}


void LPF::SetK(float fc, float fsr)
{
	if (!NeedsUpdate(fc, fsr))return;
	float oc = 2.0f * _pi * fc / fsr;
	alpha = Omega_to_Alpha_LP(oc);
	invalpha = 1.0f - alpha;
	lfsr = fsr;
	lfc = fc;
}

void HPF::SetK(float fc, float fsr)
{
	if (!NeedsUpdate(fc, fsr))return;
	alpha = fsr / (fsr + 2.0f * _pi * fc);
	lfsr = fsr;
	lfc = fc;
}

RcRelease::RcRelease()
{
	y0 = 0.0f;
	x0 = 0.0f;
	lfca = 1.0f;
	lfcr = 1.0f;
	lfsr = 44100.0f;
}

void RcRelease::SetK(float fcA, float fcR, float fsr)
{
	if (!NeedsUpdate(fcA,fcR,fsr))return;
	float rca = 1.0f / (2.0f * _pi * fcA);
	float rcr = 1.0f / (2.0f * _pi * fcR);
	alpha_A = 1.0f / (fsr * rca + 1.0f);
	invalpha_A = 1.0f - alpha_A;
	invalpha_R = 1.0f - (1.0f / (fsr * rcr + 1.0f));
	lfca = fcA;
	lfcr = fcR;
	lfsr = fsr;
}



LDR::LDR()
{
	y0 = 1.0f;
	x0 = 0.0f;
	last_ratio = 1.0f;
	fsr = 44100.0f;
	invfsr = 1.0f / fsr;
	lp0 = new RcRelease();
	lp2 = new LPF();
	rect = new Rectifier();
	LUTratio = new Lookup(TABLESIZE, TRatio, 0.0f, TableMax);
	LUTinvRC = new Lookup(TABLESIZE, TinvRC, 0.0f, TableMax);
}

void LDR::SetK(float tfsr,float limf)
{
	if (tfsr == fsr && limf == ratio_limit)
		return;
	fsr = tfsr;
	invfsr = 1.0f / fsr;
	lp0->SetK(5000,50, fsr);
	lp2->SetK(2000, fsr);
	ratio_limit = limf;
}



Lookup::Lookup(int tsize, const float* tT, float tlb, const float tub)
{
	float p1 = 1.0f, n1 = -1.0f;
	size = tsize;
	fsize = float(tsize-1);
	scale = fsize / (tub - tlb);
	T = tT;
	lb = tlb;
	ub = tub;
	xlb = lb + 1e-7;
	xub = ub - 1e-7;
	mxub = _mm_load1_ps(&xub);
	mxlb = _mm_load1_ps(&xlb);
	mlb = _mm_load1_ps(&lb);
	mscale = _mm_load1_ps(&scale);
	cp1 = _mm_load1_ps(&p1);
	cn1 = _mm_load1_ps(&n1);
}








