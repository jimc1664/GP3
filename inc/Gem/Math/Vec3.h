#ifndef GEM_MATH_VEC3_H
#define GEM_MATH_VEC3_H

#include "../Basic/Math.h"
#include "../Basic/Tuple.h"
#include "Randomizer.h"

namespace Gem {

template<class T> class mat3x4_T;
template<class T> class quat_T;

class Randomizer;

template<class typ> class vec3_T : public Convertable<vec3_T<typ>>, public tuple<typ, typ, typ>  {
	typedef vec3_T<typ> v3;
public:
	v3() {}
	//Template3 v3( T &&a, T2 &&b, T3 &&c) : tuple(a,b,c) {}

	v3( const typ &a, const typ &b, const typ &c) : tuple(a,b,c) {}

	Template1 v3(const T &a  OnlyIF( Is_BasicType<T>) )			: tuple(a,a,a) {  }
	Template1 v3(const T &a	 OnlyIF2( Is_ValidConversion,v3,T) ): tuple(convert<v3, T>(a)) {  }
	Template3 v3(const tuple<T, T2, T3> &t) : tuple((typ)t.x, (typ)t.y, (typ)t.z) {}

	v3( const tuple<typ,typ> &t, const typ &c = typ(0.0) ) : tuple(t.x, t.y, c) {}
	v3( const tuple<typ,typ,typ,typ> &t ) : tuple(t.x, t.y,t.z) {}

	//using Convertable<v3>::operator=;
	//Template1A v3& operator = (const T1A &a) { return assign( convert<v3,T1A>(a) ); }
	//Template1A v3& operator = ( T1A &&a) { return assign( convert<v3,T1A>(a) ); }
private: //friend struct Convertable<v3>;
	//v3& assign( v3 &&a) { x = a.x; y = a.y; z = a.z; return *this; }
public:
	//v3& operator =( v3 &&a) { x = a.x; y = a.y; z = a.z; return *this; }
	/*v3(const typ &i) : tuple(i, i) {}				CtorStraint(T,Is_BasicType)
	v3(const typ &i, const typ &j) : tuple(i, j) {}

	v3(const v3 &other) : tuple(other) {}
	Template2 v3(const tuple<T, T2> &t) : tuple((typ)t.a, (typ)t.b) {}

	*/
	typ& operator[](const u32 &i) { Assume(i<3); return (&x)[i]; }
	const typ& operator[](const u32 &i) const { Assume(i<3); return (&x)[i]; }

	v3 operator-() const { return v3(-x, -y, -z); }

	v3 operator+(const v3 &i) const { return v3(x+i.x, y+i.y, z+i.z); }
	v3 operator+(const typ &i) const { return v3(x+i, y+i, z+i); }
	Template1 vec3_T& operator+= (const T &i) { *this = *this + i; return *this; }

	v3 operator-(const v3 &i) const { return v3(x-i.x, y-i.y, z-i.z); }
	v3 operator-(const typ &i) const { return v3(x-i, y-i, z-i); }
	Template1 vec3_T& operator-= (const T &i) { *this = *this - i; return *this; }



	v3 operator/(const v3 &i) const { return v3(x/i.x, y/i.y, z/i.z); }
	v3 operator/(typ i) const { i = (typ)1.0 / i;  return v3(x*i, y*i, z*i); }
	Template1 vec3_T& operator/= (const T &i) { *this = *this / i; return *this; }

	template<typename T> v3 operator* (const T &a) const;
	template<> v3 operator*(const v3 &i) const { return v3(x*i.x, y*i.y, z*i.z); }
	template<> v3 operator*(const typ &i) const { return v3(x*i, y*i, z*i); }
	template<> v3 operator*(const quat_T<typ> &b) const {
		v3 a = *this;
		typ x2 = b.x + b.x, y2 = b.y + b.y, z2 = b.z + b.z;
		typ xx = x2*b.x, xy = x2*b.y, xz = x2*b.z, xw = x2*b.w;
		typ yy = y2*b.y, yz = y2*b.z, yw = y2*b.w;
		typ zw = z2*b.w, zz = z2*b.z;


		typ m_00 = typ(1.0) - (yy + zz);
		typ m_01 = (xy - zw);
		typ m_02 = (xz + yw);

		typ m_10 = (xy + zw);
		typ m_11 = typ(1.0) - (xx + zz);
		typ m_12 = (yz - xw);

		typ m_20 = (xz - yw);
		typ m_21 = (yz + xw);
		typ m_22 = typ(1.0) - (xx + yy);

		vec3_T<typ> v;
		v.x = a.x*m_00 + a.y*m_10 + a.z*m_20;
		v.y = a.x*m_01 + a.y*m_11 + a.z*m_21;
		v.z = a.x*m_02 + a.y*m_12 + a.z*m_22;
		return  v;
	}
	template<typename T> v3 operator*( const mat3x4_T<T> &a ) const { return *this *a.Rot + a.Pos; }

	Template1 vec3_T& operator*= (const T &i) { *this = *this * i; return *this; }

	/*
	Template1 vec3_T& set(const T &i) { *this = i; return *this; }

	const typ& minimum() const { return min(x, y); }
	const typ& maximum() const { return max(x, y); }
*/
	const typ sqrLeng() const { return x*x + y*y +z*z; }
	typ leng() const { return sqrt(sqrLeng()); }

	vec3_T& normalise() { *this *= invSqrt(sqrLeng()); return *this; }
	v3 getNormal() const { v3 v(*this); return v.normalise(); }

	typ dot(const v3 &j) const { return x*j.x + y*j.y + z*j.z; }
	v3 cross( const v3 &a ) const { return v3( y*a.z - z*a.y, z*a.x - x*a.z, x*a.y - y*a.x ); }

	static v3 inCube( Randomizer& r ) {
		return  v3(r.range(-1.0f,1.0f), r.range(-1.0f, 1.0f), r.range(-1.0f, 1.0f));
	}
};


Template1 vec3_T<T>	floor(const vec3_T<T> &i)											{ return vec3_T<T>(floor(i.x), floor(i.y), floor(i.z)); }
Template1 vec3_T<T>	ceil(const vec3_T<T> &i)											{ return vec3_T<T>(ceil(i.x), ceil(i.y), ceil.z); }

Template1 vec3_T<T>	abs(const vec3_T<T> &i)												{ return vec3_T<T>(abs(i.x), abs(i.y), abs(i.z)); }
Template1 vec3_T<T>	max(const vec3_T<T> &i, const vec3_T<T> &j)							{ return vec3_T<T>(max(i.x, j.x), max(i.y, j.y), max(i.z, j.z)); }
Template1 vec3_T<T>	min(const vec3_T<T> &i, const vec3_T<T> &j)							{ return vec3_T<T>(min(i.x, j.x), min(i.y, j.y), min(i.z, j.z)); }
Template1 vec3_T<T>	clamp(const vec3_T<T> &i, const vec3_T<T> &j, const vec3_T<T>& c)		{ return vec3_T<T>(clamp(i.x, j.x, c.x), clamp(i.y, j.y, c.y), clamp(i.z, j.z, c.z)); }
/*
Template1 bool		ifMin(vec3_T<T> &i, const vec3_T<T> &j)								{ return ifMin(i.x, j.x) | ifMin(i.y, j.y); }
Template1 bool		ifMax(vec3_T<T> &i, const vec3_T<T> &j)								{ return ifMax(i.x, j.x) | ifMax(i.y, j.y); }

Template1 bool		isEqual(const vec3_T<T>& i, const vec3_T<T>& j, const T& epsilon)	{ return (i - j).sqrLeng() <= epsilon; }

Template1 void minMax(vec3_T<T> &min, vec3_T<T> &max)										{ minMax(min.x, max.x); minMax(min.y, max.y); }
Template1 void minMax(vec3_T<T> &min, vec3_T<T> &max, const T &c)							{ minMax(min.a, max.a, c.a); minMax(min.b, max.b, c.b); }
Template1 void minMax(vec3_T<T> &min, vec3_T<T> &max, const T &c, const T &d)				{ minMax(min.a, max.a, c.a, d.a); minMax(min.b, max.b, c.b, d.b); }
Template1 void minMax(vec3_T<T> &min, vec3_T<T> &max, const T &c, const T &d, const T &e)	{ minMax(min.a, max.a, c.a, d.a, e.a); minMax(min.b, max.b, c.b, d.b, e.b); }

*/

Template1 T dot(const vec3_T<T> &i, const vec3_T<T> &j) { return i.dot(j); }
Template1 vec3_T<T> cross(const vec3_T<T> &i, const vec3_T<T> &j) { return i.cross(j); }


typedef vec3_T<s32> vec3s;
typedef vec3_T<u32> vec3u;
typedef vec3_T<f32> vec3f;
typedef vec3_T<f64> vec3d;

typedef vec3_T<s16> vec3s16;
typedef vec3_T<u16> vec3u16;
typedef vec3_T<s8> vec3s8;
typedef vec3_T<u8> vec3u8;

}

#endif //GEM_MATH_VEC3_H