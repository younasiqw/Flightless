#include "../SDK.h"
 
IBaseClientDll*			I::Client;
IClientModeShared*		I::ClientMode;
CInput*					I::Input;
IClientEntityList*		I::ClientEntList;
ICVar*					I::Cvar;
IMDLCache*				I::MDLCache;
IInputSystem*			I::InputSystem;
ILocalize*				I::Localize;
IEngineClient*			I::Engine;
IEngineTrace*			I::EngineTrace;
IGlobalVarsBase*		I::Globals;
ISurface*				I::Surface;
//IBaseFileSystem*		I::FileSystem;
IVPanel*				I::VPanel;
IVModelRender*			I::ModelRender;
IVModelInfo*			I::ModelInfo;
IMaterialSystem*		I::MaterialSystem;
IMaterial*				I::Material;
IVRenderView*			I::RenderView;
IGameMovement*			I::GameMovement;
IMoveHelper*			I::MoveHelper;
IPrediction*			I::Prediction;
IPhysicsSurfaceProps*	I::Physprops;
IGameEventManager2*		I::GameEvent;
IVDebugOverlay*			I::DebugOverlay;
IStudioRender*			I::StudioRender;

IMaterial* visible_tex;
IMaterial* hidden_tex;
IMaterial* visible_flat;
IMaterial* hidden_flat;