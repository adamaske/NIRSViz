#pragma once

#include "Renderer/Camera.h"
#include "Transform.h"

#include <map>
#include <vector>

class OrbitCamera : public Camera {
public:
	float m_Theta = -100.0f;
	float m_Phi = 22.0f;
	float m_Radius = 460;


	std::unordered_map<std::string, Transform*> orbit_target_map = {};
	std::string orbit_target_name = "";
	Transform* orbit_target = nullptr;

	std::string current_orbit_position = "Default";
	std::vector<std::tuple<std::string, std::tuple<float, float>>> orbit_positions = {
		{"Default",			{-100.0f, 5.0f}},
		{"Anterior",        {-90.0f , 0.0f}},
		{"Posterior",       {90.0f  , 0.0f}},
		{"Left",            {180.0f , 0.0f}},
		{"Right",           {0.0f   , 0.0f}},
		{"Superior",        {90.0f  , 90.0f}},
		{"Inferior",        {90.0f  , -90.0f}}
	};

	OrbitCamera();
	~OrbitCamera();

	void OnUpdate(float dt) override;
	void OnEvent(Event& e) override;
	void OnImGuiRender(bool standalone) override;

	void UpdateViewMatrix() override;
	void UpdateProjectionMatrix() override;

	void InsertTarget(const std::string& name, Transform* target);
	void SetCurrentTarget(const std::string& name);

	void SetOrbitPosition(float theta, float phi, float distance);
	void SetOrbitPosition(const std::string& name);

	void SetTheta(float _theta);
	void SetPhi(float _phi);
	void SetRadius(float _radius);
};