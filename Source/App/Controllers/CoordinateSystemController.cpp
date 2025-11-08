#include "pch.h"
#include "App/Controllers/CoordinateSystemController.h"
#include "Core/Log.h"
#include "Events/EventBus.h"

#include "NIRS/Anatomy/AnatomyManager.h"
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
	    GenerateCircumferencePaths(head);
		GenerateF3F4();
		GenerateP3P4();
	    m_CoordinateSystem.SetGenerated(true);

		EventBus::Instance().Publish<OnCoordinateSystemGenerated>({});


		for (auto& landmark : m_CoordinateSystem.GetLandmarks().GetAllLandmarks()) {

			auto type = landmark.Type;
			auto position = landmark.Position;

			NVIZ_INFO("[{}] : ( {:.2f}, {:.2f}, {:.2f} )", NIRS::LandmarkToString(type), position.x, position.y, position.z);
		}
		NVIZ_INFO("Coordinate system generation complete");
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
		std::reverse(roughPath.begin(), roughPath.end());
		roughPath.insert(roughPath.begin(), head->FindClosestVertex(naison.Position));
		roughPath.push_back(head->FindClosestVertex(inion.Position));

		auto finePath = PathFinder::RefinePath(*head->GetGraph(), roughPath);


		CoordinateSystem::PathData path;
		path.Rays = rays;
		path.IntersectionPoints = intersections;
		path.FineVertexPath = finePath;
		path.RoughVertexPath = roughPath;

		m_CoordinateSystem.SetSagittalPath(path);

		std::vector<Landmark> labels = { Nz, Fpz, Fz, Cz, Pz, Oz, Iz };
		std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.50f, 0.70f, 0.90f, 1.00f };

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
    }

	void CoordinateSystemController::GenerateCoronalPath(NIRS::Head* head)
	{
		using namespace NIRS;
		auto& landmarks = m_CoordinateSystem.GetLandmarks();
		auto& manualLandmarks = m_CoordinateSystem.GetManualLandmarks();

		auto& worldVertices = head->GetWorldSpaceVertexPositions();

		auto lpa = manualLandmarks.GetLandmark(ManualLandmarkType::LPA);
		auto rpa = manualLandmarks.GetLandmark(ManualLandmarkType::RPA);

		auto lpa_pos = lpa.Position;
		auto rpa_pos = rpa.Position;

		glm::vec3 lpa_rpa_midpoint = glm::vec3((lpa_pos + rpa_pos) / 2.0f);
		glm::vec3 lpa_rpa_direction = glm::normalize(rpa_pos - lpa_pos);

		auto cz = GetCoordinateSystem().GetLandmarks().GetLandmark(NIRS::Cz)->Position;
		cz.x = lpa_rpa_midpoint.x; // Ensure Cz is aligned with LPA-RPA midpoint

		glm::vec3 up_vector				= glm::normalize(cz - lpa_rpa_midpoint);
		glm::vec3 lpa_rpa_rotation_axis = glm::normalize(glm::cross(lpa_rpa_direction, up_vector));
		glm::vec3 lpa_rpa_new_direction = glm::normalize(glm::cross(lpa_rpa_rotation_axis, -up_vector));
	
		auto rays = m_Sampler.GenerateSweepingArchRays(lpa_rpa_midpoint, lpa_rpa_new_direction, lpa_rpa_rotation_axis);

		auto intersections = m_Sampler.FindIntersections(
			rays,
			worldVertices,
			head->GetMesh()->GetIndices()
		);

		auto roughPath = m_Sampler.IntersectionsToVertexPath(intersections, worldVertices);
		std::reverse(roughPath.begin(), roughPath.end());
		roughPath.insert(roughPath.begin(), head->FindClosestVertex(lpa.Position));
		roughPath.push_back(head->FindClosestVertex(rpa.Position));

		auto finePath = PathFinder::RefinePath(*head->GetGraph(), roughPath);


		CoordinateSystem::PathData path;
		path.Rays = rays;
		path.IntersectionPoints = intersections;
		path.FineVertexPath = finePath;
		path.RoughVertexPath = roughPath;
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
		auto& landmarkRegistry = m_CoordinateSystem.GetLandmarks();
		auto& worldVertices = head->GetWorldSpaceVertexPositions();

		auto fpz = landmarkRegistry.GetLandmark(NIRS::Fpz);
		auto t3 = landmarkRegistry.GetLandmark(NIRS::T3);
		auto oz = landmarkRegistry.GetLandmark(NIRS::Oz);
		auto t4 = landmarkRegistry.GetLandmark(NIRS::T4);
		std::vector<unsigned int> leftRoughPath = {
			fpz->ClosestVertexIndex, 
			t3->ClosestVertexIndex, 
			oz->ClosestVertexIndex, 
		};

		std::vector<unsigned int> rightRoughPath = {
			fpz->ClosestVertexIndex,
			t4->ClosestVertexIndex,
			oz->ClosestVertexIndex,
		};

		auto leftFinePath = PathFinder::RefinePath(*head->GetGraph(), leftRoughPath);
		auto rightFinePath = PathFinder::RefinePath(*head->GetGraph(), rightRoughPath);

		CoordinateSystem::PathData leftPath;
		leftPath.FineVertexPath = leftFinePath;
		leftPath.RoughVertexPath = leftRoughPath;

		CoordinateSystem::PathData rightPath;
		rightPath.FineVertexPath = rightFinePath;
		rightPath.RoughVertexPath = rightRoughPath;

		m_CoordinateSystem.SetCircumferencePaths({ leftPath, rightPath });

		{
			std::vector<NIRS::Landmark> labels = { Fp1, F7, T5, O1 };
			std::vector<float> percentages = { 0.10, 0.30, 0.70, 0.90 };
			auto calculatedLandmarks = LandmarkCalculator::CalculateLandmarksAlongPath(
				worldVertices, leftFinePath, labels, percentages
			);

			for (auto& [label, position] : calculatedLandmarks) {
				LandmarkData data;
				data.Type = label;
				data.Position = position;
				data.ClosestVertexIndex = head->FindClosestVertex(position);
				data.IsVisible = true;
				landmarkRegistry.SetLandmark(label, data);
			}
		}
		{
			std::vector<NIRS::Landmark> labels = { Fp2, F8, T6, O2 };
			std::vector<float> percentages = { 0.10, 0.30, 0.70, 0.90 };
			auto calculatedLandmarks = LandmarkCalculator::CalculateLandmarksAlongPath(
				worldVertices, rightFinePath, labels, percentages
			);

			for (auto& [label, position] : calculatedLandmarks) {
				LandmarkData data;
				data.Type = label;
				data.Position = position;
				data.ClosestVertexIndex = head->FindClosestVertex(position);
				data.IsVisible = true;
				landmarkRegistry.SetLandmark(label, data);
			}
		}
	}

	void CoordinateSystemController::GenerateF3F4()
	{
		using namespace NIRS;
		auto head = AnatomyManager::Instance().GetHead();
		auto worldVertices = head->GetWorldSpaceVertexPositions();
		  

		auto& landmarks = m_CoordinateSystem.GetLandmarks();
		auto fz = landmarks.GetLandmark(NIRS::Fz);

		// F3 is in the point where F7-Fz meets Fp1-C3
		auto f7 = landmarks.GetLandmark(NIRS::F7);
		auto fp1 = landmarks.GetLandmark(NIRS::Fp1);
		auto c3 = landmarks.GetLandmark(NIRS::C3);

		// F4 is the point where F8-Fz meets Fp2-C4
		auto f8 = landmarks.GetLandmark(NIRS::F8);
		auto fp2 = landmarks.GetLandmark(NIRS::Fp2);
		auto c4 = landmarks.GetLandmark(NIRS::C4);

		using VertexPath = std::vector<unsigned int>;
		VertexPath f7_fz_path = {
			f7->ClosestVertexIndex,
			fz->ClosestVertexIndex
		};

		VertexPath fp1_c3_path = {
			fp1->ClosestVertexIndex,
			c3->ClosestVertexIndex
		};

		VertexPath f8_fz_path = {
			f8->ClosestVertexIndex,
			fz->ClosestVertexIndex
		};
		VertexPath fp2_c4_path = {
			fp2->ClosestVertexIndex,
			c4->ClosestVertexIndex
		};

		auto f7_fz_fine = PathFinder::RefinePath(*head->GetGraph(), f7_fz_path);
		auto fp1_c3_fine = PathFinder::RefinePath(*head->GetGraph(), fp1_c3_path);

		auto f8_fz_fine = PathFinder::RefinePath(*head->GetGraph(), f8_fz_path);
		auto fp2_c4_fine = PathFinder::RefinePath(*head->GetGraph(), fp2_c4_path);


		auto f7fz = LandmarkCalculator::FindPointAtPercentage(worldVertices, f7_fz_fine, 0.5f);
		auto fp1c3 = LandmarkCalculator::FindPointAtPercentage(worldVertices, fp1_c3_fine, 0.5f);

		auto f8fz = LandmarkCalculator::FindPointAtPercentage(worldVertices, f8_fz_fine, 0.5f);
		auto fp2c4 = LandmarkCalculator::FindPointAtPercentage(worldVertices, fp2_c4_fine, 0.5f);

		//
		auto f3 = (f7fz + fp1c3) / 2.0f;
		auto f4 = (f8fz + fp2c4) / 2.0f;

		LandmarkData f3Data;
		f3Data.Type = F3;
		f3Data.Position = f3;
		f3Data.ClosestVertexIndex = head->FindClosestVertex(fp1c3);
		f3Data.IsVisible = true;
		landmarks.SetLandmark(F3, f3Data);
		LandmarkData f4Data;
		f4Data.Type = F4;
		f4Data.Position = f4;
		f4Data.ClosestVertexIndex = head->FindClosestVertex(fp2c4);
		f4Data.IsVisible = true;
		landmarks.SetLandmark(F4, f4Data);
		// Now generate the F3-F4 path
	}

	void CoordinateSystemController::GenerateP3P4()
	{

		using namespace NIRS;
		auto head = AnatomyManager::Instance().GetHead();
		auto worldVertices = head->GetWorldSpaceVertexPositions();
		auto& landmarks = m_CoordinateSystem.GetLandmarks();

		auto pz = landmarks.GetLandmark(NIRS::Pz);
		// P3 is where T5-Pz meets O1-C3
		auto t5 = landmarks.GetLandmark(NIRS::T5);	
		auto o1 = landmarks.GetLandmark(NIRS::O1);
		auto c3 = landmarks.GetLandmark(NIRS::C3);

		// P4 is where T6-Pz meets O2-C4
		auto t6 = landmarks.GetLandmark(NIRS::T6);
		auto o2 = landmarks.GetLandmark(NIRS::O2);
		auto c4 = landmarks.GetLandmark(NIRS::C4);

		using VertexPath = std::vector<unsigned int>;
		VertexPath t5_Pz_path = {
			t5->ClosestVertexIndex,
			pz->ClosestVertexIndex
		};

		VertexPath o1_c3_path = {
			o1->ClosestVertexIndex,
			c3->ClosestVertexIndex
		};

		VertexPath t6_Pz_path = {
			t6->ClosestVertexIndex,
			pz->ClosestVertexIndex
		};
		VertexPath o2_c4_path = {
			o2->ClosestVertexIndex,
			c4->ClosestVertexIndex
		};


		auto t5_Pz_fine = PathFinder::RefinePath(*head->GetGraph(), t5_Pz_path);
		auto o1_c3_fine = PathFinder::RefinePath(*head->GetGraph(), o1_c3_path);

		auto t6_Pz_fine = PathFinder::RefinePath(*head->GetGraph(), t6_Pz_path);
		auto o2_c4_fine = PathFinder::RefinePath(*head->GetGraph(), o2_c4_path);


		auto t5pz = LandmarkCalculator::FindPointAtPercentage(worldVertices, t5_Pz_fine, 0.5f);
		auto o1c3 = LandmarkCalculator::FindPointAtPercentage(worldVertices, o1_c3_fine, 0.5f);

		auto t6pz = LandmarkCalculator::FindPointAtPercentage(worldVertices, t6_Pz_fine, 0.5f);
		auto o2c4 = LandmarkCalculator::FindPointAtPercentage(worldVertices, o2_c4_fine, 0.5f);

		//
		auto p3 = (t5pz + o1c3) / 2.0f;
		auto p4 = (t6pz + o2c4) / 2.0f;

		LandmarkData p3Data;
		p3Data.Type = P3;
		p3Data.Position = p3;
		p3Data.ClosestVertexIndex = head->FindClosestVertex(o1c3);
		p3Data.IsVisible = true;
		landmarks.SetLandmark(P3, p3Data);

		LandmarkData p4Data;
		p4Data.Type = P4;
		p4Data.Position = p4;
		p4Data.ClosestVertexIndex = head->FindClosestVertex(o2c4);
		p4Data.IsVisible = true;
		landmarks.SetLandmark(P4, p4Data);

	}

} // namespace App