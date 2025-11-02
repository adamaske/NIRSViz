#include "pch.h"
#include "App/Controllers/CoordinateSystemController.h"
#include "Core/Log.h"
#include "Events/EventBus.h"
namespace App {

	CoordinateSystemController::CoordinateSystemController()
	    : m_CoordinateSystem(), m_Sampler()
	{
	}
	void CoordinateSystemController::GenerateCoordinateSystem(NIRS::Head* head)
	{
	    if (!head) {
	        NVIZ_ERROR("Cannot generate coordinate system: No head model");
	        return;
	    }
	    // Verify graph connectivity
	    if (!IsGraphConnected(*head->GetGraph(), head->GetMesh()->GetVertices().size())) {
	        NVIZ_ERROR("Head graph is not fully connected. Pathfinding may fail.");
	    }

	    NVIZ_INFO("Generating 10-20 coordinate system...");
	    // Generate each component
	    GenerateSagittalPath(head);
	    GenerateCoronalPath(head);
	    //GenerateCircumferencePaths(head);
	    m_CoordinateSystem.SetGenerated(true);
	    NVIZ_INFO("Coordinate system generation complete");

		EventBus::Instance().Publish<OnCoordinateSystemGenerated>({});
	}

	void CoordinateSystemController::GenerateSagittalPath(NIRS::Head* head)
	{
		using namespace NIRS;

		auto manualLandmarks = m_CoordinateSystem.GetManualLandmarks();
		auto worldVertices = head->GetWorldSpaceVertexPositions();

		auto naison = manualLandmarks.GetLandmark(ManualLandmarkType::Nasion);
		auto inion = manualLandmarks.GetLandmark(ManualLandmarkType::Inion);

		auto nz = naison.Position;
		auto iz = inion.Position;

		const auto& nz_iz_midpoint =  (nz + iz) / 2.0f;
		const auto& nz_iz_direction = glm::normalize(iz - nz);
		const auto& nz_iz_rotation_axis = glm::normalize(glm::cross(nz_iz_direction, glm::vec3(0, 1, 0)));


		auto rays = m_Sampler.GenerateSweepingArchRays(nz_iz_midpoint, nz_iz_direction, nz_iz_rotation_axis);

		auto intersections = m_Sampler.FindIntersections(
			rays,
			worldVertices,
			head->GetMesh()->GetIndices()
		);
		
		auto roughPath = m_Sampler.IntersectionsToVertexPath(intersections, worldVertices);

		auto finePath = PathFinder::RefinePath(*head->GetGraph(), roughPath);
        finePath.insert(finePath.begin(), head->FindClosestVertex(naison.Position));
        finePath.push_back(head->FindClosestVertex(inion.Position));

		CoordinateSystem::PathData path;
		path.Rays = rays;
		path.IntersectionPoints = intersections;
		path.VertexIndices = finePath;

		m_CoordinateSystem.SetSagittalPath(path);

		std::vector<Landmark> labels = { Nz, Fpz, Fz, Cz, Pz, Oz, Iz };
		std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.50f, 0.70f, 0.90f, 1.0f };

		auto calculatedLandmarks = LandmarkCalculator::CalculateLandmarksAlongPath(
			worldVertices, finePath, labels, percentages
		);

		auto& landmarks = m_CoordinateSystem.GetLandmarks();
		for (auto& [label, position] : calculatedLandmarks) {

			LandmarkData data;
			data.Type = label;
			data.Position = position;
			data.ClosestVertexIndex = head->FindClosestVertex(position);
			data.IsVisible = true;

			landmarks.SetLandmark(label, data);
		}

		for (auto& landmark : m_CoordinateSystem.GetLandmarks().GetAllLandmarks()) {

			auto type = landmark.Type;
			auto position = landmark.Position;

			NVIZ_INFO("[{}] : ( {:.2f}, {:.2f}, {:.2f} )", NIRS::LandmarkToString(type), position.x, position.y, position.z);
		}
    }

	void CoordinateSystemController::GenerateCoronalPath(NIRS::Head* head)
	{
		using namespace NIRS;
		auto landmarks = m_CoordinateSystem.GetLandmarks();
		auto manualLandmarks = m_CoordinateSystem.GetManualLandmarks();

		auto worldVertices = head->GetWorldSpaceVertexPositions();

		auto lpa = manualLandmarks.GetLandmark(ManualLandmarkType::LPA);
		auto rpa = manualLandmarks.GetLandmark(ManualLandmarkType::RPA);

		auto lpa_pos = lpa.Position;
		auto rpa_pos = rpa.Position;

		glm::vec3 lpa_rpa_midpoint = glm::vec3((lpa_pos + rpa_pos) / 2.0f);
		glm::vec3 lpa_rpa_direction = glm::normalize(rpa_pos - lpa_pos);

		auto cz = GetCoordinateSystem().GetLandmarks().GetLandmark(NIRS::Cz)->Position;
		cz.x = lpa_rpa_midpoint.x; // Ensure Cz is aligned with LPA-RPA midpoint

		NVIZ_INFO("CORNOAL CZ POSITION : ( {:.2f}, {:.2f}, {:.2f} )", cz.x, cz.y, cz.z);
		glm::vec3 up_vector				= glm::normalize(cz - lpa_rpa_midpoint);
		glm::vec3 lpa_rpa_rotation_axis = glm::normalize(glm::cross(lpa_rpa_direction, up_vector));
		glm::vec3 lpa_rpa_new_direction = glm::normalize(glm::cross(lpa_rpa_rotation_axis, -up_vector));
	
		auto rays = m_Sampler.GenerateSweepingArchRays(lpa_rpa_midpoint, lpa_rpa_new_direction, lpa_rpa_rotation_axis);
		NVIZ_INFO("CORONAL RAYS : {}", rays.size());

		auto intersections = m_Sampler.FindIntersections(
			rays,
			worldVertices,
			head->GetMesh()->GetIndices()
		);

		if (intersections.size() == 0) {
						NVIZ_WARN("No intersections found for coronal path generation.");
						CoordinateSystem::PathData path;
						path.Rays = rays;

						m_CoordinateSystem.SetCoronalPath(path);

						return;
		}

		NVIZ_INFO("CORONAL INTERSECTIONS : {}", intersections.size());
		auto roughPath = m_Sampler.IntersectionsToVertexPath(intersections, worldVertices);

		NVIZ_INFO("CORONAL roughPath : {}", roughPath.size());

		auto finePath = PathFinder::RefinePath(*head->GetGraph(), roughPath);
		finePath.insert(finePath.begin(), head->FindClosestVertex(lpa.Position));
		finePath.push_back(head->FindClosestVertex(rpa.Position));


		CoordinateSystem::PathData path;
		path.Rays = rays;
		path.IntersectionPoints = intersections;
		path.VertexIndices = finePath;

		m_CoordinateSystem.SetCoronalPath(path);

		std::vector<Landmark> labels = { LPA, T3, C3, C4, T4, RPA };
		std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.70f, 0.90f, 1.0f };

		auto calculatedLandmarks = LandmarkCalculator::CalculateLandmarksAlongPath(
			worldVertices, finePath, labels, percentages
		);

		for (auto& [label, position] : calculatedLandmarks) {
			LandmarkData data;
			data.Type = label;
			data.Position = position;
			data.ClosestVertexIndex = head->FindClosestVertex(position);
			data.IsVisible = true;
			landmarks.SetLandmark(label, data);
		}
	}

    void CoordinateSystemController::GenerateCircumferencePaths(NIRS::Head* head)
    {
		using namespace NIRS;
		auto landmarkRegistry = m_CoordinateSystem.GetLandmarks();
		auto worldVertices = head->GetWorldSpaceVertexPositions();


		auto roughPath = std::vector<unsigned int>();//m_Sampler.IntersectionsToVertexPath(intersections, worldVertices);


		auto finePath = PathFinder::RefinePath(*head->GetGraph(), roughPath);


		CoordinateSystem::PathData path;
		path.VertexIndices = finePath;

		m_CoordinateSystem.SetCoronalPath(path);

		std::vector<Landmark> labels = { LPA, T3, C3, C4, T4, RPA };
		std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.70f, 0.90f, 1.0f };

		auto calculatedLandmarks = LandmarkCalculator::CalculateLandmarksAlongPath(
			worldVertices, finePath, labels, percentages
		);

		for (auto& [label, position] : calculatedLandmarks) {
			LandmarkData data;
			data.Type = label;
			data.Position = position;
			data.ClosestVertexIndex = head->FindClosestVertex(position);
			data.IsVisible = true;
			landmarkRegistry.SetLandmark(label, data);
		}

        // Similar implementation for circumference paths
        // ... (follow same pattern)

		// Step 3. 
	// Now we have The saggital plane : Nz to Iz path
	// And T3, C3, C4, T4.
	// Now we can find { "FpZ", "T3", "Oz", "T4"};

	// We may want to split this into two section

		//VertexPath m_LeftHorizontalRoughPath = { m_LandmarkClosestVertexIndexMap[NIRS::Fpz],
		//											m_LandmarkClosestVertexIndexMap[NIRS::T3],
		//											m_LandmarkClosestVertexIndexMap[NIRS::Oz]
		//};


		//VertexPath m_RightHorizontalRoughPath = { m_LandmarkClosestVertexIndexMap[NIRS::Oz],
		//											m_LandmarkClosestVertexIndexMap[NIRS::T4],
		//											m_LandmarkClosestVertexIndexMap[NIRS::Fpz]
		//};

		//VertexPath m_LeftHorizontalFinePath;
		//VertexPath m_RightHorizontalFinePath;

		//for (unsigned int i = 0; i < m_LeftHorizontalRoughPath.size() - 1; i++)
		//{
		//	auto start = m_LeftHorizontalRoughPath[i];
		//	auto end = m_LeftHorizontalRoughPath[i + 1];
		//	auto path = DjikstraShortestPath(*m_Head->Graph, start, end);

		//	for (auto& step : path) {
		//		m_LeftHorizontalFinePath.push_back(step);
		//	}
		//}

		//for (unsigned int i = 0; i < m_RightHorizontalRoughPath.size() - 1; i++)
		//{
		//	auto start = m_RightHorizontalRoughPath[i];
		//	auto end = m_RightHorizontalRoughPath[i + 1];
		//	auto path = DjikstraShortestPath(*m_Head->Graph, start, end);

		//	for (auto& step : path) {
		//		m_RightHorizontalFinePath.push_back(step);
		//	}
		//}
		//// Invert finepath
		////m_LeftHorizontalFinePath = std::vector<unsigned int>(m_LeftHorizontalFinePath.rbegin(), m_LeftHorizontalFinePath.rend());
		////m_RightHorizontalFinePath = std::vector<unsigned int>(m_RightHorizontalFinePath.rbegin(), m_RightHorizontalFinePath.rend());

		//std::vector<NIRS::Line> horizontal_path_lines;
		//for (unsigned int i = 0; i < m_RightHorizontalFinePath.size() - 1; i++)
		//{
		//	auto start = world_space_vertices[m_RightHorizontalFinePath[i]];
		//	auto end = world_space_vertices[m_RightHorizontalFinePath[i + 1]];
		//	horizontal_path_lines.push_back({ start, end });
		//}
		//for (unsigned int i = 0; i < m_LeftHorizontalFinePath.size() - 1; i++)
		//{
		//	auto start = world_space_vertices[m_LeftHorizontalFinePath[i]];
		//	auto end = world_space_vertices[m_LeftHorizontalFinePath[i + 1]];
		//	horizontal_path_lines.push_back({ start, end });
		//}
		//m_CalculatedPathRenderer->SubmitLines(horizontal_path_lines);

		//// We dont need to flip these
		////m_HorizontalFinePath.insert(m_LPARPAFinePath.begin(), landmark_vertex_indices[ManualLandmarkType::LPA]);
		////m_HorizontalFinePath.push_back(landmark_vertex_indices[ManualLandmarkType::RPA]);


		//{ // Left Hemisphere
		//	using namespace NIRS;

		//	//std::vector<NIRS::Landmark> labels = { Fp1, F7, T5, O1, O2, T6, F8, Fp2 };
		//	//std::vector<float> percentages = { 0.05, 0.15, 0.35, 0.45, 0.55, 0.65, 0.85, 0.95 };

		//	std::vector<NIRS::Landmark> labels = { Fp1, F7, T5, O1 };
		//	std::vector<float> percentages = { 0.10, 0.30, 0.70, 0.90 };

		//	auto coordinates = FindReferencePointsAlongPath(world_space_vertices, m_LeftHorizontalFinePath, labels, percentages);
		//	for (auto& [label, position] : coordinates) {
		//		m_Landmarks[label] = position;
		//		m_LandmarkVisibility[label] = true;
		//	};
		//}

		//{ // Right Hemisphere
		//	using namespace NIRS;

		//	std::vector<NIRS::Landmark> labels = { O2, T6, F8, Fp2 };
		//	std::vector<float> percentages = { 0.10, 0.30, 0.70, 0.90 };

		//	auto coordinates = FindReferencePointsAlongPath(world_space_vertices, m_RightHorizontalFinePath, labels, percentages);
		//	for (auto& [label, position] : coordinates) {
		//		m_Landmarks[label] = position;
		//		m_LandmarkVisibility[label] = true;
		//	};
		//}

		//m_LandmarkRenderer->Clear();
		//for (auto& [label, position] : m_Landmarks) {
		//	if (m_LandmarkVisibility[label]) m_LandmarkRenderer->SubmitPoint({ position });
		//};

	}

} // namespace App