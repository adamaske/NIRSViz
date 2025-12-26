#include "pch.h"
#include "Core/Application.h"
#include <QApplication>

int main(int argc, char** argv){
	QApplication qapp(argc, argv);

	ApplicationSpecification spec;
	spec.name = "NIRSViz";
	spec.working_directory = "C:/dev/NIRSViz/";
	spec.args = { argc, argv };

	Application app(spec);
	app.show();

	return qapp.exec();
}