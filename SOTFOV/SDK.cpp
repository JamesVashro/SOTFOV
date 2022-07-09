#include "SDK/SDK.h"
#include "Hooking.h"

std::string UObject::GetName() const
{
	std::string name(Name.GetName());
	if (Name.Number > 0)
	{
		name += '_' + std::to_string(Name.Number);
	}

	auto pos = name.rfind('/');
	if (pos == std::string::npos)
	{
		return name;
	}

	return name.substr(pos + 1);
}

std::string UObject::GetFullName() const
{
	std::string name;

	if (Class != nullptr)
	{
		std::string temp;
		for (auto p = Outer; p; p = p->Outer)
		{
			temp = p->GetName() + "." + temp;
		}

		name = Class->GetName();
		name += " ";
		name += temp;
		name += GetName();
	}

	return name;
}

bool UObject::IsA(std::string cmp) const
{
	if (this->GetName().find(cmp) != std::string::npos)
		return true;

	return false;
}


bool InitializeNames() {
	static BYTE sig[] = { 0x48, 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xDB, 0x75, 0x3D };
	GNames = *reinterpret_cast<TNameEntryArray**>(FindPointer(sig, sizeof(sig)));
	std::cout << "GNames: " << GNames << std::endl;

	if (GNames)
		return true;

	return false;
}
bool InitializeObjects() {
	static BYTE sig[] = { 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x5D, 0xB0 };
	GObjects = reinterpret_cast<FUObjectArray*>(FindPointer(sig, sizeof(sig)));
	std::cout << "GObjects: " << GObjects << std::endl;
	if (GObjects)
		return true;

	return false;
}

float ACharacter::GetTargetFOV(class AAthenaPlayerCharacter* Character) {
	static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.GetTargetFOV");

	UFOVHandlerFunctions_GetTargetFOV_Params params;
	params.Character = Character;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;

	return params.ReturnValue;

}

void ACharacter::SetTargetFOV(class AAthenaPlayerCharacter* Character, float TargetFOV)
{
	static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.SetTargetFOV");

	UFOVHandlerFunctions_SetTargetFOV_Params params;
	params.Character = Character;
	params.TargetFOV = TargetFOV - 13;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}

// Function Engine.Actor.GetAttachParentActor
// (Native, Public, BlueprintCallable, BlueprintPure, Const)
// Parameters:
// class AActor*                  ReturnValue                    (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)

class AActor* AActor::GetAttachParentActor()
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");

	AActor_GetAttachParentActor_Params params;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;

	return params.ReturnValue;
}