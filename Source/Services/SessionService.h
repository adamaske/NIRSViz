#pragma once

#include "Core/Base.h"
class SNIRFService  ;
class AnatomyService; 

struct SubjectData{
	std::filesystem::path snirf_path;
	std::filesystem::path head_path;
	std::filesystem::path cortex_path;
	std::filesystem::path mri_path;
};

class SessionService {
public:

	SessionService(SNIRFService& sn, AnatomyService& anat) : snirf_(sn), anatomy_(anat) {
		
	};
	~SessionService() = default;

	bool LoadSubject(const SubjectData& data);
	void UnloadSubject();

	bool IsSubjectLoaded() const;

	// TODO : Multi-subject
	// size_t AddSubject(); -> Returns index
	// void SetActiveSubject(size_t key);
	// void UnloadSubject(size_t key);

private:
	SNIRFService& snirf_;
	AnatomyService & anatomy_;
};