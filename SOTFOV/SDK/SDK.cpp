#include "SDK.h"

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

bool UObject::IsA(UClass* cmp) const
{
	for (auto super = Class; super; super = static_cast<UClass*>(super->SuperField))
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
}

bool InitializeNames(MODULEINFO game) {
	GNames = *reinterpret_cast<TNameEntryArray**>(reinterpret_cast<uintptr_t>(game.lpBaseOfDll) + 0x780C150);

	if (GNames) {
		//std::cout << "Got Names: " << GNames << std::endl;

		return true;
	}
	return false;
}

bool InitializeObjects(MODULEINFO game) {
	GObjects = reinterpret_cast<FUObjectArray*>(reinterpret_cast<uintptr_t>(game.lpBaseOfDll) + 0x7810690);


	if (GObjects) {
		//std::cout << "Got Objects: " << GObjects << std::endl;

		return true;
	}
	return false;
}

void APlayerController::FOV(float NewFOV)
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");

	APlayerController_FOV_Params params;
	params.NewFOV = NewFOV;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}

void UCanvas::K2_DrawLine(const struct FVector2D& ScreenPositionA, const struct FVector2D& ScreenPositionB, float Thickness, const struct FLinearColor& RenderColor)
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.Canvas.K2_DrawLine");

	UCanvas_K2_DrawLine_Params params;
	params.ScreenPositionA = ScreenPositionA;
	params.ScreenPositionB = ScreenPositionB;
	params.Thickness = Thickness;
	params.RenderColor = RenderColor;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}
void UCanvas::K2_DrawText(class UFont* RenderFont, const struct FString& RenderText, const struct FVector2D& ScreenPosition, const struct FVector2D& Scale, const struct FLinearColor& RenderColor, float Kerning, const struct FLinearColor& ShadowColor, const struct FVector2D& ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, const struct FLinearColor& OutlineColor)
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.Canvas.K2_DrawText");

	UCanvas_K2_DrawText_Params params;
	params.RenderFont = RenderFont;
	params.RenderText = RenderText;
	params.ScreenPosition = ScreenPosition;
	params.Scale = Scale;
	params.RenderColor = RenderColor;
	params.Kerning = Kerning;
	params.ShadowColor = ShadowColor;
	params.ShadowOffset = ShadowOffset;
	params.bCentreX = bCentreX;
	params.bCentreY = bCentreY;
	params.bOutlined = bOutlined;
	params.OutlineColor = OutlineColor;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}
void UCanvas::K2_DrawBox(const struct FVector2D& ScreenPosition, const struct FVector2D& ScreenSize, float Thickness)
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.Canvas.K2_DrawBox");

	UCanvas_K2_DrawBox_Params params;
	params.ScreenPosition = ScreenPosition;
	params.ScreenSize = ScreenSize;
	params.Thickness = Thickness;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}
void UCanvas::K2_DrawBorder(class UTexture* BorderTexture, class UTexture* BackgroundTexture, class UTexture* LeftBorderTexture, class UTexture* RightBorderTexture, class UTexture* TopBorderTexture, class UTexture* BottomBorderTexture, const struct FVector2D& ScreenPosition, const struct FVector2D& ScreenSize, const struct FVector2D& CoordinatePosition, const struct FVector2D& CoordinateSize, const struct FLinearColor& RenderColor, const struct FVector2D& BorderScale, const struct FVector2D& BackgroundScale, float Rotation, const struct FVector2D& PivotPoint, const struct FVector2D& CornerSize)
{
	static auto fn = UObject::FindObject<UFunction>("Function Engine.Canvas.K2_DrawBorder");

	UCanvas_K2_DrawBorder_Params params;
	params.BorderTexture = BorderTexture;
	params.BackgroundTexture = BackgroundTexture;
	params.LeftBorderTexture = LeftBorderTexture;
	params.RightBorderTexture = RightBorderTexture;
	params.TopBorderTexture = TopBorderTexture;
	params.BottomBorderTexture = BottomBorderTexture;
	params.ScreenPosition = ScreenPosition;
	params.ScreenSize = ScreenSize;
	params.CoordinatePosition = CoordinatePosition;
	params.CoordinateSize = CoordinateSize;
	params.RenderColor = RenderColor;
	params.BorderScale = BorderScale;
	params.BackgroundScale = BackgroundScale;
	params.Rotation = Rotation;
	params.PivotPoint = PivotPoint;
	params.CornerSize = CornerSize;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}

void AProjectileWeapon::ScopeOn()
{
	static auto fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.ScopeOn");

	AProjectileWeapon_ScopeOn_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}

void AProjectileWeapon::ScopeOff()
{
	static auto fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.ScopeOff");

	AProjectileWeapon_ScopeOff_Params params;

	auto flags = fn->FunctionFlags;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
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
	params.TargetFOV = TargetFOV;

	auto flags = fn->FunctionFlags;
	fn->FunctionFlags |= 0x400;

	UObject::ProcessEvent(this, fn, &params);

	fn->FunctionFlags = flags;
}