#pragma once
#include "Systems/System.h"

#include "GUI/SNIRFFileLoaderPanel.h"

// This handles the files which are created, loaded, opened, closed etc
namespace NIRS {
	class ProjectDatabase;
}

struct NewProjectDialogState {
	bool open = false;
	char snirf_path[512]  = {};
	char head_path[512]   = {};
	char cortex_path[512] = {};
	char mri_path[512]    = {};
};

class FileSystem : public System {
public:

	void OnAttach() override;
	void OnGUIRender() override;
	void RenderMenuBar() override;

	void UserLoadSNIRF();

private:
	bool snirf_loader_panel_open_ = false;
	SNIRFFileLoaderPanel snirf_loader_panel_ = {};

	NewProjectDialogState new_project_dialog_;

	void RenderNewProjectDialog();
	void HandleOpenProject();
};