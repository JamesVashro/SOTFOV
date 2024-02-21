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


bool InitializeNames() 
{
	static BYTE sig[] = { 0x48, 0x8B, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0x00, 0x75, 0x4A };

	GNames = *(TNameEntryArray**)FindPointer(sig, sizeof(sig));
	printf("GNames: %p\n", GNames);
	return GNames != nullptr;
}
bool InitializeObjects() 
{
	GObjects = (TUObjectArray*)((uintptr_t)getModuleInfo(NULL).lpBaseOfDll + 0x806d798);
	return GObjects != nullptr;
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

// Function Engine.PlayerController.FOV
// (Exec, Native, Public)
// Parameters:
// float                          NewFOV                         (Parm, ZeroConstructor, IsPlainOldData)

void APlayerController::FOV(float NewFOV)
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");

	struct
	{
		float NewFOV;
	} params;
	params.NewFOV = NewFOV;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}