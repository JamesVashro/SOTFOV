#include "vars.h"

namespace frick {

	Vars::Vars() {
		vars = this;
	}

	Vars::~Vars() {
		FOV = 105.f;
		sniperFOV = 50.f;
		pistolFOV = 85.f;
		blunderFOV = 85.f;
		spyGlassFOV = 28.f;

		GWorld = nullptr;
		playerCharacter = nullptr;
		AACharacter = nullptr;
		localPlayer = nullptr;
		ViewPortClient = nullptr;
	}
}