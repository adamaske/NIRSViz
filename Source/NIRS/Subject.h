#pragma once

// The subject has some files, a .snirf file (or several),
// and an anatomical .nii file, 
// and a some-naming-convetion-anat.obj which is the 
// anatomical mesh 
struct Subject {

	SNIRF snirf;
	MRIImage mri;
	Cortex visual_cortex;

};