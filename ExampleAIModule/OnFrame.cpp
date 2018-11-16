#include "Source/ExampleAIModule.h"
#define bw Broodwar
//This is the method called each frame. This is where the bot's logic
//shall be called.

void ExampleAIModule::initAI()
{
	bw->setLocalSpeed(20);
	this->SCVCap = 9;
	this->marineCap = 0;
	this->SCVCount = 0;
	this->marineCount = 0;

	this->mineralCost = 0;

	this->mBuildOrder.push_back(
		{ UnitTypes::Terran_Supply_Depot, BuildType::Build, 9, 12, 0 });
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
		this->SCVCount++;
	else if (unit->getType() == UnitTypes::Terran_Marine)
		this->marineCount++;
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Terran_SCV)
		this->SCVCount--;
	else if (unit->getType() == UnitTypes::Terran_Marine)
		this->marineCount--;
}

//Called when a unit has been completed, i.e. finished built.
void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}


Unit ExampleAIModule::findUnit(UnitType ut)
{
	for (Unit u : bw->self()->getUnits())
		if (u->getType() == ut)
			return u;
	return nullptr;
}

void ExampleAIModule::workLazyWorkers()
{
	for (Unit u : bw->self()->getUnits())
		if (u->getType() == UnitTypes::Terran_SCV)
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
					Broodwar->printf("Send worker %d to mineral %d", u->getID(), closestMineral->getID());
				}
			}
}

void ExampleAIModule::onFrame()
{
	//bw->printf("It works!");
	bw->drawTextScreen(10, 10, "Build Order Size: %d\nSCV: %d\nMarine: %d",
		this->mBuildOrder.size(),
		this->SCVCount,
		this->marineCount);
	Unit target;

	this->workLazyWorkers();
	
	if (this->SCVCount < this->SCVCap)
	{
		if((bw->self()->minerals() - this->mineralCost) >= UnitTypes::Terran_SCV.mineralPrice())
			if ((target = findUnit(UnitTypes::Terran_Command_Center)) != nullptr)
				if (!target->isTraining())
					target->train(UnitTypes::Terran_SCV);
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