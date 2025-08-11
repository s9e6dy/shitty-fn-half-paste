#pragma once
#pragma comment (lib, "d3d9.lib")

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx9.h"
#include "Imgui/imgui_impl_win32.h"
#include <emmintrin.h>
#include <immintrin.h>



int Width;
int Height;


enum EFortItemTier : char
{
	No_Tier = 0,
	I = 1,
	II = 2,
	III = 3,
	IV = 4,
	V = 5,
	VI = 6,
	VII = 7,
	VIII = 8,
	IX = 9,
	X = 10,
	NumItemTierValues = 11,
	EFortItemTier_MAX = 12
};

struct FLinearColor
{
	FLinearColor() : r(0.f), g(0.f), b(0.f), a(0.f) {}
	FLinearColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	operator bool() { return bool(this->r || this->g || this->b || this->a); }
	friend bool operator	== (const FLinearColor& a, const FLinearColor& b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }

	void Clear()
	{
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}

	ImColor GetColor()
	{
		return { r * 100, g * 100, b * 100, a * 100 };
	}

	void Setup(int r, int g, int b, int a)
	{
		this->r = r / 100;
		this->g = g / 100;
		this->b = b / 100;
		this->a = a / 100;
	}

	float r, g, b, a;
};

class FVector
{
public:
	double x, y, z;

	FVector()
	{
		x = y = z = 0.0f;
	}

	FVector(double X, double Y, double Z)
	{
		x = X; y = Y; z = Z;
	}

	FVector(double XYZ)
	{
		x = XYZ; y = XYZ; z = XYZ;
	}

	FVector(double* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	FVector(const double* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	__forceinline FVector& operator=(const FVector& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	__forceinline FVector& operator=(const double* v)
	{
		x = v[0]; y = v[1]; z = v[2]; return *this;
	}

	__forceinline double& operator[](int i)
	{
		return ((double*)this)[i];
	}

	__forceinline double operator[](int i) const
	{
		return ((double*)this)[i];
	}

	__forceinline FVector& operator+=(const FVector& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	__forceinline FVector& operator-=(const FVector& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	__forceinline FVector& operator*=(const FVector& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	__forceinline FVector& operator/=(const FVector& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	__forceinline FVector& operator+=(double v)
	{
		x += v; y += v; z += v; return *this;
	}

	__forceinline FVector& operator-=(double v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	__forceinline FVector& operator*=(double v)
	{
		x *= v; y *= v; z *= v; return *this;
	}

	__forceinline FVector& operator/=(double v)
	{
		x /= v; y /= v; z /= v; return *this;
	}

	__forceinline FVector operator-() const
	{
		return FVector(-x, -y, -z);
	}

	__forceinline FVector operator+(const FVector& v) const
	{
		return FVector(x + v.x, y + v.y, z + v.z);
	}

	__forceinline FVector operator-(const FVector& v) const
	{
		return FVector(x - v.x, y - v.y, z - v.z);
	}

	__forceinline FVector operator*(const FVector& v) const
	{
		return FVector(x * v.x, y * v.y, z * v.z);
	}

	__forceinline FVector operator/(const FVector& v) const
	{
		return FVector(x / v.x, y / v.y, z / v.z);
	}

	__forceinline FVector operator+(double v) const
	{
		return FVector(x + v, y + v, z + v);
	}

	__forceinline FVector operator-(double v) const
	{
		return FVector(x - v, y - v, z - v);
	}

	__forceinline FVector operator*(double v) const
	{
		return FVector(x * v, y * v, z * v);
	}

	__forceinline FVector operator/(double v) const
	{
		return FVector(x / v, y / v, z / v);
	}

	__forceinline double Distance(FVector v)
	{
		return sqrtf((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y) + (v.z - z) * (v.z - z));
	}

	__forceinline double Length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	__forceinline double Dot(const FVector& v) const
	{
		return (x * v.x + y * v.y + z * v.z);
	}
};

class FVector2D
{
public:
	double x, y;

	FVector2D()
	{
		x = y = 0.0f;
	}

	FVector2D(double X, double Y)
	{
		x = X; y = Y;;
	}

	FVector2D(double XY)
	{
		x = XY; y = XY;
	}

	FVector2D(double* v)
	{
		x = v[0]; y = v[1];
	}

	FVector2D(const double* v)
	{
		x = v[0]; y = v[1];
	}

	__forceinline bool IsInScreen()
	{
		if (((x <= 0 || x > Width) && (y <= 0 || y > Height)) || ((x <= 0 || x > Width) || (y <= 0 || y > Height))) {
			return false;
		}
		else {
			return true;
		}
	}

	__forceinline operator ImVec2() const { return ImVec2(x, y); }

	__forceinline FVector2D& operator=(const FVector2D& v)
	{
		x = v.x; y = v.y; return *this;
	}

	__forceinline FVector2D& operator=(const double* v)
	{
		x = v[0]; y = v[1]; return *this;
	}

	__forceinline double& operator[](int i)
	{
		return ((double*)this)[i];
	}

	__forceinline double operator[](int i) const
	{
		return ((double*)this)[i];
	}

	__forceinline FVector2D& operator+=(const FVector2D& v)
	{
		x += v.x; y += v.y; return *this;
	}

	__forceinline FVector2D& operator-=(const FVector2D& v)
	{
		x -= v.x; y -= v.y; return *this;
	}

	__forceinline FVector2D& operator*=(const FVector2D& v)
	{
		x *= v.x; y *= v.y; return *this;
	}

	__forceinline FVector2D& operator/=(const FVector2D& v)
	{
		x /= v.x; y /= v.y; return *this;
	}

	__forceinline FVector2D& operator+=(double v)
	{
		x += v; y += v; return *this;
	}

	__forceinline FVector2D& operator-=(double v)
	{
		x -= v; y -= v; return *this;
	}

	__forceinline FVector2D& operator*=(double v)
	{
		x *= v; y *= v; return *this;
	}

	__forceinline FVector2D& operator/=(double v)
	{
		x /= v; y /= v; return *this;
	}

	__forceinline FVector2D operator-() const
	{
		return FVector2D(-x, -y);
	}

	__forceinline FVector2D operator+(const FVector2D& v) const
	{
		return FVector2D(x + v.x, y + v.y);
	}

	__forceinline FVector2D operator-(const FVector2D& v) const
	{
		return FVector2D(x - v.x, y - v.y);
	}

	__forceinline FVector2D operator*(const FVector2D& v) const
	{
		return FVector2D(x * v.x, y * v.y);
	}

	__forceinline FVector2D operator/(const FVector2D& v) const
	{
		return FVector2D(x / v.x, y / v.y);
	}

	__forceinline FVector2D operator+(double v) const
	{
		return FVector2D(x + v, y + v);
	}

	__forceinline FVector2D operator-(double v) const
	{
		return FVector2D(x - v, y - v);
	}

	__forceinline FVector2D operator*(double v) const
	{
		return FVector2D(x * v, y * v);
	}

	__forceinline FVector2D operator/(double v) const
	{
		return FVector2D(x / v, y / v);
	}

	__forceinline double Distance(FVector2D v)
	{
		return sqrtf((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y));
	}

	__forceinline double Length() const
	{
		return sqrtf(x * x + y * y);
	}

	__forceinline double Dot(const FVector2D& v) const
	{
		return (x * v.x + y * v.y);
	}
};

struct FQuat
{
	double x;
	double y;
	double z;
	double w;
};

struct FTransform
{
	FQuat rot;
	FVector translation;
	FVector scale;
	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

namespace ExternalUtils {
	inline float custom_sqrtf(float _X)
	{

		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(_X)));
	}

	static inline void custom_Memcpy(const void* dstp, const void* srcp, UINT len)
	{

		ULONG* dst = (ULONG*)dstp;
		ULONG* src = (ULONG*)srcp;
		UINT i, tail;

		for (i = 0; i < (len / sizeof(ULONG)); i++)
			*dst++ = *src++;

		tail = len & (sizeof(ULONG) - 1);
		if (tail) {

			UCHAR* dstb = (UCHAR*)dstp;
			UCHAR* srcb = (UCHAR*)srcp;

			for (i = len - tail; i < len; i++)
				dstb[i] = srcb[i];
		}
	}



	inline float custom_sinf(float _X)
	{

		return _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(_X)));
	}

	inline float custom_cosf(float _X)
	{

		return _mm_cvtss_f32(_mm_cos_ps(_mm_set_ss(_X)));
	}

	inline float custom_acosf(float _X)
	{

		return _mm_cvtss_f32(_mm_acos_ps(_mm_set_ss(_X)));
	}

	inline float custom_tanf(float _X)
	{

		return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(_X)));
	}

	inline float custom_atan2f(float _X, float _Y)
	{

		return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(_X), _mm_set_ss(_Y)));
	}

	inline float custom_pow(float _X, float _Y)
	{

		return _mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y)));
	}

	inline float custom_pow(float _X)
	{

		return _mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X)));
	}

	inline float custom_asinf(float _X)
	{

		return _mm_cvtss_f32(_mm_asin_ps(_mm_set_ss(_X)));
	}

	float custom_fabsf(float x) {

		__m128 x_vec = _mm_set_ss(x);
		x_vec = _mm_and_ps(x_vec, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
		return _mm_cvtss_f32(x_vec);
	}

	static void* custom_memset(void* dst0, int c0, unsigned int len)
	{
		unsigned int i;
		unsigned int fill;
		unsigned int chunks = len / sizeof(fill);
		char* char_dest = (char*)dst0;
		unsigned int* uint_dest = (unsigned int*)dst0;
		fill = (c0 << 24) + (c0 << 16) + (c0 << 8) + c0;

		for (i = len; i > chunks * sizeof(fill); i--) {
			char_dest[i - 1] = c0;
		}

		for (i = chunks; i > 0; i--) {
			uint_dest[i - 1] = fill;
		}

		return dst0;
	}

	inline int custom_compare(const char* X, const char* Y)
	{

		while (*X && *Y) {
			if (*X != *Y) {
				return 0;
			}
			X++;
			Y++;
		}

		return (*Y == '\0');
	}

	inline int custom_wcompare(const wchar_t* X, const wchar_t* Y)
	{

		while (*X && *Y) {
			if (*X != *Y) {
				return 0;
			}
			X++;
			Y++;
		}

		return (*Y == L'\0');
	}

	inline const wchar_t* custom_wcsstr(const wchar_t* X, const wchar_t* Y)
	{

		while (*X != L'\0') {
			if ((*X == *Y) && custom_wcompare(X, Y)) {
				return X;
			}
			X++;
		}
		return NULL;
	}

	inline const char* custom_strstr(const char* X, const char* Y)
	{
		while (*X != '\0') {
			if ((*X == *Y) && custom_compare(X, Y)) {
				return X;
			}
			X++;
		}
		return NULL;
	}

	inline int custom_strlen(const char* string)
	{

		int cnt = 0;
		if (string)
		{
			for (; *string != 0; ++string) ++cnt;
		}
		return cnt;
	}

	inline int custom_wcslen(const wchar_t* string)
	{

		int cnt = 0;
		if (string)
		{
			for (; *string != 0; ++string) ++cnt;
		}
		return cnt;
	}
}