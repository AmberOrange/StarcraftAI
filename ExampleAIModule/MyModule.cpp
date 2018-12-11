#include "Source/ExampleAIModule.h"
#include <cmath>
#define bw Broodwar
//This is the method called each frame. This is where the bot's logic
//shall be called.

void ExampleAIModule::initAI()
{
	bw->setLocalSpeed(0);
	this->SCVGasCap = 2;

	this->mUnitCap =	{ 7,0,0,0 };
	this->mUnitCount =	{ 0,0,0,0 };

	this->mineralCost = 0;
	this->gasCost = 0;
	this->supplyCost = 0;

	this->researchSiege = nullptr;
	this->addonMachine = nullptr;

	this->goHam = false;

	this->mChokePoint = this->findGuardPoint();
	this->mHomePoint = Position(bw->self()->getStartLocation().x*32, bw->self()->getStartLocation().y*32);
	for (TilePosition tp : bw->getStartLocations())
	{
		Position pos(tp.x * 32, tp.y * 32);
		if (pos != this->mHomePoint)
			this->mAttackPoint = pos;
	}


	// This is the build order that has to be completed before we attack
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Factory,		BuildType::Build,	{ 12,20,5,5 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Bunker,			BuildType::Build,	{ 0,20,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Academy,		BuildType::Build,	{ 0,0,0,3 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Bunker,			BuildType::Build,	{ 0,16,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Refinery,		BuildType::Build,	{ 0,0,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Bunker,			BuildType::Build,	{ 0,12,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Bunker,			BuildType::Build,	{ 0,8,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Barracks,		BuildType::Build,	{ 12,4,0,0 } });

	// Move a SCV to choke to know where to build later on
	this->findUnit(UnitTypes::Terran_SCV)->move(this->mChokePoint);
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
	if (unit->getPlayer() == bw->self())
	{
		if (unit->getType() == UnitTypes::Terran_SCV)
			this->mUnitCount.SCV++;
		else if (unit->getType() == UnitTypes::Terran_Marine)
			this->mUnitCount.marine++;
		else if (unit->getType() == UnitTypes::Terran_Medic)
			this->mUnitCount.medic++;
		else if (unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
			this->mUnitCount.siege++;
		else
			this->confirmActiveOrderDone(unit);
	}
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == bw->self())
	{
		if (unit->getType() == UnitTypes::Terran_SCV)
			this->mUnitCount.SCV--;
		else if (unit->getType() == UnitTypes::Terran_Marine)
		{
			this->mUnitCount.marine--;
		}
		else if (unit->getType() == UnitTypes::Terran_Medic)
			this->mUnitCount.medic--;
		else if (unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
			this->mUnitCount.siege--;
		else if (unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
			this->mUnitCount.siege--;
		else if (unit->getType().isBuilding())
		{
			this->mBuildOrder.push_back(
				{ unit->getType(),		BuildType::Build,	{ 0,0,0,0 } });
		}
	}
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
	if (unit->getPlayer() == bw->self())
		this->confirmActiveOrderDone(unit);
}

//Called when a unit has been completed, i.e. finished built.
void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Terran_Barracks
		|| unit->getType() == UnitTypes::Terran_Factory)
	{
		unit->setRallyPoint(this->mChokePoint);
		if (unit->getType() == UnitTypes::Terran_Factory)
			this->addonMachine = unit;
	}
	else if (unit->getType() == UnitTypes::Terran_Supply_Depot)
		this->supplyCost -= 8;
	else if (unit->getType() == UnitTypes::Terran_Machine_Shop)
		this->researchSiege = unit;
}


Unit ExampleAIModule::findUnit(UnitType ut)
{
	for (Unit u : bw->self()->getUnits())
		if (u->getType() == ut)
			if (ut == UnitTypes::Terran_SCV)
			{
				if (u->canBuild() && !u->isGatheringGas())
				{
					bool alreadyOccupied = false;
					for (ActiveOrder ao : this->mActiveOrder)
					{
						if (ao.u->getID() == u->getID())
						{
							alreadyOccupied = true;
							break;
						}
					}

					if(!alreadyOccupied)
						return u;
				}
			}
			else
				return u;
	return nullptr;
}

void ExampleAIModule::UpdateUnits()
{
	for (Unit u : bw->self()->getUnits())
		if (u->getType() == UnitTypes::Terran_SCV)
		{
			if (u->isIdle())
			{
				Unit closestMineral = NULL;
				Unit commandCenter = this->findUnit(UnitTypes::Terran_Command_Center);
				for (auto m : Broodwar->getMinerals())
				{
					if (closestMineral == NULL || commandCenter->getDistance(m) < u->getDistance(closestMineral))
					{
						closestMineral = m;
					}
				}
				if (closestMineral != NULL)
				{
					u->rightClick(closestMineral);
				}
			}
		}
		else if (u->getType() == UnitTypes::Terran_Refinery)
		{
			int nrRefWorkers = 0;
			for (Unit u2 : bw->self()->getUnits())
			{
				if (u2->isGatheringGas())
					nrRefWorkers++;
			}
			if (nrRefWorkers < this->SCVGasCap)
			{
				for (Unit u2 : bw->self()->getUnits())
				{
					if (u2->getType() == UnitTypes::Terran_SCV)
					{
						if (u2->canGather())
						{
							u2->gather(u);
							nrRefWorkers++;
							if (nrRefWorkers == this->SCVGasCap)
								break;
						}
					}
				}
			}
		}
		else
		{
			attackUnitAI(u);
		}
}

void ExampleAIModule::tormentActiveOrders()
{
	for (ActiveOrder& ao : this->mActiveOrder)
	{
		if (ao.bo.bt == BuildType::Build)
		{
			if (!ao.u->build(ao.bo.ut, ao.pos))
			{
				if (ao.bo.ut == UnitTypes::Terran_Bunker)
				{
					
					bw->drawCircle(CoordinateType::Map, ao.pos.x * 32, ao.pos.y * 32, 16, Color(255, 0, 0), true);
					bw->drawCircle(CoordinateType::Map, ao.u->getPosition().x, ao.u->getPosition().y, 16, Color(255, 255, 0), true);
					if (!bw->canBuildHere(ao.pos, ao.bo.ut))
					{
						TilePosition newPos = findPlaceForBunker(ao.u);
						ao.pos = newPos;
					}
				}
			}
		}
	}
}

// This function is being called when we're done with an active order
// and want to update the cap.
void ExampleAIModule::confirmActiveOrderDone(Unit unit)
{
	for (int i = 0; i < this->mActiveOrder.size(); i++)
	{
		if (unit->getType() == this->mActiveOrder[i].bo.ut)
		{
			this->mUnitCap.SCV = std::max(this->mActiveOrder[i].bo.unitCap.SCV, this->mUnitCap.SCV);
			this->mUnitCap.marine = std::max(this->mActiveOrder[i].bo.unitCap.marine, this->mUnitCap.marine);
			this->mUnitCap.siege = std::max(this->mActiveOrder[i].bo.unitCap.siege, this->mUnitCap.siege);
			this->mUnitCap.medic = std::max(this->mActiveOrder[i].bo.unitCap.medic, this->mUnitCap.medic);
			this->mActiveOrder.erase(this->mActiveOrder.begin() + i);
			this->mineralCost -= unit->getType().mineralPrice();
			this->gasCost -= unit->getType().gasPrice();
			break;
		}
	}
}

TilePosition ExampleAIModule::findPlaceForBunker(Unit builder)
{
	bool foundIt = false;
	TilePosition tPos;

	int tileRadius = 3;
	Position line = this->mGuardPoint - this->mChokePoint;
	TilePosition tChoke(this->mChokePoint.x / 32, this->mChokePoint.y / 32);
	float direction = acosf((float)line.x / (float)line.getLength()) + 3.14f;
	while (!foundIt && tileRadius < 20)
	{
		for (float i = -1.57f; i < 1.57; i += 0.39f * 1.5f)
		{
			tPos = TilePosition(
				tChoke.x + (int)(cosf(direction+i)*tileRadius),
				tChoke.y + (int)(sinf(direction+i)*tileRadius));
			if (bw->canBuildHere(tPos, UnitTypes::Terran_Bunker, builder,true))
			{
				foundIt = true;
				break;
			}
		}
		tileRadius++;
	}
	return tPos;
}

void ExampleAIModule::drawBunker()
{
	TilePosition tPos;

	int tileRadius = 3;
	Position line = this->mGuardPoint - this->mChokePoint;
	TilePosition tChoke(this->mChokePoint.x / 32, this->mChokePoint.y / 32);
	float direction = acosf((float)line.x / (float)line.getLength()) + 3.14f;
	while ( tileRadius < 20)
	{
		for (float i = -1.57f; i < 1.57f; i += 0.39f * 1.5f)
		{
			tPos = TilePosition(
				tChoke.x + (int)(cosf(direction+i)*tileRadius),
				tChoke.y + (int)(sinf(direction+i)*tileRadius));
			bw->drawCircleMap(Position(tPos.x * 32, tPos.y * 32), 10, Color(0, 255, 100));
		}
		tileRadius++;
	}
}

void ExampleAIModule::updateFunction()
{
	Unit target;
	if (bw->getFrameCount() % 20 == 0)
	{
		this->UpdateUnits();
		this->tormentActiveOrders();
	}

	// If we're running out of supplies for coming units, make more supply depots
	if ((mUnitCap.marine + mUnitCap.medic + mUnitCap.SCV + mUnitCap.siege + 3) > (bw->self()->supplyTotal() / 2) + this->supplyCost)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_Supply_Depot.mineralPrice())
		{
			Unit bu = findUnit(UnitTypes::Terran_SCV);
			TilePosition pos = bw->getBuildLocation(UnitTypes::Terran_Supply_Depot,
				TilePosition(this->mHomePoint.x / 32, this->mHomePoint.y / 32));
			ActiveOrder ao =
			{
				bu,
				pos,
				{
					UnitTypes::Terran_Supply_Depot,
					BuildType::Build,
					{0,0,0,0}
				}
			};

			this->mineralCost += UnitTypes::Terran_Supply_Depot.mineralPrice();
			this->supplyCost += 8;
			this->mActiveOrder.push_back(ao);
		}
	}

	// If we can make an SCV, and we're not capped, do it.
	if (this->mUnitCount.SCV < this->mUnitCap.SCV)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_SCV.mineralPrice())
			if ((target = findUnit(UnitTypes::Terran_Command_Center)) != nullptr)
				if (!target->isTraining())
					target->train(UnitTypes::Terran_SCV);
	}

	// Same for siege tanks.
	if (this->mUnitCount.siege < this->mUnitCap.siege)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_Siege_Tank_Tank_Mode.mineralPrice())
			if ((bw->self()->gas() - this->gasCost) >= UnitTypes::Terran_Siege_Tank_Tank_Mode.gasPrice())
				if ((target = findUnit(UnitTypes::Terran_Factory)) != nullptr)
					if (!target->isTraining() && target->canTrain())
					{
						target->train(UnitTypes::Terran_Siege_Tank_Tank_Mode);
					}
	}

	// etc. etc.
	if (this->mUnitCount.medic < this->mUnitCap.medic)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_Medic.mineralPrice())
			if ((bw->self()->gas() - this->gasCost) >= UnitTypes::Terran_Medic.gasPrice())
				if ((target = findUnit(UnitTypes::Terran_Barracks)) != nullptr)
					if (!target->isTraining())
						target->train(UnitTypes::Terran_Medic);
	}

	if (this->mUnitCount.marine < this->mUnitCap.marine)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_Marine.mineralPrice())
			if ((target = findUnit(UnitTypes::Terran_Barracks)) != nullptr)
				if (!target->isTraining())
					target->train(UnitTypes::Terran_Marine);
	}

	// If it's time to research siege
	if (this->researchSiege != nullptr)
	{
		if (this->researchSiege->research(TechTypes::Tank_Siege_Mode))
		{
			this->researchSiege = nullptr;
			bw->printf("Researching Siege mode, Oh yeah!");
		}
	}

	// If it's time to build the machine shop
	if (this->addonMachine != nullptr)
	{
		if (this->addonMachine->buildAddon(UnitTypes::Terran_Machine_Shop))
		{
			this->addonMachine = nullptr;
			bw->printf("Adding Machine Shop, Oh yeah!");
		}
	}

	// Check if we got any build orders to complete
	if (this->mBuildOrder.size() > 0)
	{
		//bw->printf("There is something to build!");
		if ((bw->self()->minerals() - this->mineralCost) >= this->mBuildOrder.back().ut.mineralPrice() &&
			(bw->self()->gas() - this->gasCost) >= this->mBuildOrder.back().ut.gasPrice())
		{
			//bw->printf("I have enough for this!");
			if (this->mBuildOrder.back().bt == BuildType::Build)
			{
				//bw->printf("It's a building!");
				Unit bu = findUnit(UnitTypes::Terran_SCV);
				if (bu != nullptr)
				{
					//bw->printf("I found a SCV!");
					TilePosition pos;
					if (this->mBuildOrder.back().ut == UnitTypes::Terran_Bunker)
					{
						pos = this->findPlaceForBunker(bu);
					}
					else {
						TilePosition desiredPos(this->mHomePoint.x / 32, this->mHomePoint.y / 32);
						int radius = 1000;
						pos = bw->getBuildLocation(this->mBuildOrder.back().ut,
							desiredPos,
							radius);
					}

					ActiveOrder ao =
					{
						bu,
						pos,
						this->mBuildOrder.back()
					};

					this->mineralCost += this->mBuildOrder.back().ut.mineralPrice();
					this->gasCost += this->mBuildOrder.back().ut.gasPrice();
					this->mActiveOrder.push_back(ao);
					this->mBuildOrder.pop_back();
				}
			}
		}
	}
	else {
		if (!this->goHam &&
			this->mUnitCount.marine == this->mUnitCap.marine &&
			this->mUnitCount.siege == this->mUnitCap.siege &&
			this->mUnitCount.medic == this->mUnitCap.medic)
		{
			// All criterias for the attack is met, let's go!
			this->goHam = true;
			bw->printf("Time to attack!");
			bw->setLocalSpeed(10);
			for (Unit u : bw->self()->getUnits())
			{
				if (u->getType() == UnitTypes::Terran_Bunker)
					u->unloadAll();
			}
			this->mUnitCap = { 0,50,10,0 };
		}
	}
}

void ExampleAIModule::attackUnitAI(Unit u)
{
	if (u->getType() == UnitTypes::Terran_Marine)
		this->marineAI(u);
	else if (u->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
		siegeAITank(u);
	else if (u->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
		siegeAISiege(u);
	else if (u->getType() == UnitTypes::Terran_Medic)
		this->medicAI(u);
}

void ExampleAIModule::marineAI(Unit u)
{
	Unit target = bw->getClosestUnit(u->getPosition(), Filter::IsEnemy);
	if (!u->isHoldingPosition() && target != nullptr && u->isInWeaponRange(target))
	{
		u->holdPosition();
	} else
	if (!this->goHam)
	{
		if (!u->isLoaded())
		{
			bool goingToBunker = false;
			for (Unit u2 : bw->self()->getUnits())
			{
				if (u2->getType() == UnitTypes::Terran_Bunker)
				{
					if (u2->canLoad(u))
					{
						u2->load(u);
						goingToBunker = true;
						break;
					}
				}
			}
			if (!goingToBunker)
			{
				if (u->getPosition().getDistance(this->mChokePoint) > 30)
				{
					if (!u->isMoving())
						u->move(this->mChokePoint);
				}
				else {
					if(u->isIdle())
						u->holdPosition();
				}
			}
		}
	}
	else {
		if (u->getDistance(this->mAttackPoint) > 1500)
		{
			if ((!u->isAttacking() || u->isIdle()) && !u->isMoving())
				u->move(this->mAttackPoint);
		}
		else {
			if (this->mUnitCount.SCV > 0 && (!u->isAttacking() || u->isIdle()) && !u->isFollowing())
			{
				int distance = MAXINT;
				int tempDist = 0;
				Unit siegeTank = nullptr;
				for (Unit u2 : bw->self()->getUnits())
				{
					if (u2->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode ||
						u2->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
					{
						tempDist = u2->getDistance(this->mAttackPoint);
						if (tempDist < distance)
						{
							distance = tempDist;
							siegeTank = u2;
						}
					}
				}
				if (siegeTank != nullptr)
					u->follow(siegeTank);
				else
					u->move(this->mAttackPoint);
			} else if ((!u->isAttacking() || u->isIdle()) && !u->isMoving())
				u->move(this->mAttackPoint);
		}
	}
}

void ExampleAIModule::siegeAITank(Unit u)
{
	Unitset targets = bw->getUnitsInRadius(
		u->getPosition(),
		UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange(),
		Filter::IsEnemy);
	if (targets.size() >= 3)
	{
		int count = 0;
		for (Unit u2 : targets)
		{
			if (!u->isFlying() &&
				u->isTargetable() &&
				u->getDistance(u2) > UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().minRange())
				count++;
		}
		if (count >= 3)
		{
			u->siege();
		}
	}
	else {
		Unit target = bw->getClosestUnit(
			u->getPosition(),
			Filter::IsEnemy);
		if (!u->isHoldingPosition() && target != nullptr && u->isInWeaponRange(target))
		{
			u->holdPosition();
		}
		else if(!this->goHam)
		{
			if (u->getPosition().getDistance(this->mChokePoint) < 20)
			{
				if (u->isIdle())
					u->move(this->mChokePoint);
			}
		}
		else {
			if ((!u->isAttacking() || u->isIdle()) && !u->isMoving())
				u->move(this->mAttackPoint);
		}

	}
}

void ExampleAIModule::siegeAISiege(Unit u)
{
	Unit target = bw->getClosestUnit(u->getPosition(), Filter::IsEnemy);
	if (target == nullptr || !u->isInWeaponRange(target))
	{
		u->unsiege();
	}
}

void ExampleAIModule::medicAI(Unit u)
{
	bool healing = false;
	Unitset needMedic = u->getUnitsInRadius(200, Filter::IsAlly);
	for (Unit u2 : needMedic)
	{
		if (!u2->getType().isBuilding() &&
			u2->getHitPoints() < u2->getType().maxHitPoints() &&
			!u2->isBeingHealed())
		{
			u->rightClick(u2);
			healing = true;
			break;
		}
	}
	if(!healing)
		if (!this->goHam)
		{
			if (u->getPosition().getDistance(this->mChokePoint) > 30)
			{
				if (!u->isMoving())
					u->move(this->mChokePoint);
			}
			else {
				if (u->isIdle())
					u->holdPosition();
			}
		}
		else {
			if (u->getDistance(this->mAttackPoint) > 1500)
			{
				if ((!u->isAttacking() || u->isIdle()) && !u->isMoving())
					u->move(this->mAttackPoint);
			}
			else {
				if (this->mUnitCount.SCV > 0 && (!u->isAttacking() || u->isIdle()) && !u->isFollowing())
				{
					int distance = MAXINT;
					int tempDist = 0;
					Unit siegeTank = nullptr;
					for (Unit u2 : bw->self()->getUnits())
					{
						if (u2->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode ||
							u2->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
						{
							tempDist = u2->getDistance(this->mAttackPoint);
							if (tempDist < distance)
							{
								distance = tempDist;
								siegeTank = u2;
							}
						}
					}
					if (siegeTank != nullptr)
						u->follow(siegeTank);
					else
						u->move(this->mAttackPoint);
				}
				else if ((!u->isAttacking() || u->isIdle()) && !u->isMoving())
					u->move(this->mAttackPoint);
			}
		}
}

void ExampleAIModule::onFrame()
{
	// First, just draw a bunch of information on the screen
	this->drawTerrainData();
	this->drawBunker();
	bw->drawCircle(CoordinateType::Map, this->mChokePoint.x, this->mChokePoint.y,
		20, Color(255, 255, 0));
	bw->drawTextScreen(10, 10, "Build Order Size: %d\nMineralCost: %d\nGasCost: %d\nNext (%d): %s",
		this->mBuildOrder.size(),
		this->mineralCost,
		this->gasCost,
		this->mBuildOrder.size(),
		this->mBuildOrder.back().ut.c_str());
	bw->drawTextScreen(200, 10, "Active Order List (%d):", this->mActiveOrder.size());
	for (int i = 0; i < this->mActiveOrder.size(); i++)
		bw->drawTextScreen(200, 18 + 8 * i, "%s", this->mActiveOrder[i].bo.ut.c_str());

	bw->drawTextScreen(400, 20, "SCV: %d (%d)\nMarine: %d (%d)\nSiege: %d (%d)\nMedic: %d (%d)",
		this->mUnitCount.SCV,
		this->mUnitCap.SCV,
		this->mUnitCount.marine,
		this->mUnitCap.marine,
		this->mUnitCount.siege,
		this->mUnitCap.siege,
		this->mUnitCount.medic,
		this->mUnitCap.medic
	);

	this->updateFunction();
}