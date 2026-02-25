#pragma once
#include "Core/Base.h"

#include "NIRS/Anatomy/Head.h"
#include "NIRS/Anatomy/Cortex.h"
#include "NIRS/Anatomy/AnatomyProvider.h"

class AnatomyService : public IAnatomyProvider {
public:

	AnatomyService() = default;
	~AnatomyService() = default;

	// Load head and cortex anatomy data from disk''
	void LoadHead(const std::filesystem::path& obj_filepath);
	void LoadCortex(const std::filesystem::path& obj_filepath);

	bool HasHead()   const override { return head_   != nullptr; }
	bool HasCortex() const override { return cortex_ != nullptr; }

	const NIRS::Head& GetHead() override { return *head_.get(); };
	NIRS::Head& GetHeadMutable() override { return *head_.get(); };

	const NIRS::Cortex& GetCortex() override {  return *cortex_.get(); };
	NIRS::Cortex& GetCortexMutable() override { return *cortex_.get(); };

private:

	Scope<NIRS::Head> head_;
	Scope<NIRS::Cortex> cortex_;

};