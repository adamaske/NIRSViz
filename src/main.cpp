#include "pch.h"
#include "Core/Application.h"

int main(int argc, char** argv){
	Log::Init();
	
	ApplicationSpecification spec;
	spec.Name = "NIRS VIZ";
	spec.CommandLineArgs = { argc, argv };

	auto app = CreateRef<Application>(spec);
	app->Run();

	return 0;
}