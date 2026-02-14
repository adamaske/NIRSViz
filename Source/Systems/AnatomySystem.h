#pragma once
#include "Systems/System.h"

#include "NIRS/Anatomy/Head.h"
#include "NIRS/Anatomy/Cortex.h"

#include "NIRS/Coordinate/CoordinateSystemGenerator.h"


#include "Renderer/Viewport/Viewport3D.h"

#include "NIRS/Anatomy/AnatomyProvider.h"

#include "Renderer/Mesh/Mesh.h"

class AnatomyService;
// TODO : Delete Anatomy Manager, this system takes all of AnatomyManager's roles

class AnatomySystem : public System, public IAnatomyRenderer {
public:
    AnatomySystem(AnatomyService& anat_serv) : 
        anatomy_service_(anat_serv) 
    {};

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(DeltaTime dt) override;
    void OnGUIRender() override;

    void OnEvent(Event& event) override;
    void RenderMenuBar() override;

    void SetupRendering();
    void RenderAnatomy();


	void StopRenderingAnatomy() override;
	void StartRenderingAnatomy() override;

	void GenerateCoordinateSystem();

    const CoordinateSystemGenerator& GetCoordinateSystemGenerator()
		{return *coordinate_generator_; };
    CoordinateSystemGenerator& GetCoordinateSystemGeneratorMuteable()
		{ return *coordinate_generator_; };

    enum DrawMode {
        NONE = 0,
        ANATOMY_NO_COORDINATES = 1,
		ANATOMY_BASIC_COORDINATES = 2,
		ANATOMY_FULL_COORDINATES = 3,
		COORDINATES_ONLY = 4
    };
    void SetDrawMode(DrawMode mode);

private:

    AnatomyService& anatomy_service_;

	Ref<Shader> phong_shader_;
	Ref<Shader> flat_shader_;
    // TODO : Create a generic viewport class ?

	Scope<CoordinateSystemGenerator> coordinate_generator_;
	
    // Manual Landmarks


    // TODO : Merge AtlasLayer and AnatomyViewport and AnatomyManager into AnatomySystem
    Scope<Viewport3D> anatomy_viewport_;


};