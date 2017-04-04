#ifndef GEM_SCENE3_MATERIAL_H
#define GEM_SCENE3_MATERIAL_H

#include "../Basic.h"

#include "Scene.h"
#include "../Org/ary.h"

namespace Gem { 

namespace Dis { class ShaderProg; class Texture; }
	
namespace Scene3 {


namespace Cmpnt {
	class Camera;
};

class Material {
public:
	/*Material(const Passive &o) : Tex(o.Tex), Mesh(o.Mesh), Prog(o.Prog) {
		//Hdwr = o.Hdwr;
		Count = 0;
	} */

	Material(Dis::Texture &t, Dis::ShaderProg &p) : Tex(t), Prog(p) {}

	~Material() {}

//private:	
	//Dis::Mesh *Hdwr;
	//Scene3::Texture &Tex;
	Dis::ShaderProg &Prog;
	Dis::Texture &Tex;

	bool Unlit; //hack
	/*
	struct RenderList {
		Cmpnt::Camera *Cam;
		ary<Cmpnt::Renderable> RL;
	};
	ary<RenderList> RList;

	RenderList& getEntry( Cmpnt::Camera *c ) { //todo - will be hotspot for optimisation
		for( int i = RList.count(); i--; ) {
			if( RList[i].Cam == c ) {
				return RList[i];
			}
		}
		auto &ret = RList.add();
		ret.Cam = c;
		return ret;
	} */

	//dList<Cmpnt::Renderable> RList;

	//int Count;
};


}	}

#endif //GEM_SCENE3_MATERIAL_H