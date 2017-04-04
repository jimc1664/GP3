#ifndef GEM_SCENE3_SCENE_H
#define GEM_SCENE3_SCENE_H

#include "../Basic.h"
#include "../Org/dList.h"
#include "../Org/ary.h"
#include "../Org/TemplateCmpnt.h"

#include "../Math/Vec3.h"
#include "../Math/matrix3x4.h"
#include "../Math/Quaternion.h"

#include <type_traits>

namespace Gem { 


namespace Dis { class DrawList; }
	
namespace Scene3 {

namespace Cmpnt { class Updateable; class Renderable; class Parent; class Camera; class Light;}

struct BuildDrawLCntx;

class Node_Base : public dListNode<Node_Base> {
public:
	Node_Base() : Prnt(0) {}
	virtual void addObjTo( BuildDrawLCntx & dl) = 0;
	virtual ~Node_Base(){
		int a = 0;
	}
	Cmpnt::Parent *Prnt;
};
class NodeGroup : public dList<Node_Base> {

};

class Scene; class Material;

struct BuildDrawLCntx {
	BuildDrawLCntx( Scene &scn, Dis::DrawList & dl ) : Scn(scn ), Dl(dl), CurCam(0) {}

	Template2 void proc( T &a, T2 &p ) { a.onBuildDl( *this, p ); }

	//void add( Cmpnt::Renderable *

	Dis::DrawList &Dl;
	Scene &Scn;

	//use of arrays here is temporary...   need threading, sorting, cacheing of references (todo)

	struct MatEntry {
		MatEntry( Material &m ) : Mat(m) {}
		Material &Mat;
		ary<Cmpnt::Renderable*, Ctor::Simple > RL;
	};


	struct CamEntry {
		CamEntry( Cmpnt::Camera &c ) : Cam(c) {}
		
		MatEntry& get( Material &mat ) { //todo - likely optimisation hotspot - use cache handles / frame state 
			for( int i = ML.count(); i--; ) {
				if( &ML[i].Mat == &mat ) {
					return ML[i];
				}
			}
			return ML.add( MatEntry(mat) );
		}
		void add( Material &mat, Cmpnt::Renderable &r ) {
			get(mat).RL.add(&r);			
		}
		Cmpnt::Camera &Cam;
		ary<MatEntry> ML;
		ary<Cmpnt::Light*, Ctor::Simple> LL;
	};
	ary<CamEntry> CL;

	void add( Cmpnt::Camera &cam ) {
		CurCam = &CL.add( CamEntry(cam) );		
	}

	CamEntry *CurCam;

	void build();
private:

};


class Scene {
public:
	Scene(){}
	~Scene() { clear(); }
	//Sprite( const char * caption, const vec2u16 &size, const u32 &flags);


	template<class Typ>  Typ* add(Typ *nd) { 
		addNode( static_cast<Node_Base*>(nd) ); 
		nd->add(AddCntx(*this));
		return nd;
	}
	void addNode(Node_Base *nd);
	void clear();

	template<class Cam> void render( Dis::DrawList & dl, Cam *cam) {
		//children

		BuildDrawLCntx cntx(*this, dl);
		cam->setCam( cntx, cam->prmFor<Cmpnt::Camera>() );

		auto *nd = static_cast<Node_Base*>(cam);
		if( cam->Prnt == null ) {
			for( auto it = Root.start(); it != Root.end(); it++ ) {
				Node_Base * cur = it;
				if(cur == nd) continue;
				cur->addObjTo(cntx);
			}				
		} else Error("unhandled");


		cntx.build();
	}


	void update( const f32 &delta );


	dList<Cmpnt::Updateable>  Updateables;
//private:	  //todo...

	NodeGroup Root;

	//simplifictaion of a system yet to add -  to defer deletion of 'stuff'for a few frames 
	//- so temporary references are dropped - so they don't have to be kept track of  -- one frame should really be enough 
	dList<Node_Base> ToBeDeleted[3];
	int TBDi;
};

struct UpdateCntx {
	UpdateCntx( Scene &scn, const float &d);
	Template2 void proc( T &a, T2 &p ) { a.onUpdate( *this, p ); }
	const float Delta;

	void destroy(Node_Base *nd);
	void detach(Node_Base *nd);

	Scene &Scn;
private:
	dList<Cmpnt::Updateable>::iter It;
};
struct AddCntx {
	AddCntx( Scene &s ) : Scn(s) {}
	Template2 void proc( T &a, T2 &p ) { a.onAdd( *this, p ); }
	Scene &Scn;
};
struct AddToDl_LightCntx {
	AddToDl_LightCntx(Dis::DrawList &dl, const mat3x4f &v ) : Dl(dl), View(v) {}
	Template2 void proc(T &a, T2 &p) { a.addToDl_Light( *this, p); }
	Dis::DrawList &Dl;
	const mat3x4f &View;
};
namespace Cmpnt {
class ScnBaseComponent {
protected:
	template<class T> struct TopIf {};	
public:
	//void update( Scene::UpdateCntx &cntx  ) {}
	void onAdd( AddCntx &ac, Nothing  ) {}
	void onUpdate( UpdateCntx &ac, Nothing  ) {}
	void onBuildDl( BuildDrawLCntx &cntx, Nothing ) {} 
	void addToDl( Dis::DrawList& dl, Nothing ) {} 

	void addToDl_Light(AddToDl_LightCntx& dl, Nothing) {}
};
class Parent : public ScnBaseComponent, public NodeGroup, public dListNode<Updateable>, public TCmpnt<Updateable> {
public:
};
class Updateable : public ScnBaseComponent, public dListNode<Updateable>, public TCmpnt<Updateable> {
public:
	void onAdd( AddCntx &ac, Prm &p ) {
		ac.Scn.Updateables.add(this);
	}
	virtual void updateObj( UpdateCntx &cntx) =0;
	//virtual ~Updateable() { int a = 0; }
};
class Renderable : public ScnBaseComponent, public dListNode<Renderable>, public TCmpnt<Renderable> {
public:
	virtual void addObjToDl( Dis::DrawList& dl) =0;
	//virtual ~Renderable() { int a = 0; }
};

class Offset : public ScnBaseComponent, public TCmpnt<Offset> {
public:
	vec3f Pos;
	quatF Rot;
};


	class Light : public ScnBaseComponent, public TCmpnt<Light, Offset > {
	public:

		Light() : Rad(5) {}
		//void onAdd(AddCntx &ac, Prm &p) {}

		void onBuildDl(BuildDrawLCntx &cntx, Prm &p);
		void addToDl_Light( AddToDl_LightCntx &dl, Prm &p);
		//void onUpdate(UpdateCntx &cntx, Prm &p);
		virtual void addObjToDl_Light(AddToDl_LightCntx &cntx) = 0;
		float Rad;
	private:
	};
}


/*
template<class Typ> class ScnNode : public Scene::Node_Base, public Typ {
public:
	ScnNode() {	
		static_assert( std::is_convertible<Typ,ScnComponent >::value, "Type mismatch" );
	}
	Template1 ScnNode( const T &a) : Typ(a) { static_assert( std::is_convertible<Typ,ScnComponent >::value, "Type mismatch" ); }
	Template2 ScnNode( const T &a, const T2 &b ) : Typ(a, b ) { static_assert( std::is_convertible<Typ,ScnComponent >::value, "Type mismatch" ); }
	~ScnNode() override {}

	void update( Scene::UpdateCntx &cntx ) override { Typ::update(cntx); }
	void addTo( Dis::DrawList & dl) override {Typ::addTo(dl); }
}; */
template<  typename Comp > struct S3_T_Base : public Node_Base, public Comp {
	void add( AddCntx &cntx ) {  procForAll(cntx); }
	virtual void updateObj( UpdateCntx &cntx) { procForAll(cntx); }
	virtual void addObjToDl( Dis::DrawList& dl) { procForAll(dl); }
	virtual void addObjTo( BuildDrawLCntx &cntx ) { procForAll(cntx); } 
	virtual void addObjToDl_Light(AddToDl_LightCntx &cntx ) { procForAll( cntx ); }
	virtual ~S3_T_Base() {}

	template<typename SubCmpnt> typename SubCmpnt::Prm prmFor() { return PrmHelper<typename SubCmpnt::Prm>::cast(this); }
};
template< typename Typ1, typename Typ2 = Nothing, typename Typ3 = Nothing, typename Typ4 = Nothing, typename Typ5 = Nothing > class S3_T;
template< typename Typ1> class S3_T<Typ1, Nothing, Nothing, Nothing, Nothing> : public S3_T_Base<  typename Typ1::Req::Comp > { };
template< typename Typ1, typename Typ2> class S3_T<Typ1, Typ2, Nothing, Nothing, Nothing> : public S3_T_Base<  typename dpndnC_Join< typename Typ1::Req, typename Typ2::Req>::R::Comp > {};



}	}

#endif //GEM_SCENE3_SCENE_H