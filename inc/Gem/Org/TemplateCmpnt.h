#ifndef GEM_ORG_TEMPLATECMPNT_H
#define GEM_ORG_TEMPLATECMPNT_H

#include "../Basic.h"

namespace Gem {


template< typename Typ1, typename Typ2, typename Typ3 = Nothing, typename Typ4 = Nothing, typename Typ5 = Nothing > struct cRefTuple;
template< typename Typ1, typename Typ2> struct cRefTuple<Typ1, Typ2, Nothing, Nothing, Nothing> {
	cRefTuple() {}
	cRefTuple(const Typ1 &A, const Typ2 &B) : a(A), b(B) {}
	cRefTuple(cRefTuple &&t) : a(t.a), b(t.b) {}

	operator const Typ1*() const { return &a; }
	operator const Typ2*() const { return &b; }
	Template1 const T& get() const { return *static_cast<const T*>(*this); }

	//template<typename In> static refTuple castFrom( In *in ) { refTuple( *static_cast<Typ1*>(in) ); }

	const Typ1& a;		const Typ2& b;
};
template< typename Typ1, typename Typ2> cRefTuple<Typ1, Typ2> rTpl(const Typ1 &A, const Typ2 &B) { return cRefTuple<Typ1, Typ2>(A, B); }


template< typename Typ1, typename Typ2, typename Typ3 = Nothing, typename Typ4 = Nothing, typename Typ5 = Nothing > struct refTuple;
template< typename Typ1, typename Typ2> struct refTuple<Typ1, Typ2, Nothing, Nothing, Nothing> {
	refTuple() {}
	refTuple(Typ1 &A, Typ2 &B) : a(A), b(B) {}
	refTuple(refTuple &&t) : a(t.a), b(t.b) {}

	operator Typ1*() { return &a; }
	operator Typ2*() { return &b; }
	operator const Typ1*() const { return &a; }
	operator const Typ2*() const { return &b; }
	Template1 T& get() { return *static_cast<T*>(*this); }
	Template1 const T& get() const { return *static_cast<const T*>(*this); }

	template<typename In> static refTuple castFrom( In *in ) { return refTuple( *static_cast<Typ1*>(in), *static_cast<Typ2*>(in) ); }

	Typ1& a;		Typ2& b;
};
template< typename Typ1, typename Typ2> refTuple<Typ1, Typ2> rTpl(const Typ1 &A, const Typ2 &B) { return refTuple<Typ1, Typ2>(A, B); }

template< typename Typ1, typename Typ2, typename Typ3> struct refTuple<Typ1, Typ2, Typ3, Nothing, Nothing> {
	refTuple() {}
	refTuple(Typ1 &A, Typ2 &B, Typ3 &C) : a(A), b(B), c(C) {}
	refTuple(refTuple &&t) : a(t.a), b(t.b), c(t.c) {}
	refTuple(const refTuple &t) : a(t.a), b(t.b), c(t.c) {}

//	template<typename In> refTuple( In &in ) : a(  *static_cast<Typ1*>(&in) ), b(  *static_cast<Typ2*>(&in) ), c(  *static_cast<Typ3*>(&in) ) {}

	operator Typ1*() { return &a; }
	operator Typ2*() { return &b; }
	operator Typ3*() { return &c; }
	operator const Typ1*() const { return &a; }
	operator const Typ2*() const { return &b; }
	operator const Typ3*() const { return &c; }
	Template1 T& get() { return *static_cast<T*>(*this); }
	Template1 const T& get() const { return *static_cast<const T*>(*this); }

	template<typename In>  static refTuple castFrom( In *in ) { return refTuple( *static_cast<Typ1*>(in), *static_cast<Typ2*>(in), *static_cast<Typ3*>(in) ); }

	Typ1& a;	Typ2& b;	Typ3& c;
};
//template< typename Typ1, typename Typ2> refTuple<Typ1, Typ2> rTpl(const Typ1 &A, const Typ2 &B) { return refTuple<Typ1, Typ2>(A, B); }

template< typename Typ1, typename Typ2, typename Typ3, typename Typ4> struct refTuple<Typ1, Typ2, Typ3, Typ4, Nothing> {
	refTuple() {}
	refTuple(Typ1 &A, Typ2 &B, Typ3 &C, Typ4 &D) : a(A), b(B), c(C), d(D) {}
	refTuple(refTuple &&t) : a(t.a), b(t.b), c(t.c), d(t.d) {}

	//template<typename In> refTuple( In &in ) : a(  *static_cast<Typ1*>(&in) ), b(  *static_cast<Typ2*>(&in) ), c(  *static_cast<Typ3*>(&in) ), d(  *static_cast<Typ4*>(&in) ) {}

	operator Typ1*() { return &a; }
	operator Typ2*() { return &b; }
	operator Typ3*() { return &c; }
	operator Typ4*() { return &d; }
	operator const Typ1*() const { return &a; }
	operator const Typ2*() const { return &b; }
	operator const Typ3*() const { return &c; }
	operator const Typ4*() const { return &d; }
	Template1 T& get() { return *static_cast<T*>(*this); }
	Template1 const T& get() const { return *static_cast<const T*>(*this); }

	template<typename In>  static refTuple castFrom( In *in ) { return refTuple( *static_cast<Typ1*>(in), *static_cast<Typ2*>(in), *static_cast<Typ3*>(in), *static_cast<Typ4*>(in) ); }

	Typ1& a;	Typ2& b;	Typ3& c;	Typ4& d;
};

template< typename Out, typename In > Out prm_cast( In *i ) {
	return Out::castFrom(i);
}

template<typename Out> struct PrmHelper { template<typename In> static Out cast( In * i ) { return Out::castFrom(i); } };  //todo - Out must be some form of RefTuple
template<> struct PrmHelper<Nothing> { template<typename In> static Nothing cast( In * i ) { return Nothing(); } };
template<typename Out> struct PrmHelper<Out&> { template<typename In> static Out& cast(In * i) { return *static_cast<Out*>(i); } };


template<typename Top, typename Cmpnt > struct TopIFFor : public Cmpnt { };   //this could/should be clear

template< typename Typ1, typename Typ2 = Nothing, typename Typ3 = Nothing, typename Typ4 = Nothing, typename Typ5 = Nothing, typename Typ6 = Nothing, typename Typ7 = Nothing > struct dpndnC;
template< typename Typ1> struct dpndnC<Typ1, Nothing, Nothing, Nothing, Nothing, Nothing, Nothing> {
	struct Comp :public TopIFFor<Comp,Typ1> {
		Template1 void procForAll( T &cntx ) {
			cntx.proc(*static_cast<Typ1*>(this), PrmHelper<typename Typ1::Prm>::cast(this));
		}
	};
	Template1 struct Join {
		typedef dpndnC<Typ1, T> R;
	};
	template<> struct Join<Nothing> { typedef dpndnC R; };
	template<> struct Join<Typ1> { typedef dpndnC R; };
	Template1 struct JoinTo {
		typedef typename T::template Join<Typ1>::R R; 
	};
};


template< typename Typ1, typename Typ2> struct dpndnC<Typ1, Typ2, Nothing, Nothing, Nothing, Nothing, Nothing> {
	struct Comp : public TopIFFor<Comp,Typ1>, public TopIFFor<Comp,Typ2> {
		Template1 void procForAll( T &cntx ) {
			cntx.proc(*static_cast<Typ1*>(this), PrmHelper<typename Typ1::Prm>::cast(this));
			cntx.proc(*static_cast<Typ2*>(this), PrmHelper<typename Typ2::Prm>::cast(this));	
		}
	};
	Template1 struct Join {
		typedef dpndnC<Typ1, Typ2, T> R;
	};
	template<> struct Join<Nothing> { typedef dpndnC R; };
	template<> struct Join<Typ1> { typedef dpndnC R; };
	template<> struct Join<Typ2> { typedef dpndnC R; };
	Template1 struct JoinTo {
		typedef typename T::template Join<Typ1>::R _R1;
		typedef typename _R1::template Join<Typ2>::R R;
	};
};

template< typename Typ1, typename Typ2, typename Typ3> struct dpndnC<Typ1, Typ2, Typ3, Nothing, Nothing, Nothing, Nothing> {
	struct Comp : public TopIFFor<Comp,Typ1>, public TopIFFor<Comp,Typ2>, public TopIFFor<Comp,Typ3> {
		Template1 void procForAll( T &cntx ) {
			cntx.proc(*static_cast<Typ1*>(this), PrmHelper<typename Typ1::Prm>::cast(this) );
			cntx.proc(*static_cast<Typ2*>(this), PrmHelper<typename Typ2::Prm>::cast(this));	
			cntx.proc(*static_cast<Typ3*>(this), PrmHelper<typename Typ3::Prm>::cast(this));	
		}
	};
	Template1 struct Join {
		typedef dpndnC<Typ1, Typ2, Typ3, T> R;
	};
	template<> struct Join<Nothing> { typedef dpndnC R; };
	template<> struct Join<Typ1> { typedef dpndnC R; };
	template<> struct Join<Typ2> { typedef dpndnC R; };
	template<> struct Join<Typ3> { typedef dpndnC R; };
	Template1 struct JoinTo {
		typedef typename T::template Join<Typ1>::R _R1;
		typedef typename _R1::template Join<Typ2>::R _R2;
		typedef typename _R2::template Join<Typ3>::R R;
	};
};
template< typename Typ1, typename Typ2, typename Typ3, typename Typ4> struct dpndnC<Typ1, Typ2, Typ3, Typ4, Nothing, Nothing, Nothing> {
	struct Comp : public TopIFFor<Comp, Typ1>, public TopIFFor<Comp, Typ2>, public TopIFFor<Comp, Typ3>, public TopIFFor<Comp, Typ4> {
		Template1 void procForAll(T &cntx) {
			cntx.proc(*static_cast<Typ1*>(this), PrmHelper<typename Typ1::Prm>::cast(this));
			cntx.proc(*static_cast<Typ2*>(this), PrmHelper<typename Typ2::Prm>::cast(this));
			cntx.proc(*static_cast<Typ3*>(this), PrmHelper<typename Typ3::Prm>::cast(this));
			cntx.proc(*static_cast<Typ4*>(this), PrmHelper<typename Typ4::Prm>::cast(this));
		}
	};
	Template1 struct Join {
		typedef dpndnC<Typ1, Typ2, Typ3, Typ4, T> R;
	};
	template<> struct Join<Nothing> { typedef dpndnC R; };
	template<> struct Join<Typ1> { typedef dpndnC R; };
	template<> struct Join<Typ2> { typedef dpndnC R; };
	template<> struct Join<Typ3> { typedef dpndnC R; };
	template<> struct Join<Typ4> { typedef dpndnC R; };
	Template1 struct JoinTo {
		typedef typename T::template Join<Typ1>::R _R1;
		typedef typename _R1::template Join<Typ2>::R _R2;
		typedef typename _R2::template Join<Typ3>::R _R3;
		typedef typename _R3::template Join<Typ4>::R R;
	};
};
template< typename Typ1, typename Typ2, typename Typ3, typename Typ4, typename Typ5> struct dpndnC<Typ1, Typ2, Typ3, Typ4, Typ5, Nothing, Nothing> {
	struct Comp : public TopIFFor<Comp, Typ1>, public TopIFFor<Comp, Typ2>, public TopIFFor<Comp, Typ3>, public TopIFFor<Comp, Typ4>, public TopIFFor<Comp, Typ5> {
		Template1 void procForAll(T &cntx) {
			cntx.proc(*static_cast<Typ1*>(this), PrmHelper<typename Typ1::Prm>::cast(this));
			cntx.proc(*static_cast<Typ2*>(this), PrmHelper<typename Typ2::Prm>::cast(this));
			cntx.proc(*static_cast<Typ3*>(this), PrmHelper<typename Typ3::Prm>::cast(this));
			cntx.proc(*static_cast<Typ4*>(this), PrmHelper<typename Typ4::Prm>::cast(this));
			cntx.proc(*static_cast<Typ5*>(this), PrmHelper<typename Typ5::Prm>::cast(this));
		}
	};
	Template1 struct Join {
		typedef dpndnC<Typ1, Typ2, Typ3, Typ4, Typ5, T> R;
	};
	template<> struct Join<Nothing> { typedef dpndnC R; };
	template<> struct Join<Typ1> { typedef dpndnC R; };
	template<> struct Join<Typ2> { typedef dpndnC R; };
	template<> struct Join<Typ3> { typedef dpndnC R; };
	template<> struct Join<Typ4> { typedef dpndnC R; };
	template<> struct Join<Typ5> { typedef dpndnC R; };
	Template1 struct JoinTo {
		typedef typename T::template Join<Typ1>::R _R1;
		typedef typename _R1::template Join<Typ2>::R _R2;
		typedef typename _R2::template Join<Typ3>::R _R3;
		typedef typename _R3::template Join<Typ3>::R _R4;
		typedef typename _R4::template Join<Typ4>::R R;
	};
};
template< typename Typ1, typename Typ2, typename Typ3, typename Typ4, typename Typ5, typename Typ6> struct dpndnC<Typ1, Typ2, Typ3, Typ4, Typ5, Typ6, Nothing> {
	int implementMe;
};

template< typename Typ1, typename Typ2> struct dpndnC_Join {
	typedef typename Typ2::template JoinTo< Typ1 > ::R R;
};

/*
#define Def_TemplateCmpnt_TopIF( cmpnt ) \
}; \
template<typename Base> class cmpnt##_TopIF; \
template<typename Base> struct TopIFFor<Base, cmpnt> : public cmpnt##_TopIF<Base>{};\
template<typename Base> class cmpnt##_TopIF : public cmpnt { \
typedef cmpnt CmpntT; 

#define Def_TemplateCmpnt_TopIF__Call_B( ret, name, prms, ... ) \
	ret name prms  override { return 	CmpntT::name( __VA_ARGS__ PrmHelper<typename CmpntT::Prm>::cast(  static_cast<Base*>(this) ) ); } 

#define Def_TemplateCmpnt_TopIF__Call_A( ret, name,  ... ) \
public: \
	virtual ret name( __VA_ARGS__ ) = 0;  \
protected:  \
	ret name ( __VA_ARGS__ Prm &p );
	*/

template< class Base, typename Typ1= Nothing, typename Typ2= Nothing, typename Typ3 = Nothing, typename Typ4 = Nothing, typename Typ5 = Nothing > struct TCmpnt;
template< class Base> struct TCmpnt<Base, Nothing, Nothing, Nothing, Nothing, Nothing> {
	typedef Nothing Prm;
	typedef typename dpndnC<Base> Req;

	Template1 T& get() { return *static_cast<T*>(this); }
	Template1 const T& get() const { return *static_cast<const T*>(this); }
	// RS_Proc ProcBase;
	//Template1 void proc_T(DisMain &dm, const T* _this) const { static_cast<const Base*>(this)->proc(dm, Prm(*static_cast<const Typ1*>(_this))); }
};
template< class Base, typename Typ1> struct TCmpnt<Base, Typ1, Nothing, Nothing, Nothing, Nothing> {
	typedef Typ1& Prm;
	typedef typename dpndnC<Base,Typ1> Req;

	template<typename In> static Typ1& castFrom( In *in ) { *static_cast<Typ1*>(in) }

	Template1 T& get() { return *static_cast<T*>(this); }
	Template1 const T& get() const { return *static_cast<const T*>(this); }
	// RS_Proc ProcBase;
	//Template1 void proc_T(DisMain &dm, const T* _this) const { static_cast<const Base*>(this)->proc(dm, Prm(*static_cast<const Typ1*>(_this))); }
};
template< class Base, typename Typ1, typename Typ2> struct TCmpnt<Base, Typ1, Typ2, Nothing, Nothing, Nothing> {
	typedef refTuple<Typ1, Typ2> Prm;
	typedef typename dpndnC<Base, Typ1, Typ2> Req;
	//typedef typename dpndnC<Typ1, Typ2>::Comp Req;
	//typedef RS_Proc ProcBase;
	//Template1 void proc_T(DisMain &dm, const T* _this) const { static_cast<const Base*>(this)->proc(dm, Prm(*static_cast<const Typ1*>(_this), *static_cast<const Typ2*>(_this))); }
	Template1 T& get() { return *static_cast<T*>(this); }
	Template1 const T& get() const { return *static_cast<const T*>(this); }
};
template< class Base, typename Typ1, typename Typ2, typename Typ3> struct TCmpnt<Base, Typ1, Typ2, Typ3, Nothing, Nothing> {
	typedef refTuple<Typ1, Typ2, Typ3> Prm;
	typedef typename dpndnC<Base, Typ1, Typ2, Typ3> Req;
	//typedef typename dpndnC<Typ1, Typ2>::Comp Req;
	//typedef RS_Proc ProcBase;
	//Template1 void proc_T(DisMain &dm, const T* _this) const { static_cast<const Base*>(this)->proc(dm, Prm(*static_cast<const Typ1*>(_this), *static_cast<const Typ2*>(_this))); }
	Template1 T& get() { return *static_cast<T*>(this); }
	Template1 const T& get() const { return *static_cast<const T*>(this); }
};
template< class Base, typename Typ1, typename Typ2, typename Typ3, typename Typ4 > struct TCmpnt<Base, Typ1, Typ2, Typ3, Typ4, Nothing> {
	typedef refTuple<Typ1, Typ2, Typ3, Typ4> Prm;
	typedef typename dpndnC<Base, Typ1, Typ2, Typ3, Typ4> Req;
	//typedef typename dpndnC<Typ1, Typ2>::Comp Req;
	//typedef RS_Proc ProcBase;
	//Template1 void proc_T(DisMain &dm, const T* _this) const { static_cast<const Base*>(this)->proc(dm, Prm(*static_cast<const Typ1*>(_this), *static_cast<const Typ2*>(_this))); }
	Template1 T& get() { return *static_cast<T*>(this); }
	Template1 const T& get() const { return *static_cast<const T*>(this); }
};
}

#endif //GEM_ORG_TEMPLATECMPNT_H