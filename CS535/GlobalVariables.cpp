#include "GlobalVariables.h"

GlobalVariables* GlobalVariables::_instance = nullptr;

GlobalVariables* GlobalVariables::Instance()
{
	if (!_instance)
	{
		_instance = new GlobalVariables();
	}

	return _instance;
}

GlobalVariables::GlobalVariables()
{
	curScene = nullptr;
}
