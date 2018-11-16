#include "Source/ExampleAIModule.h"
#define bw Broodwar
//This is the method called each frame. This is where the bot's logic
//shall be called.


void ExampleAIModule::onFrame()
{
	bw->printf("It works!");


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