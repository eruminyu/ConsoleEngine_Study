#include <Engine/Engine.h>
#include <Level/DrillerLevel.h>

int main()
{
	Craft::Engine engine;
	engine.AddNewLevel<DrillerLevel>();
	engine.Run();
}
