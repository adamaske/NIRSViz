#include "pch.h"
#include "Projection/ProjectionManager.h"

void ProjectionManager::StartProjection(const ProjectionMode& mode)
{
	if (mode != ProjectionMode::VERTEX_BASED) return;


}

void ProjectionManager::StopProjection()
{

}

void ProjectionManager::SetProjectionWavelength(const NIRS::WavelengthType& wavelength)
{
	wavelength_ = wavelength;

}


