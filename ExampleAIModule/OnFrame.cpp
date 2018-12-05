#include "Source/ExampleAIModule.h"
#define bw Broodwar
//This is the method called each frame. This is where the bot's logic
//shall be called.

void ExampleAIModule::initAI()
{
	bw->setLocalSpeed(5);
	//this->SCVCap = 9;
	//this->marineCap = 0;
	//this->SCVCount = 0;
	//this->marineCount = 0;
	this->SCVGasCap = 2;

	this->mUnitCap =	{ 9,0,0,0 };
	this->mUnitCount =	{ 0,0,0,0 };

	this->mineralCost = 0;
	this->gasCost = 0;

	this->mChokePoint = this->findGuardPoint();
	this->mHomePoint = Position(bw->self()->getStartLocation().x*32, bw->self()->getStartLocation().y*32);
	
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Machine_Shop,	BuildType::Addon,	{ 12,8,3,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Factory,		BuildType::Build,	{ 12,8,3,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Supply_Depot,	BuildType::Build,	{ 12,3,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Bunker,			BuildType::Build,	{ 12,0,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Refinery,		BuildType::Build,	{ 12,0,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Barracks,		BuildType::Build,	{ 12,0,0,0 } });
	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Supply_Depot,	BuildType::Build,	{ 12,0,0,0 } });


	this->findUnit(UnitTypes::Terran_SCV)->move(this->mChokePoint);
	//this->mBuildOrder.push_back(
	//	{ UnitTypes::Terran_Supply_Depot, BuildType::Build, 9, 12, 0 });
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
	/*if (unit->getPlayer() == Broodwar->self())
	{
		Broodwar->sendText("A %s [%x] has been created at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	}*/
	if (unit->getType() == UnitTypes::Terran_SCV)
		this->mUnitCount.SCV++;
	else if (unit->getType() == UnitTypes::Terran_Marine)
		this->mUnitCount.marine++;
	else if (unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
		this->mUnitCount.siege++;
	else
		this->checkCreatedUnit(unit);
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Terran_SCV)
		this->mUnitCount.SCV--;
	else if (unit->getType() == UnitTypes::Terran_Marine)
		this->mUnitCount.marine--;
	else if (unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
		this->mUnitCount.siege--;
	else if (unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
		this->mUnitCount.siege--;
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
	this->checkCreatedUnit(unit);
}

//Called when a unit has been completed, i.e. finished built.
void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Terran_Barracks
		|| unit->getType() == UnitTypes::Terran_Factory)
		unit->setRallyPoint(this->mChokePoint);
	//Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
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

void ExampleAIModule::workLazyWorkers()
{
	for (Unit u : bw->self()->getUnits())
		if (u->getType() == UnitTypes::Terran_SCV)
		{
			if (u->isIdle())
			{
				Unit closestMineral = NULL;
				for (auto m : Broodwar->getMinerals())
				{
					if (closestMineral == NULL || u->getDistance(m) < u->getDistance(closestMineral))
					{
						closestMineral = m;
					}
				}
				if (closestMineral != NULL)
				{
					u->rightClick(closestMineral);
					//Broodwar->printf("Send worker %d to mineral %d", u->getID(), closestMineral->getID());
				}
			}
		}
		else if (u->getType() == UnitTypes::Terran_Bunker)
		{
			if (u->canLoad())
			{
				for (Unit u2 : bw->self()->getUnits())
				{
					if (u2->getType() == UnitTypes::Terran_Marine && !u2->isLoaded())
					{
						u->load(u2);
						break;
					}
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
				bw->printf("RefWorkers: %d", nrRefWorkers);
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
}

void ExampleAIModule::tormentActiveOrders()
{
	for (ActiveOrder ao : this->mActiveOrder)
	{
		if (ao.bo.bt == BuildType::Build)
		{
			if (!ao.u->build(ao.bo.ut, ao.pos))
			{
				if (ao.bo.ut == UnitTypes::Terran_Bunker)
				{
					bw->printf("%s", ao.u->getOrder().c_str());
					//bw->printf("Can't Build %s", ao.bo.ut.c_str());
					//ao.u->move(Position(ao.pos.x*32,ao.pos.y*32));
					bw->drawCircle(CoordinateType::Map, ao.pos.x * 32, ao.pos.y * 32, 16, Color(255, 0, 0), true);
					bw->drawCircle(CoordinateType::Map, ao.u->getPosition().x, ao.u->getPosition().y, 16, Color(255, 255, 0), true);
					if (!bw->canBuildHere(ao.pos, ao.bo.ut))
					{
						bw->printf("I can't %s build here...", ao.bo.ut.c_str());
						ao.pos = bw->getBuildLocation(ao.bo.ut, ao.pos, 5);
					}
				}
			}
		}
	}
}

void ExampleAIModule::checkCreatedUnit(Unit unit)
{
	for (int i = 0; i < this->mActiveOrder.size(); i++)
	{
		if (unit->getType() == UnitTypes::Terran_Refinery)
			bw->printf("REFINERY!!!");
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

void ExampleAIModule::onFrame()
{
	this->drawTerrainData();
	bw->drawCircle(CoordinateType::Map, this->mChokePoint.x, this->mChokePoint.y,
		20, Color(255, 255, 0));
	bw->drawTextScreen(100, 100, "HELLO %d %d", this->mChokePoint.x, this->mChokePoint.y);
	//bw->drawCircle(CoordinateType::Map, this->mHomePoint.x*32, this->mHomePoint.y*32, 4, Color(255, 0, 0), true);
	//bw->printf("It works!");
	bw->drawTextScreen(10, 10, "Build Order Size: %d\nMineralCost: %d\nGasCost: %d\nNext (%d): %s",
		this->mBuildOrder.size(),
		this->mineralCost,
		this->gasCost,
		this->mBuildOrder.size(),
		this->mBuildOrder.back().ut.c_str());
	bw->drawTextScreen(200, 10, "Active Order List (%d):", this->mActiveOrder.size());
	for (int i = 0; i < this->mActiveOrder.size(); i++)
		bw->drawTextScreen(200, 18 + 8 * i, "%s", this->mActiveOrder[i].bo.ut.c_str());
	bw->drawTextScreen(400, 20, "SCV: %d (%d)\nMarine: %d (%d)\nSiege: %d (%d)\n",
		this->mUnitCount.SCV,
		this->mUnitCap.SCV,
		this->mUnitCount.marine,
		this->mUnitCap.marine,
		this->mUnitCount.siege,
		this->mUnitCap.siege
	);
	Unit target;

	this->workLazyWorkers();
	this->tormentActiveOrders();
	
	if (this->mUnitCount.SCV < this->mUnitCap.SCV)
	{
		if((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_SCV.mineralPrice())
			if ((target = findUnit(UnitTypes::Terran_Command_Center)) != nullptr)
				if (!target->isTraining())
					target->train(UnitTypes::Terran_SCV);
	}

	if (this->mUnitCount.siege < this->mUnitCap.siege)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_Siege_Tank_Tank_Mode.mineralPrice())
			if ((bw->self()->gas() - this->gasCost) >= UnitTypes::Terran_Siege_Tank_Tank_Mode.gasPrice())
				if ((target = findUnit(UnitTypes::Terran_Factory)) != nullptr)
					if (!target->isTraining() && !target->canTrain())
						target->train(UnitTypes::Terran_Siege_Tank_Tank_Mode);
	}

	if (this->mUnitCount.marine < this->mUnitCap.marine)
	{
		if ((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_Marine.mineralPrice())
			if ((target = findUnit(UnitTypes::Terran_Barracks)) != nullptr)
				if (!target->isTraining())
					target->train(UnitTypes::Terran_Marine);
	}
	//bw->printf("%d, %d", (int)this->mBuildOrder.back().bt, (int)BuildType::Build);

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
					Position desiredPos = this->mHomePoint;
					int radius = 64;
					if (this->mBuildOrder.back().ut == UnitTypes::Terran_Supply_Depot ||
						this->mBuildOrder.back().ut == UnitTypes::Terran_Bunker)
					{
						//bw->printf("CHOKE %d, %d", this->mChokePoint.x, this->mChokePoint.y);
						//bw->printf("HOME %d, %d", this->mHomePoint.x, this->mHomePoint.y);
						//desiredPos = this->mChokePoint - this->mHomePoint;
						//bw->printf("DESIREDPOS %d, %d", desiredPos.x, desiredPos.y);
						//desiredPos = this->mChokePoint - (desiredPos / 4);
						desiredPos = this->mChokePoint;
						//bw->printf("DESIREDPOS %d, %d", desiredPos.x, desiredPos.y);
						radius = 10;
					}

					TilePosition pos = bw->getBuildLocation(this->mBuildOrder.back().ut,
						TilePosition(desiredPos.x/32,desiredPos.y/32),
						radius);

					bw->printf("The location I picked is %d, %d!", pos.x/32, pos.y/32);
					bw->drawCircle(CoordinateType::Screen, pos.x, pos.y, 3, Color(255,0,0), true);

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
			else if (this->mBuildOrder.back().bt == BuildType::Addon)
			{
				if (this->mBuildOrder.back().ut == UnitTypes::Terran_Machine_Shop)
				{
					if (this->findUnit(UnitTypes::Terran_Factory)->buildAddon(this->mBuildOrder.back().ut))
					{
						this->mBuildOrder.pop_back();
					}

				}
			}
		}
	}


	//if (Broodwar->getFrameCount() % 100 == 0)
	//{
	//	for (auto u : Broodwar->self()->getUnits())
	//	{
	//		if (u->getType().isWorker())
	//		{
	//			Position guardPoint = findGuardPoint();
	//			u->rightClick(guardPoint);
	//			break;
	//		}
	//	}
	//}

	//if (analyzed)
	//{
	//	drawTerrainData();
	//}
}