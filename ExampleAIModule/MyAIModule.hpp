#pragma once
#include <BWAPI.h>

#include <BWTA.h>
#include <windows.h>

#include <cmath>

#include <vector>

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

using namespace BWAPI;
using namespace BWTA;
using namespace std;

class MyAIModule : public BWAPI::AIModule
{
public:
	//Methods inherited from BWAPI:AIModule

	MyAIModule() { };
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);

	//Own methods
	void drawStats();
	void drawTerrainData();
	void showPlayers();
	void showForces();
	Position findGuardPoint();

private:
	enum BuildType
	{
		Build,
		Addon
	};

	struct UnitCounter
	{
		int SCV;
		int marine;
		int siege;
		int medic;
	};

	struct BuildOrder
	{
		UnitType ut;
		BuildType bt;
		//int foodReq;
		//int SCVCap;
		//int marineCap;
		UnitCounter unitCap;
	};
	struct ActiveOrder
	{
		Unit u;
		TilePosition pos;
		BuildOrder bo;
	};
};
