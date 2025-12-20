#pragma once
#include "Systems/System.h"

#include "GUI/SNIRFFileLoaderPanel.h"
// This handles the files which are created, loaded, opened, closed etc 
namespace NIRS {
	class ProjectDatabase;
}

class FileSystem : public System {
public:

	void OnAttach() override;
	void OnGUIRender() override;
	void RenderMenuBar() override;

	void PostInit();

	void UserLoadSNIRF();


private:
	bool snirf_loader_panel_open_ = false;
	SNIRFFileLoaderPanel* snirf_loader_panel_ = nullptr;
};