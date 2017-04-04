#include "stdafx.h"

#include "Gem/Math/vec3.h"
vec3f& debugPosition();


#include <Gem/Main.h>
#include <Gem/MainWindow.h>
#include <Gem/ConCur/WorkHost.h>
#include <Gem/ConCur/ConCur.h>

#include <Gem/Dis/ShaderProg.h>
#include <Gem/Dis/Texture.h>

#include <Gem/Scene2/Camera.h>
#include <Gem/Scene2/Sprite.h>
#include <Gem/Scene2/Texture.h>
#include <Gem/Scene2/Scene.h>

#include <Gem/Scene3/Camera.h>
#include <Gem/Scene3/TestObj.h>
#include <Gem/Scene3/Passive.h>
//#include <Gem/Scene3/Texture.h>
#include <Gem/Scene3/Mesh.h>
#include <Gem/Scene3/Scene.h>
#include <Gem/Scene3/Planet.h>
#include <Gem/Scene3/Material.h>

#include <Gem/Resource/Resource.h>

#include <Gem/Dis/RenderState.h>
#include <Gem/JUI/InputGroup.h>
#include <Gem/OS/Time.h>

#include <Gem/Math/Randomizer.h>

#include "Audio.h"
#include <array> //counter

bool debugFlag(const u32 &i);

typedef Scene2::ScnNode<Scene2::Sprite> Label;
class Button  : public Scene2::ScnNode<Scene2::Sprite> {
public:
	Button( Scene2::Texture& tex, Scene2::Texture& tex2, const vec2f &at,const vec2f &size ) : T1(tex), T2(tex2)  {
		Tex = &tex;
		Pos = at;
		Size = size;
		Rotation = 0;
		Clicked = false;

	}

	void update(Scene2::Scene::UpdateCntx &cntx) override {

		rectf r;
		r.tl() = Pos - Size *0.5f;
		r.br() = Pos + Size *0.5f;

		bool mPress = JUI::key(JUI::Keycode::LMouse);

		if( r.pointCheck( (vec2f) JUI::mPos() ) ) {
			Tex = &T2;
			if (!LMouse && mPress) Pressed = true;
		} else {
			Tex = &T1;
			Pressed = false;
		}
		if (!mPress && Pressed) {
			Clicked = true;
			Pressed = false;
		} 
		LMouse = mPress;

	}
	Scene2::Texture &T1, &T2;

	bool Clicked, LMouse, Pressed;
};
class Counter : public Scene2::Scene::Node_Base {
public:
	
	const float Stride = 94;
	Counter(Scene2::Texture &tex, const vec2f &at, const f32 &scale, const u32 &val)
		: Tex({	Scene2::Texture( tex, rectf(0,164, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride,164, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride*2,164, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride*3,164, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride*4,164, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(0,0, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride,0, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride*2,0, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride*3,0, Stride, 164), vec2s(496,329)  ), 
			Scene2::Texture( tex, rectf(Stride*4,0, Stride, 164), vec2s(496,329)  )
	}), Value(val)
	{
		for( int i = 3; i--; ) {
			Spr[i].Size = vec2f(99, 164)*scale;
			Spr[i].Pos = at + vec2f(Spr[i].Size.x,0)*(f32)i;
			Spr[i].Tex = &Tex[0];
			Spr[i].Rotation = 0;
		}
	}
//private:
	~Counter() override {}

	void update(Scene2::Scene::UpdateCntx &cntx) override {

	}
	void addTo(Dis::DrawList & dl) override {

		int v = Value;
		for(int i = 3; i-- ; ) {
			
			Spr[i].Tex = &Tex[v % 10];
			Spr[i].addTo(dl);

			v /= 10;
		}

	}
	int Value;
	Scene2::Sprite Spr[3];
	std::array<Scene2::Texture, 10> Tex;
	//Scene2::Texture Tex[10];
};

/*
class Frame {
protected:	
	Frame( const CStr & str ) : BackgroundTex( str ) {
		BackgroundSpr.Size = vec2f(1024, 768);
		BackgroundSpr.Pos = BackgroundSpr.Size*0.5f;
		BackgroundSpr.Tex = &BackgroundTex;
		BackgroundSpr.Rotation = 0;
	}

	Scene2::Scene Scene;

	Scene2::Sprite  BackgroundSpr;	
	Scene2::Texture BackgroundTex;
};

/*
class MainMenu : public Frame{
public:

	MainMenu() 
		: Frame( CSTR("Media//menubackground.png") ),
		TitleTex( CSTR("Media//UI//title.png") ),
		HowToTex( CSTR("Media//UI//HowTo.png") ),
		PlayBttnTex1( CSTR("Media//UI//buttons.png"), rects(0,192, 256,64), vec2s(256,256) ),
		PlayBttnTex2( PlayBttnTex1, rects(0,128, 256,64), vec2s(256,256) ),	
		QuitBttnTex1( PlayBttnTex1, rects(0,64, 256,64), vec2s(256,256) ),	
		QuitBttnTex2( PlayBttnTex1, rects(0,0, 256,64), vec2s(256,256) ),
		PlayBttn(PlayBttnTex1,PlayBttnTex2, vec2f( 300,600), vec2f(256,64) ), 
		QuitBttn( QuitBttnTex1,QuitBttnTex2, vec2f( 724,600), vec2f(256,64))
	{

		Scene.add(&PlayBttn);
		Scene.add(&QuitBttn);

		
		Scene.add(&Title);

		Audio::load(CSTR("Media//Audio//music.wav"), true)->play();
	}

	void loop( volatile bool &shutdown, Dis::BufferedDrawList &bdl ) {

		OS::Time time = NoConstruct();
		sizet frameTime = 18;

		float deltaTime = 0.001f*(f32)frameTime;
		
		PlayBttn.Clicked = QuitBttn.Clicked = false;
		Title.Pos = vec2f(512, 300);
		Title.Size = vec2f(600,200);
		Title.Rotation = 0;
		Title.Tex = &TitleTex;

		for(;!shutdown;) {
			
			time.update();

			
			Scene.update(deltaTime);
			{
				auto dl = bdl.forUpdate();			
				BackgroundSpr.addTo(dl);
				Scene.addTo(dl);

				//Spr3.addTo(dl);
			}

			sizet ct = (sizet)time.updateElap().asI();
			if( ct < frameTime ) ConCur::sleep(frameTime-ct);
			//else uh oh

			if( PlayBttn.Clicked ){
				if (Title.Tex == &HowToTex) return;

				Title.Size = vec2f(0.0f,0.0f);
				Title.Tex = &HowToTex;
				Title.Size = vec2f(400,200);

				PlayBttn.Clicked = false;
			}
			if (QuitBttn.Clicked) ConCur::fullExit(-1);
		}

		Scene.clear();
	}

	Scene2::Texture PlayBttnTex1, PlayBttnTex2, QuitBttnTex1, QuitBttnTex2, TitleTex, HowToTex;
	Button PlayBttn, QuitBttn;
	Label Title;
};

*/


class Game *Gm;


class Game  {
public:
	Game() : 
		CntrTex( CSTR("Media//ui//counter.png") ),
		StdShdr(CSTR("Media//shaders//textureVS.glsl"), CSTR("Media//shaders//textureFS.glsl")),
		UnlitShdr(CSTR("Media//shaders//textureVS.glsl"), CSTR("Media//shaders//textureUnlitFS.glsl")),
	//	//	PlanetShdr( CSTR("Media//shaders//planetVS.glsl"), CSTR("Media//shaders//planetFS.glsl")  ),
		Tex1( CSTR("Media//armoredrecon_diff.png") ),
//		Tex2( CSTR("Media//Cobra Bomber//Object01cobrabomber.png") ),
//		Mesh2( CSTR("Media//Cobra Bomber//Cobra_Bomber.fbx") ),
	//	Tex2( CSTR("Media//Textures//sh3.png") ),
	//	Mesh2( CSTR("Media//Sample_Ship.fbx") ),
		Tex2(CSTR("Media//wasphunter_texture.png")),
		Mesh2(CSTR("Media//wasphunter.fbx")),
		//	Tex2( CSTR("Media//Morena smuggler ship//Morena_map-smuggler.png") ),
	//	Mesh2( CSTR("Media//Morena smuggler ship//Morena smuggler ship.fbx") ),
		NoiseTex( CSTR("Media//checker.png") ),
	////	Mesh1( CSTR("Media//armoredrecon.fbx") ),
		//CarMat( Tex1, StdShdr),
		TankMat(Tex2, StdShdr),
		//	PlanetMat( NoiseTex, PlanetShdr), 
		DefferedMat( NoiseTex, StdShdr),
		//Car(Mesh1, CarMat),
		Ship(Mesh2, TankMat),

		SkyboxMsh(CSTR("Media//Skyboxes.fbx")),
		SkyboxTex(CSTR("Media//Skybox_F.png")),
		SkyboxMat(SkyboxTex, UnlitShdr),
		Skybox(SkyboxMsh, SkyboxMat),

		RoidM( StdShdr, UnlitShdr ),
		Cntr(CntrTex, vec2f(900, 50), 0.4f, 0)
	{

		Mvmnt.Key[0] = 'A';
		Mvmnt.Key[1] = 'D';
		Mvmnt.Key[2] = 'S';
		Mvmnt.Key[3] = 'W';
		MoveObj = 0;

		JoystickTgl.Key = JUI::Keycode::Space;
		JoystickTgl.Key = JUI::Keycode::LMouse;
		JoystickTgl.activate();

		Gm = this;
		for( int i = 12; i--; ) {
			DebugToggles[i].Key = 0x70+i;
			DebugToggles[i].activate();
		}

		SkyboxMat.Unlit = true;
	}


	void loop( volatile bool &shutdown, Dis::BufferedDrawList &bdl ) {

		OS::Time time = NoConstruct();

		sizet frameTime = 18;
		  
		float deltaTime = 0.001f*(f32)frameTime;


		Cntr.Value = 0;

		Scene.clear();
	
		MoveObj = Scene.add(new Scene3::TestObj(DefferedMat, vec3f(0, 0, 0)));
		//auto car = MoveObj = Scene.add(new Scene3::PassiveObj(Car, vec3f(0, 10, 10)));
		//Scene.add(new Scene3::PassiveObj(Car, vec3f(10, 10, 20)));
		
		//Scene.add( new Scene3::PassiveObj( Ship, vec3f(17,0,-3), quatF::identity(), vec3f(1,1,1)*1.25f ) );
		auto ship = Scene.add( new Scene3::PassiveObj( Ship, vec3f(-0,0,-0), quatF::identity() , vec3f(1,1,1)*0.15f ) );
		float skyScl = 10;
		Scene.add(new Scene3::PassiveObj(Skybox, vec3f(0, 0, 0), quatF::identity(), vec3f(1, 1, 1)*-skyScl));
		Scene.add(new Scene3::PassiveObj(Skybox, vec3f(0, 0, 0), quatF::euler(vec3f(-180, 0, 0)* DEG_TO_RAD) , vec3f(1, 1, 1)*-skyScl));
		auto cam = Scene.add( new Scene3::CameraObj( vec3f(0,5,-30.0f) ) );
		
		Mvmnt.activate();
	//	MoveObj = cam;

		Dis::RenderState_2d rs2;
		Dis::RenderState_3d rs3;
				
		Dis::RS_ViewPort vp; vp.ScrnSz = vec2u(1024, 768);
		vp.apply();
		Dis::RS_Ortho ortho; ortho.ScrnSz = vec2u(1024, 768); ortho.Height = 768;
		Dis::RS_Projection proj; proj.ScrnSz = vec2u(1024, 768); proj.Far = 5000;

		//WorkHost::waitFor();

		quatF rotOffset =  quatF::euler(vec3f(-80, 180,0 )* DEG_TO_RAD ); 
		ship->Rot *= rotOffset;
		bool lMove = false; vec2s16 lMp;

		vec3f shipVel(0.0f);
		float fireTimer = 0;
		for(;!shutdown;) {
			
			time.update();
			
			Scene.update(deltaTime);
			//		printf("p  %f %f %f \n", car->Pos.x, car->Pos.y, car->Pos.z);
			//if( MoveObj ) {
			float speed = 15.0f;
			auto a = Mvmnt.value();

			ship->Rot *= rotOffset.getInverse();
			bool freeCam = debugFlag(8);

			float rotSpd = 1.2f;

			if( freeCam ) {
				MoveObj = cam;
				MoveObj->Pos += vec3f(a.x, 0, a.y) *12.0f *deltaTime  * MoveObj->Rot;
				shipVel = 0.0f;
			} else {
				MoveObj = ship;
			//	speed *= 0.2f;

				auto vec = vec3f(a.x, a.y, 0.3f);
				if (JUI::key(JUI::Keycode::RMouse)) {
					vec.z = 20;;
					rotSpd *= 0.3f;
					speed *= 3;
				}
				vec = vec.getNormal() * speed;

				shipVel -= (shipVel - vec * MoveObj->Rot.as<mat3f>()) *4.0f *deltaTime;
			}



//			float fwd = 0.3f;

			
			//printf("p  %f %f %f \n", shipVel.x, shipVel.y, shipVel.z);
			//todo mouse drag object
			bool drag = JoystickTgl.Value;
			if(drag) {
				if( lMove ) {
					vec2f mr = ((vec2f)(lMp - JUI::mPos()));
					float mx = 15;
					if (mr.sqrLeng() > mx*mx)
						mr = mr.getNormal()*mx;

					mr *= deltaTime*rotSpd;
					//MoveObj->Rot = (quatF::euler( vec3f( mr.y, mr.x, 0)  ) * MoveObj->Rot ).normalise();

					//MoveObj->Rot = (quatF::yRotation( mr.x ) * MoveObj->Rot );
					//MoveObj->Rot = (quatF::xRotation( mr.y ) * MoveObj->Rot );

					auto r = quatF::euler(vec3f(mr.y, mr.x, 0));
					MoveObj->Rot *= r;
					MoveObj->Rot.normalise();		
				}
				lMp = JUI::mPos();
			}
			lMove = drag;

			if ((fireTimer -= deltaTime) < 0 && JUI::key(JUI::Keycode::Space)) {
				RoidM.pew(Scene, shipVel, deltaTime, ship);
				fireTimer = 0.3f;
			}

			if( !freeCam ) {
				cam->Pos -= shipVel *deltaTime;
				cam->Rot = nLerp(cam->Rot, ship->Rot, deltaTime *8.0f);
				cam->Pos += (ship->Pos - vec3f(0, -3, 13)*cam->Rot.as<mat3f>() - cam->Pos) *  deltaTime *6.0f;
			
				
			} 
			if( !debugFlag(10))
				RoidM.proc(Scene, shipVel, deltaTime, Cntr.Value);

			ship->Rot *= rotOffset;
			//}
			{
				auto dl = bdl.forUpdate();			
				//BackgroundSpr.addTo(dl);
			
				proj.addTo(dl);
				rs3.addTo(dl);

				//Scene.addTo(dl);

				Scene.render( dl, cam);

				ortho.addTo(dl);
				rs2.addTo(dl);
				Cntr.addTo(dl);
				//ScoreSpr.addTo(dl);
			}
			//Cntr.Value++;

			sizet ct = (sizet)time.updateElap().asI();
			if( ct < frameTime ) ConCur::sleep(frameTime-ct);
		}

		//Scene.clear();
	}

	Res_From_T<Dis::ShaderProg, DStr, DStr > StdShdr, UnlitShdr;
	Res_From_T<Dis::Texture, DStr>  Tex1, Tex2, NoiseTex, SkyboxTex;


	Scene2::Texture CntrTex;
	
	Scene3::Scene Scene;
	//Scene3::Texture  Tex1, Tex2, NoiseTex;
	Scene3::Material TankMat, DefferedMat, SkyboxMat;// , PlanetMat;
	Scene3::Mesh Mesh2, SkyboxMsh;
	Scene3::Passive Ship, Skybox;


	class RoidMan {
	public:
		RoidMan(Dis::ShaderProg& ss, Dis::ShaderProg&  us ) :
			StdShdr(ss),
			RoidTex(CSTR("Media//Roids//Textures//am1.png")),
			RoidMsh(CSTR("Media//Roids//asteroid1.fbx")),
			RoidMat(RoidTex, StdShdr),
			Roid(RoidMsh, RoidMat),
			PlasmaTex(CSTR("Media//plasma.png")),
			PlasmaMsh(CSTR("Media//plasma.fbx")),
			PlasmaMat(PlasmaTex, us ),
			Plasma(PlasmaMsh, PlasmaMat) {

			Pew = Audio::load(CSTR("Media//Audio//pew.wav") );
			Boom = Audio::load(CSTR("Media//Audio//explosion.wav") );
			Timer = -20;
		}
		void proc( Scene3::Scene &scn, const vec3f &sv, const float &delta, int &score ) {

			if( (Timer -= delta)  < 0  ) {
				
				RoidList.add(SimpleObj( Rand, scn, Roid, vec3f( 0, 23.6f, -1.5f)) );
				
				Timer += 2;
			}

			for (auto& r : RoidList) {
				if( r.update(sv, delta) ) {
					
					r.randoPos(Rand);
				} else if( r.Mid.sqrLeng() < 100.0f)
				{
					r.randoPos(Rand);
					score = max(0, score - 10);
					Boom->play();
				}
			}

			for (int i = PlasmaList.count(); i--; ) {
				auto& r = PlasmaList[i];
				if (r.Active) {
					if (r.update(sv, delta)) {
						Unactive.add(i);
						//printf("p  %f %f %f \n", shipVel.x, shipVel.y, shipVel.z);
						printf("plasma recycle  %d   -- p  %f %f %f \n", i, r.Obj->Pos.x, r.Obj->Pos.y, r.Obj->Pos.z);
						r.Obj->Pos *= 10000.0f;
						r.Active = false;
						//	Active.removeAt(i);
					} else {
						
						for (auto r2 : RoidList) {
							if( (r2.Mid - r.Mid).sqrLeng() < 25 ) {
								Unactive.add(i);
								r.Obj->Pos *= 10000.0f;
								r.Active = false;

								score++;
								r2.randoPos(Rand);
								break;
							}
						}
					}
				}
			}
		}

		void pew(Scene3::Scene &scn, const vec3f &sv, const float &delta, Scene3::PassiveObj *ship ) {

			auto& obj = plasma( scn,  sv, delta, ship );

			auto rot = ship->Rot.as<mat3f>();
			vec3f p = ship->Pos + vec3f(0, -0.5f, 2.0f)*rot;
			obj.Vel = sv*0.5f + vec3f(0, 0, 85 ) *rot;
			obj.Obj->Pos = p;
			obj.Active = true;
			//Active.add((PlasmaList.count()));
			Pew->play();
		}
	private:

		struct  SimpleObj {
			SimpleObj() { Obj = 0;  }
			SimpleObj(Randomizer &Rand, Scene3::Scene &scn, Scene3::Passive &Roid, const vec3f &com, float scl = 0.1f) : CoM(com) {


				CoM *= scl;
				Obj = scn.add(new Scene3::PassiveObj(Roid, vec3f(0,0,0), quatF::randf(Rand), vec3f(1, 1, 1)*scl));

				randoPos(Rand);
				Active = true;
			}
			SimpleObj(Randomizer &Rand, Scene3::Scene &scn, Scene3::Passive &Roid, const vec3f &com, const vec3f &sv, Scene3::PassiveObj *ship) : CoM(com) {
				float scl = 0.02f;
				AVel = quatF::randf(Rand);
				CoM *= scl;


				Obj = scn.add(new Scene3::LightDooDad(Roid, vec3f(0,0,0), quatF::randf(Rand), vec3f(1, 1, 1)*scl));
				Active = true;
			}
			bool update(const vec3f &sv, float delta) {
				Obj->Pos += (Vel-sv) *delta;
				float mx = 260;
				if( Obj->Pos.sqrLeng() > mx*mx )
				{
					return true; 
				}
				vec3f off = CoM *Obj->Rot;
				Obj->Rot *= sLerp(quatF::identity(), AVel, delta);
				Obj->Rot.normalise();
				Mid = Obj->Pos + off;
				off -= CoM *Obj->Rot;
				
				Obj->Pos += off;
				return  false;
			}
			void randoPos ( Randomizer &Rand) {
				vec3f p;
				for (int iter = 99; iter--; ) {
					p = vec3f::inCube(Rand).getNormal()  *Rand.range(150.0f, 250.0f);
					if( p.sqrLeng() > 200.0f )
						break;
				}
				Vel = (vec3f::inCube(Rand) *Rand.range(30.0f, 60.0f) -p).getNormal()*Rand.range(8.0f, 32.0f);
				AVel = quatF::randf(Rand);
				Mid = Obj->Pos = p;
			}
			vec3f Vel, CoM, Mid;
			quatF AVel;
			Scene3::Cmpnt::Offset* Obj;
			bool Active;
		};
		SimpleObj& plasma(Scene3::Scene &scn, const vec3f &sv, const float &delta, Scene3::PassiveObj *ship) {
			if (Unactive.count() >0) {
				int retI = Unactive[Unactive.count() - 1];
				auto& ret = PlasmaList[retI ];
				Unactive.decCount();
				printf("plasma recycled  %d \n ", retI);
				return ret;
			} else {
				return PlasmaList.add(SimpleObj(Rand, scn, Plasma, vec3f(0, 0, 0), sv, ship));
			}
		}
		Dis::ShaderProg& StdShdr;
		Res_From_T<Dis::Texture, DStr> RoidTex, PlasmaTex;

		Scene3::Material  RoidMat, PlasmaMat;// , PlanetMat;
		Scene3::Mesh RoidMsh, PlasmaMsh;
		Scene3::Passive Roid, Plasma;

		float Timer;
		Randomizer Rand;
		ary< SimpleObj > RoidList, PlasmaList;
		ary<int, Ctor::Simple> Unactive;

		Audio * Pew, *Boom;
	};

	RoidMan RoidM;
	//Res_FromFile<Scene3::Texture> Tex1, Tex2, NoiseTex;

	Counter Cntr;
	
	JUI::AxisDual Mvmnt;
	Scene3::Cmpnt::Offset *MoveObj;

	JUI::Toggle JoystickTgl;
	JUI::Toggle DebugToggles[12];

};
bool debugFlag( const u32 &i ) {
	return Gm->DebugToggles[i].Value;
}

class Main { 
friend class Gem::Main_Hlpr;
	
	Main()  { 
		DEFClassFunc( void, mainLoopWrp, Main, mainLoop );
		MainThread.start( mainLoopWrp, this );			
	//	Cam.setOutput(Wndw);		
	}

	~Main() {
		MainThread.waitFor();
	}

	void mainLoop() {
		Shutdown = false;
		Audio::initOAL();
		Audio::load(CSTR("Media//Audio//music.wav"), true)->play();
		Game game;
		for(;!Shutdown;) {		
			game.loop( Shutdown, Wndw.DrawL );						
		}		
	}

	void shutdown() {
		Shutdown = true;
	}

	AutoCtor( MainWindow, Wndw,	( "Alice", vec2u16(1024,768), OS::Window::Flg_Resizeable ) );


	Thread MainThread;
	volatile bool Shutdown;
};

DEF_MainClass( Main );
