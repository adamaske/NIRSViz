#include "pch.h"
#include "Services/AnatomyService.h"

void AnatomyService::LoadHead(const std::filesystem::path& obj_filepath) {
	head_ = CreateScope<NIRS::Head>(obj_filepath);
}

void AnatomyService::LoadCortex(const std::filesystem::path& obj_filepath) {
	cortex_ = CreateScope<NIRS::Cortex>(obj_filepath);
}