#pragma once
#include <BWAPI.h>

#include <BWTA.h>
#include <windows.h>

#include <cmath>

#include <vector>
#include <list>

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

using namespace BWAPI;
using namespace BWTA;
using namespace std;

class ExampleAIModule : public BWAPI::AIModule
{
public:
	//Methods inherited from BWAPI:AIModule

	ExampleAIModule() { };
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
		UnitCounter unitCap;
	};
	struct ActiveOrder
	{
		Unit u;
		TilePosition pos;
		BuildOrder bo;
	};

	UnitCounter mUnitCap;
	UnitCounter mUnitCount;
	int SCVGasCap;
	int mineralCost;
	int gasCost;
	int supplyCost;

	Unit researchSiege;
	Unit addonMachine;

	bool goHam;

	vector<BuildOrder> mBuildOrder;
	vector<ActiveOrder> mActiveOrder;

	Position mChokePoint;
	Position mGuardPoint;
	Position mHomePoint;
	Position mAttackPoint;

	void initAI();
	Unit findUnit(UnitType ut);
	void UpdateUnits();
	void tormentActiveOrders();
	void confirmActiveOrderDone(Unit unit);
	TilePosition findPlaceForBunker(Unit builder);
	void drawBunker();
	void updateFunction();
	void attackUnitAI(Unit u);
	void marineAI(Unit u);
	void siegeAITank(Unit u);
	void siegeAISiege(Unit u);
	void medicAI(Unit u);
};
