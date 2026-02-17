#pragma once
#include <vector>
#include "Core/Base.h"
#include "MRI/MRIImage.h"

#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/IndexBuffer.h"
#include "Renderer/Buffer/VertexBuffer.h"

#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Texture.h"

#include "App/Data/Transform.h"

enum class SlicePlane {
	Axial = 0,
	Sagittal = 1,
	Coronal = 2
};

struct Slice {
	Transform transform;
	glm::mat4 world_transform;
	SlicePlane plane = SlicePlane::Axial;
	float position = 0.5f; // 0.0 to 1.0
	Ref<Texture> texture;
};

namespace NVMRI {



	class MRISliceViewer {
	public:
		MRISliceViewer() {};
		~MRISliceViewer() {};

		void OnAttach(NVMRI::MRIImage* mri_image);
		void OnUpdate(DeltaTime dt);
		void Render(bool standalone);

		void DrawSlice(Slice& slice);
		void UpdateSliceTexture(int sliceIndex);

		glm::mat4 ComputeSliceTransform(
			SlicePlane plane, float normalized_position) const;
	private:
		std::vector<uint8_t> ExtractSliceRGBA8(SlicePlane plane, uint32_t sliceIdx);

		float image_scalar_ = 1.0f; // For window/level adjustments
	public:
		std::vector<Slice> slices_;
		MRIImage* mri_image_ = nullptr;

		Ref<VertexArray> plane_vao_;
		Ref<VertexBuffer> plane_vbo_;
		Ref<IndexBuffer> plane_ibo_;
		Ref<Shader> slice_shader_;
	};
}