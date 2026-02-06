#pragma once
#include "Systems/System.h"

#include "GUI/SNIRFFileLoaderPanel.h"
#include "NIRS/SNIRFProvider.h"

// This handles the files which are created, loaded, opened, closed etc 
namespace NIRS {
	class ProjectDatabase;
}

class FileSystem : public System, public ISNIRFProvider {
public:

	void OnAttach() override;
	void OnGUIRender() override;
	void RenderMenuBar() override;

	void PostInit();

	void UserLoadSNIRF();

	virtual const Ref<SNIRF>& GetLoadedSNIRF() override;

private:
	bool snirf_loader_panel_open_ = false;

	SNIRFFileLoaderPanel* snirf_loader_panel_ = nullptr;

	Ref<SNIRF> loaded_snirf_ = nullptr;
};