#include "vars.h"

namespace frick {

	Vars::Vars() {
		vars = this;
	}

	Vars::~Vars() {
		FOV = 104.f;

		GWorld = nullptr;
		playerCharacter = nullptr;
		AACharacter = nullptr;
		localPlayer = nullptr;
	}
}