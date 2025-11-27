#pragma once
#include "Systems/System.h"

// This handles the files which are created, loaded, opened, closed etc 
namespace NIRS {
	class ProjectDatabase;
}

class FileSystem : public System {
public:

	void OnAttach() override;
	void OnGUIRender() override;
	void RenderMenuBar() override;


	void NewProject();
	void OpenProject();

	void RenderProjectExplorer();
	
	void RenderProjectDetails(bool standalone);
private:

	Ref<NIRS::ProjectDatabase> m_ProjectDatabase;

};