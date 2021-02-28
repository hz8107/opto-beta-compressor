#pragma once

#include <cmath>
#include <JuceHeader.h>

#define _pi 3.1415926536f

inline float Omega_to_Alpha_LP(float oc)
{
	return (9.378000141e-1f + -3.419401271e-1f * oc) * oc;
}


//1-pole RC lowpass filter
class LPF
{
private:
	float alpha,invalpha;//invalpha in fact is (1.0f - alpha)
	float y0;
	float lfc, lfsr;
public:
	LPF();
	void SetK(float fc, float fsr);
	inline bool NeedsUpdate(float fc, float fsr) { return !(fc == lfc && fsr == lfsr); };
	inline float Run(float x)
	{
		return y0 = alpha * x + invalpha * y0;
	}
};

//RC attack-release
class RcRelease
{
private:
	float alpha_A,invalpha_A,invalpha_R;
	float y0,x0;
	float lfca,lfcr, lfsr;//l here means last
public:
	RcRelease();
	void SetK(float fcA,float fcR, float fsr);
	inline bool NeedsUpdate(float fcA, float fcR, float fsr) { return !(fcA == lfca && fsr == lfsr && fcR == lfcr); };
	inline float Run(float x)
	{
		register float t;
		t = (x > y0 ? (alpha_A * x + invalpha_A * y0) : y0);
		return y0 = invalpha_R * t;
	}
};

//1-pole rc highpass filter
class HPF
{
private:
	float alpha;
	float y0, x0;
	float lfc, lfsr;
public:
	HPF();
	void SetK(float fc, float fsr);
	inline bool NeedsUpdate(float fc, float fsr) { return !(fc == lfc && fsr == lfsr); };
	inline float Run(float x)
	{
		y0 = alpha * (y0 + x - x0);
		x0 = x;
		return y0;
	}
};

//determine if a parameter needs update ,do convert etc
class ParamMapping
{
private:
	float value_in;
	bool y_calced;
public:
	float y;
	bool is_changed;
	ParamMapping(float dv) { value_in = dv; is_changed = true; y = 0.0f; y_calced = false; };
	bool setx(float x)
	{
		if (!y_calced)
		{
			value_in = x;
			is_changed = false;
			y_calced = true;
			return true;
		}
		else
		{
			if (x != value_in) {
				is_changed = true;
				value_in = x;
			}
			else
				is_changed = false;
			return is_changed;
		}
	};
	float getx() { return value_in; }
};

//more accurate rectifier ,but some times needs division
class Rectifier
{
private:
	float x0;
public:
	Rectifier() { x0 = 0.0f; }
	inline float f(float x)
	{
		return x >= 0.0f ? 0.5f * x * x : -0.5f * x * x;
	}
	inline float Run(float x)
	{
		register float t = fabs(x - x0) < 0.01f ? fabs(x) : ((f(x)-f(x0)) / (x - x0));
		x0 = x;
		return t;
	}
};

//sse abs implement
inline __m128 _mm_abs_ps(__m128 x)
{
	__m128 c0 = _mm_cmplt_ps(x, _mm_setzero_ps());//<0 -> 0xff...ff
	return _mm_add_ps(_mm_and_ps(c0, _mm_sub_ps(_mm_setzero_ps(), x)), _mm_andnot_ps(c0, x));
}


//LUT
class Lookup
{
private:
	int size;
	const float* T;
	float lb, ub,fsize,scale,xlb,xub;
	__m128 cp1, cn1, mxub, mxlb, mlb, mscale;
public:
	Lookup(int tsize, const float* tT, float tlb, const float tub);
	inline float F(float x)
	{
		register float tx = x;
		if (x > xub)
			tx = xub;
		if (x < xlb)
			tx = xlb;
		tx = (tx - lb) * scale;
		register int idx = (int)tx;
		tx -= (float)idx;
		register float L = T[idx];
		return (T[idx + 1] - L) * tx + L;
	}
	inline __m128 F4(__m128 x)
	{
		__m128 tx,t0,t1,c0;
		__m128i idx;
		int i0, i1, i2, i3;
		
		tx = x;

		c0 = _mm_cmplt_ps(tx, _mm_setzero_ps());//<0 -> ff

		tx = _mm_abs_ps(tx);

		tx = _mm_min_ps(tx, mxub);
		tx = _mm_max_ps(tx, mxlb);// do clip

		tx = _mm_mul_ps(_mm_sub_ps(tx, mlb), mscale);
		idx = _mm_cvtps_epi32(tx);
		t0 = _mm_cvtepi32_ps(idx);
		tx = _mm_sub_ps(tx,t0);

		t0 = _mm_set_ps(T[i3 = idx.m128i_i32[3]], T[i2 = idx.m128i_i32[2]], T[i1 = idx.m128i_i32[1]], T[i0 = idx.m128i_i32[0]]);

		t1 = _mm_set_ps(T[i3 + 1], T[i2 + 1], T[i1 + 1], T[i0 + 1]);

		tx = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(t1, t0), tx), t0);

		tx = _mm_add_ps(_mm_and_ps(c0, _mm_sub_ps(_mm_setzero_ps(), tx)), _mm_andnot_ps(c0, tx));//see SymF()

		return tx;

	}
};

class LDR
{
private:
	float y0, x0;
	float fsr,invfsr;
	float ratio_limit,last_ratio;
	RcRelease* lp0;
	Rectifier* rect;
	LPF *lp2;
	Lookup* LUTratio, * LUTinvRC;
public:
	float probe;
	LDR();
	void SetK(float fsr, float limf);
	inline float Run(float tVin)
	{
		register float invRC,oc,a;
		register float Vin = tVin;
		register float res;
		register float tratio;
		//Vin = rect->Run(Vin);
		Vin = (Vin >= 0.0f ? Vin : -Vin);//here , fabs is slower but dont know why 
		Vin = lp0->Run(Vin);// in fact , lp0 is a charge / A-R 'circuit'
		tratio = LUTratio->F(Vin);
		invRC = LUTinvRC->F(Vin);
		invRC *= (tratio <= y0 ? 17.14285714f : 1.0f);//the A-R RC constant in LDR has a nearly constantly ratio
		oc = invfsr * invRC;//here avoid a division
		a = oc;//in this condition alpha ~= omegaC
		y0 = a * tratio + (1.0f - a) * y0;//do the regular rc lowpass
		res = ratio_limit + (1.0f - ratio_limit) * y0;//here , keep the maximum compress ratio, like a resisitor series the LDR
		res = res > 1.0f ? 1.0f : res;
		res = lp2->Run(res);//make sure the 'vca' signal dont make much AM effect
		return res;
	}
};

//FIR downsampling & Upsampling both float and __m128 version
class DS4x_FIR
{
#define TAP_NUM 32
private:
	const float *h;
	float b[TAP_NUM];
	__declspec(align(16)) float b2[8][4];
	int hp;
	float x_1, x_2, x_3, x_4, x_5, x_6, x_7;
	__declspec(align(16)) float ta[4], tb[4];
	__m128 x_03, x_47;
	static __m128 ra0,  rb0,  ra1,  rb1,  ra2,  rb2,  ra3,  rb3;
	static __m128 _h0,  _h1,  _h2,  _h3,  _h4,  _h5,  _h6,  _h7;
	__m128 b21, b22, b23, b24, b25, b26, b27;
	//__m128* _mb[8];
public:
	static void Init(const float* ht)
	{

		__declspec(align(16)) float h20[4], h21[4], h22[4], h23[4], h24[4], h25[4], h26[4], h27[4];
		__declspec(align(16)) float ta0[4], tb0[4], ta1[4], tb1[4], ta2[4], tb2[4], ta3[4], tb3[4];

		int t = 0;
		h20[0] = ht[t + 0]; h20[1] = ht[t + 1]; h20[2] = ht[t + 2]; h20[3] = ht[t + 3]; t = 4 * 1;
		h21[0] = ht[t + 0]; h21[1] = ht[t + 1]; h21[2] = ht[t + 2]; h21[3] = ht[t + 3]; t = 4 * 2;
		h22[0] = ht[t + 0]; h22[1] = ht[t + 1]; h22[2] = ht[t + 2]; h22[3] = ht[t + 3]; t = 4 * 3;
		h23[0] = ht[t + 0]; h23[1] = ht[t + 1]; h23[2] = ht[t + 2]; h23[3] = ht[t + 3]; t = 4 * 4;
		h24[0] = ht[t + 0]; h24[1] = ht[t + 1]; h24[2] = ht[t + 2]; h24[3] = ht[t + 3]; t = 4 * 5;
		h25[0] = ht[t + 0]; h25[1] = ht[t + 1]; h25[2] = ht[t + 2]; h25[3] = ht[t + 3]; t = 4 * 6;
		h26[0] = ht[t + 0]; h26[1] = ht[t + 1]; h26[2] = ht[t + 2]; h26[3] = ht[t + 3]; t = 4 * 7;
		h27[0] = ht[t + 0]; h27[1] = ht[t + 1]; h27[2] = ht[t + 2]; h27[3] = ht[t + 3];

		_h0 = _mm_load_ps(h20); _h1 = _mm_load_ps(h21);
		_h2 = _mm_load_ps(h22); _h3 = _mm_load_ps(h23);
		_h4 = _mm_load_ps(h24); _h5 = _mm_load_ps(h25);
		_h6 = _mm_load_ps(h26); _h7 = _mm_load_ps(h27);

		ta0[0] = ht[0]; ta0[1] = ht[4]; ta0[2] = ht[8]; ta0[3] = ht[12];
		tb0[0] = ht[15]; tb0[1] = ht[11]; tb0[2] = ht[7]; tb0[3] = ht[3];
		ta1[0] = ht[1]; ta1[1] = ht[5]; ta1[2] = ht[9]; ta1[3] = ht[13];
		tb1[0] = ht[14]; tb1[1] = ht[10]; tb1[2] = ht[6]; tb1[3] = ht[2];
		ta2[0] = ht[2]; ta2[1] = ht[6]; ta2[2] = ht[10]; ta2[3] = ht[14];
		tb2[0] = ht[13]; tb2[1] = ht[9]; tb2[2] = ht[5]; tb2[3] = ht[1];
		ta3[0] = ht[3]; ta3[1] = ht[7]; ta3[2] = ht[11]; ta3[3] = ht[15];
		tb3[0] = ht[12]; tb3[1] = ht[8]; tb3[2] = ht[4]; tb3[3] = ht[0];

		ra0 = _mm_load_ps(ta0); rb0 = _mm_load_ps(tb0);
		ra1 = _mm_load_ps(ta1); rb1 = _mm_load_ps(tb1);
		ra2 = _mm_load_ps(ta2); rb2 = _mm_load_ps(tb2);
		ra3 = _mm_load_ps(ta3); rb3 = _mm_load_ps(tb3);

	}
	DS4x_FIR(const float *fir_table) {
		hp = 0;
		h = fir_table;

		for (int i = 0; i < TAP_NUM; i++)
		{
			b[i] = 0.0f;
		}

		float tr[4];
		for (int i = 0; i < TAP_NUM/4; i++)
		{
			int ti = i * 4;
			b2[i][0] = 0.0f; b2[i][1] = 0.0f;
			b2[i][2] = 0.0f; b2[i][3] = 0.0f;
			//_mb[i] = (__m128*)b2[i];
		}
		int t = 0;

		x_03 = _mm_setzero_ps();
		x_47 = _mm_setzero_ps();
		b21 = _mm_setzero_ps();
		b22 = _mm_setzero_ps();
		b23 = _mm_setzero_ps();
		b24 = _mm_setzero_ps();
		b25 = _mm_setzero_ps();
		b26 = _mm_setzero_ps();
		b27 = _mm_setzero_ps();

		x_1 = 0.0f; x_2 = 0.0f; x_3 = 0.0f; x_4 = 0.0f;
		x_5 = 0.0f; x_6 = 0.0f; x_7 = 0.0f;
	}
	//only for verify the sse version
	inline float RunDown(float x0, float x1, float x2, float x3)
	{
		float acc = 0.0f;
		int t,ti;
		hp -= 1;
		hp = (hp + 8) & 7;
		b2[hp][0] = x3;
		b2[hp][1] = x2;
		b2[hp][2] = x1;
		b2[hp][3] = x0;
		for (int i = 0; i < TAP_NUM/4; i++)
		{
			ti = i * 4;
			t = (hp + i) & 7;
			acc += b2[t][0] * h[ti];
			acc += b2[t][1] * h[ti + 1];
			acc += b2[t][2] * h[ti + 2];
			acc += b2[t][3] * h[ti + 3];
		}
		return acc;
	}
	inline void RunUp(float x, float* i0, float* i1, float* i2, float* i3)
	{
		float x_0 = x * 4.0f;
		*i0 = x_0 * h[0] + x_1 * h[4] + x_2 * h[8] + x_3 * h[12] + x_4 * h[15] + x_5 * h[11] + x_6 * h[7] + x_7 * h[3];
		*i1 = x_0 * h[1] + x_1 * h[5] + x_2 * h[9] + x_3 * h[13] + x_4 * h[14] + x_5 * h[10] + x_6 * h[6] + x_7 * h[2];
		*i2 = x_0 * h[2] + x_1 * h[6] + x_2 * h[10] + x_3 * h[14] + x_4 * h[13] + x_5 * h[9] + x_6 * h[5] + x_7 * h[1];
		*i3 = x_0 * h[3] + x_1 * h[7] + x_2 * h[11] + x_3 * h[15] + x_4 * h[12] + x_5 * h[8] + x_6 * h[4] + x_7 * h[0];//32fmul
		x_7 = x_6;
		x_6 = x_5;
		x_5 = x_4;
		x_4 = x_3;
		x_3 = x_2;
		x_2 = x_1;
		x_1 = x_0;
	}

	inline __m128 RunUpSIMD(float x)
	{
		__m128 r0,r1,r2,s1,ret;
		float x_0 = x * 4.0f,x_4;

		x_4 = x_03.m128_f32[3];//here can be optimized

		x_03 = _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(x_03), 4));//do the 'register shift'(cast seems a empty instruction?)
		x_47 = _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(x_47), 4));
		
		x_47.m128_f32[0] = x_4;
		x_03.m128_f32[0] = x_0;

		r0 = _mm_hadd_ps(_mm_mul_ps(x_03, ra0), _mm_mul_ps(x_47, rb0));
		r0 = _mm_hadd_ps(r0, r0);
		r1 = _mm_hadd_ps(r0, r0);//r1[0] is the sum0

		r0 = _mm_hadd_ps(_mm_mul_ps(x_03, ra1), _mm_mul_ps(x_47, rb1));
		r0 = _mm_hadd_ps(r0, r0);
		r2 = _mm_hadd_ps(r0, r0);//r2[0] is the sum1

		s1 = _mm_unpacklo_ps(r1, r2);//s1 = {sum0,sum1,...}

		r0 = _mm_hadd_ps(_mm_mul_ps(x_03, ra2), _mm_mul_ps(x_47, rb2));
		r0 = _mm_hadd_ps(r0, r0);
		r1 = _mm_hadd_ps(r0, r0);

		r0 = _mm_hadd_ps(_mm_mul_ps(x_03, ra3), _mm_mul_ps(x_47, rb3));
		r0 = _mm_hadd_ps(r0, r0);
		r2 = _mm_hadd_ps(r0, r0);

		ret = _mm_movelh_ps(s1, _mm_unpacklo_ps(r1, r2));//A = {sum0,sum1,*,*} B = {sum2,sum3,*,*}
		//ret = {sum0,sum1,sum2,sum3}
		return ret;
	}
	inline float RunDownSIMD(__m128 x)
	{
		__declspec(align(16)) float acc = 0.0f;
		__m128 r0, r1, r2, r3;
		//x={x0,x1,x2,x3}
		x = _mm_shuffle_ps(x, x, _MM_SHUFFLE(0,1,2,3));//reverse 
		r0 = _mm_hadd_ps(_mm_mul_ps(_h0, x), _mm_mul_ps(_h1, b21));
		r1 = _mm_hadd_ps(_mm_mul_ps(_h2, b22), _mm_mul_ps(_h3, b23));
		r2 = _mm_hadd_ps(_mm_mul_ps(_h4, b24), _mm_mul_ps(_h5, b25));
		r3 = _mm_hadd_ps(_mm_mul_ps(_h6, b26), _mm_mul_ps(_h7, b27));
		r0 = _mm_hadd_ps(_mm_hadd_ps(r0, r1), _mm_hadd_ps(r2, r3));
		r0 = _mm_hadd_ps(r0, r0);
		r0 = _mm_hadd_ps(r0, r0);
		_mm_store_ss(&acc, r0);

		b27 = b26;
		b26 = b25;
		b25 = b24;
		b24 = b23;
		b23 = b22;
		b22 = b21;
		b21 = x;
		return acc;
	}
};


inline float SymF(float x, Lookup* mlut)
{
	return x >= 0.0f ? mlut->F(x) : -mlut->F(-x);
}

//i dont know if 0.05f is right but the method will add 2nd harmomic, also it seems like the transfer curve
inline float tube(float x)
{
	return x * (1.0f + 0.05f * x);
}

inline __m128 tubeSIMD(__m128 x)
{
	__declspec(align(16)) float fc1 = 1.0f, fc2 = 0.05f;
	__m128 c1, c2, rx = x;
	c1 = _mm_load1_ps(&fc1);
	c2 = _mm_load1_ps(&fc2);
	rx = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(c2, rx), c1),rx);
	return rx;
}

class QSat
{
private:
	DS4x_FIR *fir;
	float lfsr;
public:
	QSat::QSat(const float *fir_table)
	{
		fir = new DS4x_FIR(fir_table);
	}
	//only for comparison
	inline float Run(Lookup* lut, float x)
	{
		float i0, i1, i2, i3 = x;
		fir->RunUp(x, &i0, &i1, &i2, &i3);

		i0 = SymF(tube(i0), lut);//4mul
		i1 = SymF(tube(i1), lut);
		i2 = SymF(tube(i2), lut);
		i3 = SymF(tube(i3), lut);

		i0 = fir->RunDown(i0, i1, i2, i3);

		return i0;
	}
	inline float Run2(Lookup* lut, float x)
	{
		float i0, i1, i2, i3 = x;
		__m128 mx;

		mx = fir->RunUpSIMD(x);
		mx = tubeSIMD(mx);
		mx = lut->F4(mx);
		i0 = fir->RunDownSIMD(mx);

		return i0;
	}
	void SetK(float fsr)
	{ 
		;
	}
};

