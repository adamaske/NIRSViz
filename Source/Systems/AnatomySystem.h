#pragma once
#include "Systems/System.h"

#include "NIRS/Anatomy/Head.h"
#include "NIRS/Anatomy/Cortex.h"

#include "NIRS/Coordinate/CoordinateSystemGenerator.h"


#include "Renderer/Viewport/Viewport3D.h"

#include "NIRS/Anatomy/AnatomyProvider.h"

#include "Renderer/Mesh/Mesh.h"

// TODO : The anatomy system can be told by other system when to start/stop rendering anatomy
class IAnatomyRenderer {
public:
    virtual void StopRenderingAnatomy() = 0;
	virtual void StartRenderingAnatomy() = 0;
};

// TODO : Delete Anatomy Manager, this system takes all of AnatomyManager's roles

class AnatomySystem : public System, public IAnatomyProvider {
public:
    AnatomySystem();
    ~AnatomySystem();

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(DeltaTime dt) override;
    void OnGUIRender() override;

    void OnEvent(Event& event) override;
    void RenderMenuBar() override;

    void SetupRendering();
    void RenderAnatomy();

	void GenerateCoordinateSystem();

    const NIRS::Head& GetHead() override;
    const NIRS::Cortex& GetCortex() override;

	NIRS::Head& GetHeadMutable() override;
	NIRS::Cortex& GetCortexMutable() override;

    const CoordinateSystemGenerator& GetCoordinateSystemGenerator() { return *coordinate_generator_; };
    CoordinateSystemGenerator& GetCoordinateSystemGeneratorMuteable() { return *coordinate_generator_; };

    enum DrawMode {
        NONE = 0,
        ANATOMY_NO_COORDINATES = 1,
		ANATOMY_BASIC_COORDINATES = 2,
		ANATOMY_FULL_COORDINATES = 3,
		COORDINATES_ONLY = 4
    };
    void SetDrawMode(DrawMode mode);
private:
	Ref<Shader> phong_shader_;
	Ref<Shader> flat_shader_;
    // TODO : Create a generic viewport class ?

	Scope<CoordinateSystemGenerator> coordinate_generator_;
	
    // Manual Landmarks


    // TODO : Merge AtlasLayer and AnatomyViewport and AnatomyManager into AnatomySystem
    Scope<Viewport3D> anatomy_viewport_;

	Mesh mesh_test_;
	void RenderTestMesh();
};