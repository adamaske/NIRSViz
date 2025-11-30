#include "pch.h"
#include "Core/Application.h"

#include <windows.h>

int main(int argc, char** argv){
	Log::Init();
	
	ApplicationSpecification spec;
	spec.name = "NIRSViz";
	spec.args = { argc, argv };

	Application app(spec);
	app.Run();

	return 0;
}