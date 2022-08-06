#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <Psapi.h>

#include "FindShit.h"
#include "MinHook.h"
#include "UE4.h"

#include <iostream>

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct { unsigned char MAKE_PAD(offset); type name; }

class FNameEntry
{
public:
	__int32 Index;
	char pad_0x0004[0x4];
	FNameEntry* HashNext;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};

	const char* GetName() const
	{
		return AnsiName;
	}
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
	int32_t Num() const
	{
		return NumElements;
	}

	bool IsValidIndex(int32_t index) const
	{
		return index >= 0 && index < Num() && GetById(index) != nullptr;
	}

	ElementType const* const& GetById(int32_t index) const
	{
		return *GetItemPtr(index);
	}

private:
	ElementType const* const* GetItemPtr(int32_t Index) const
	{
		int32_t ChunkIndex = Index / ElementsPerChunk;
		int32_t WithinChunkIndex = Index % ElementsPerChunk;
		ElementType** Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	enum
	{
		ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
	};

	ElementType** Chunks[ChunkTableSize];
	__int32 NumElements;
	__int32 NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

static TNameEntryArray* GNames = nullptr;

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

// ScriptStruct CoreUObject.Rotator
// Size: 0x0c (Inherited: 0x00)
struct FRotator {
	float Pitch; // 0x00(0x04)
	float Yaw; // 0x04(0x04)
	float Roll; // 0x08(0x04)
};

struct FName
{
	int32_t ComparisonIndex;
	int32_t Number;

	std::string GetName() const {
		auto nameEntry = GNames->GetById(ComparisonIndex);
		if (!nameEntry) return "";
		return nameEntry->AnsiName;
	}

	std::string GetById(size_t id) const
	{
		return GNames->GetById(static_cast<int32_t>(id))->GetName();
	}
};

// ScriptStruct CoreUObject.Transform
// Size: 0x30 (Inherited: 0x00)
struct FTransform {
	struct FQuat Rotation; // 0x00(0x10)
	struct FVector Translation; // 0x10(0x0c)
	char UnknownData_1C[0x4]; // 0x1c(0x04)
	struct FVector Scale3D; // 0x20(0x0c)
	char UnknownData_2C[0x4]; // 0x2c(0x04)
};

struct FUObjectItem
{
	class UObject* Object; //0x0000
	__int32 Flags; //0x0008
	__int32 ClusterIndex; //0x000C
	__int32 SerialNumber; //0x0010
	int padding;
};

class TUObjectArray
{
public:
	FUObjectItem* Objects;
	int32_t MaxElements;
	int32_t NumElements;

	UObject* GetByIndex(int index) { return  Objects[index].Object; }
};

class FUObjectArray
{
public:
	__int32 ObjFirstGCIndex; //0x0000
	__int32 ObjLastNonGCIndex; //0x0004
	__int32 MaxObjectsNotConsideredByGC; //0x0008
	__int32 OpenForDisregardForGC; //0x000C

	TUObjectArray ObjObjects; //0x0010
};

static FUObjectArray* GObjects = nullptr;

class UClass;
// Class CoreUObject.Object
// 0x0028
class UObject
{
public:
	void* Vtable;				// 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int32_t                                            ObjectFlags;         // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int32_t                                            InternalIndex;       // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	class UClass* Class;               // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	struct FName                                       Name;                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	class UObject* Outer;               // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	static inline TUObjectArray& GetGlobalObjects()
	{
		return GObjects->ObjObjects;
	}

	std::string GetName() const;

	std::string GetFullName() const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GetGlobalObjects().NumElements; ++i)
		{
			auto object = GetGlobalObjects().GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}

	template<typename T>
	static T* GetObjectCasted(std::size_t index)
	{
		return static_cast<T*>(GetGlobalObjects().GetByIndex(index));
	}

	bool IsA(std::string cmp) const;

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Object");
		return ptr;
	}

	inline void ProcessEvent(void* obj, class UFunction* function, void* parms)
	{
		auto vtable = *reinterpret_cast<void***>(obj);
		reinterpret_cast<void(*)(void*, class UFunction*, void*)>(vtable[55])(obj, function, parms);
	}
};

// Class Engine.Texture
// 0x0110 (0x0138 - 0x0028)
class UTexture : public UObject {
public:
	unsigned char padding[0x110];
};

// Class Engine.Texture2D
// 0x0058 (0x0190 - 0x0138)
class UTexture2D : public UTexture {
public:
	unsigned char padding[0x58];
};

// Class Engine.Font
// Size: 0x1d8 (Inherited: 0x28)
class UFont : public UObject {
public:
	unsigned char padding[0x1d8];
};

// Class Engine.Canvas
// 0x00C8 (0x00F0 - 0x0028)
class UCanvas : public UObject
{
public:
	float                                              OrgX;                                                     // 0x0028(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              OrgY;                                                     // 0x002C(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              ClipX;                                                    // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              ClipY;                                                    // 0x0034(0x0004) (ZeroConstructor, IsPlainOldData)
	struct FColor                                      DrawColor;                                                // 0x0038(0x0004) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      bCenterX : 1;                                             // 0x003C(0x0001)
	unsigned char                                      bCenterY : 1;                                             // 0x003C(0x0001)
	unsigned char                                      bNoSmooth : 1;                                            // 0x003C(0x0001)
	unsigned char                                      UnknownData00[0x3];                                       // 0x003D(0x0003) MISSED OFFSET
	int                                                SizeX;                                                    // 0x0040(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                                SizeY;                                                    // 0x0044(0x0004) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x8];                                       // 0x0048(0x0008) MISSED OFFSET
	struct FPlane                                      ColorModulate;                                            // 0x0050(0x0010) (ZeroConstructor, IsPlainOldData)
	class UTexture2D* DefaultTexture;                                           // 0x0060(0x0008) (ZeroConstructor, IsPlainOldData)
	class UTexture2D* GradientTexture0;                                         // 0x0068(0x0008) (ZeroConstructor, IsPlainOldData)
	class UReporterGraph* ReporterGraph;                                            // 0x0070(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData02[0x78];                                      // 0x0078(0x0078) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.Canvas");
		return ptr;
	}

	//struct FVector2D K2_TextSize(struct UFont* RenderFont, struct FString RenderText, struct FVector2D Scale); // Function Engine.Canvas.K2_TextSize // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2e99020
	//struct FVector2D K2_StrLen(struct UFont* RenderFont, struct FString RenderText); // Function Engine.Canvas.K2_StrLen // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2e98f30
	//struct FVector K2_Project(struct FVector WorldLocation); // Function Engine.Canvas.K2_Project // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2e98bb0
	//void K2_DrawTriangle(struct UTexture* RenderTexture, struct TArray<struct FCanvasUVTri> Triangles); // Function Engine.Canvas.K2_DrawTriangle // Final|Native|Public|BlueprintCallable // @ game+0x2e97d70
	void K2_DrawText(struct FString RenderText, struct FVector2D ScreenPosition, struct FVector2D Scale, struct FLinearColor RenderColor, float Kerning, struct FLinearColor ShadowColor, struct FVector2D ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, struct FLinearColor OutlineColor);
	void K2_DrawLine(const struct FVector2D& ScreenPositionA, const struct FVector2D& ScreenPositionB, float Thickness, const struct FLinearColor& RenderColor); // Function Engine.Canvas.K2_DrawLine // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2e96cc0
	void K2_DrawBox(const struct FVector2D& ScreenPosition, const struct FVector2D& ScreenSize, float Thickness);
	//void K2_DrawBorder(struct UTexture* BorderTexture, struct UTexture* BackgroundTexture, struct UTexture* LeftBorderTexture, struct UTexture* RightBorderTexture, struct UTexture* TopBorderTexture, struct UTexture* BottomBorderTexture, struct FVector2D ScreenPosition, struct FVector2D ScreenSize, struct FVector2D CoordinatePosition, struct FVector2D CoordinateSize, struct FLinearColor RenderColor, struct FVector2D BorderScale, struct FVector2D BackgroundScale, float Rotation, struct FVector2D PivotPoint, struct FVector2D CornerSize); // Function Engine.Canvas.K2_DrawBorder // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2e96780

};

struct UCanvas_K2_DrawText_Params
{
	class UFont* RenderFont;                                               // (Parm, ZeroConstructor, IsPlainOldData)
	struct FString                                     RenderText;                                               // (Parm, ZeroConstructor)
	struct FVector2D                                   ScreenPosition;                                           // (Parm, ZeroConstructor, IsPlainOldData)
	struct FVector2D                                   Scale;                                                    // (Parm, ZeroConstructor, IsPlainOldData)
	struct FLinearColor                                RenderColor;                                              // (Parm, ZeroConstructor, IsPlainOldData)
	float                                              Kerning;                                                  // (Parm, ZeroConstructor, IsPlainOldData)
	struct FLinearColor                                ShadowColor;                                              // (Parm, ZeroConstructor, IsPlainOldData)
	struct FVector2D                                   ShadowOffset;                                             // (Parm, ZeroConstructor, IsPlainOldData)
	bool                                               bCentreX;                                                 // (Parm, ZeroConstructor, IsPlainOldData)
	bool                                               bCentreY;                                                 // (Parm, ZeroConstructor, IsPlainOldData)
	bool                                               bOutlined;                                                // (Parm, ZeroConstructor, IsPlainOldData)
	struct FLinearColor                                OutlineColor;                                             // (Parm, ZeroConstructor, IsPlainOldData)
};


// Function Engine.Canvas.K2_DrawBox
struct UCanvas_K2_DrawBox_Params
{
	struct FVector2D                                   ScreenPosition;                                           // (Parm, ZeroConstructor, IsPlainOldData)
	struct FVector2D                                   ScreenSize;                                               // (Parm, ZeroConstructor, IsPlainOldData)
	float                                              Thickness;                                                // (Parm, ZeroConstructor, IsPlainOldData)
};

// Function Engine.Canvas.K2_DrawLine
struct UCanvas_K2_DrawLine_Params
{
	struct FVector2D                                   ScreenPositionA;                                          // (Parm, ZeroConstructor, IsPlainOldData)
	struct FVector2D                                   ScreenPositionB;                                          // (Parm, ZeroConstructor, IsPlainOldData)
	float                                              Thickness;                                                // (Parm, ZeroConstructor, IsPlainOldData)
	struct FLinearColor                                RenderColor;                                              // (Parm, ZeroConstructor, IsPlainOldData)
};

// Class CoreUObject.Field
// 0x0008 (0x0030 - 0x0028)
class UField : public UObject
{
public:
	class UField* Next;                                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Field");
		return ptr;
	}

};

// Class CoreUObject.Struct
// 0x0058 (0x0088 - 0x0030)
class UStruct : public UField
{
public:
	class UStruct* SuperField;                                               // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	char                                               pad_0038[8];          // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int32_t                                            PropertySize;         // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int32_t                                            MinAlignment;         // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	char                                               pad_0048[16];         // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	class UField* Children;                                                  // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	char                                               pad_0060[40];         // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Struct");
		return ptr;
	}

};

// Class CoreUObject.Class
// 0x0138 (0x01C0 - 0x0088)
class UClass : public UStruct
{
public:
	unsigned char                                      UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET

	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	}

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Class");
		return ptr;
	}

};


// Class CoreUObject.Function
// 0x0030 (0x00B8 - 0x0088)
class UFunction : public UStruct
{
public:
	int32_t                                            FunctionFlags;                                            // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int16_t                                            RepOffset;                                                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int16_t                                            NumParams;                                                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int16_t                                            ParamSize;                                                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int16_t                                            ReturnValueOffset;                                        // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int16_t                                            RPCId;                                                    // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	int16_t                                            RPCResponseId;                                            // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	char                                               pad_0098[24];                                             // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	void* Func;																									 // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Function");
		return ptr;
	}

};

// Enum Engine.EActorSpawnRestrictions
enum class EActorSpawnRestrictions : uint8_t
{
	EActorSpawnRestrictions__ServerOnly = 0,
	EActorSpawnRestrictions__ClientOnly = 1,
	EActorSpawnRestrictions__ServerAndClient = 2,
	EActorSpawnRestrictions__SpawnRestrictionMax = 3,
	EActorSpawnRestrictions__EActorSpawnRestrictions_MAX = 4
};

// Enum Engine.ENetRole
enum class ENetRole : uint8_t
{
	ROLE_None = 0,
	ROLE_SimulatedProxy = 1,
	ROLE_AutonomousProxy = 2,
	ROLE_Authority = 3,
	ROLE_MAX = 4
};

// ScriptStruct Engine.RepMovement
// 0x0038
struct FRepMovement
{
	unsigned char paddingp[0x38];
};

// ScriptStruct Engine.TickFunction
// 0x0048
struct FTickFunction
{
	unsigned char padding[0x48];
};

// ScriptStruct Engine.ActorTickFunction
// 0x0008 (0x0050 - 0x0048)
struct FActorTickFunction : public FTickFunction
{
	unsigned char padding[0x0008];
};

// ScriptStruct Engine.Vector_NetQuantize100
// 0x0000 (0x000C - 0x000C)
struct FVector_NetQuantize100 : public FVector
{
	float                                              X;                                                        // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	float                                              Y;                                                        // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	float                                              Z;                                                        // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
};

// ScriptStruct Engine.RepAttachment
// 0x0048
struct FRepAttachment
{
	unsigned char padding[0x38];
	class USceneComponent* AttachComponent;                                          // 0x0038(0x0008) (ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData)
	unsigned char                                      UnknownData01[0x8];                                       // 0x0040(0x0008) MISSED OFFSET
};

// Enum Engine.EAutoReceiveInput
enum class EAutoReceiveInput : uint8_t
{
	EAutoReceiveInput__Disabled = 0,
	EAutoReceiveInput__Player0 = 1,
	EAutoReceiveInput__Player1 = 2,
	EAutoReceiveInput__Player2 = 3,
	EAutoReceiveInput__Player3 = 4,
	EAutoReceiveInput__Player4 = 5,
	EAutoReceiveInput__Player5 = 6,
	EAutoReceiveInput__Player6 = 7,
	EAutoReceiveInput__Player7 = 8,
	EAutoReceiveInput__EAutoReceiveInput_MAX = 9
};

// Enum Engine.ESpawnActorCollisionHandlingMethod
enum class ESpawnActorCollisionHandlingMethod : uint8_t
{
	ESpawnActorCollisionHandlingMethod__Undefined = 0,
	ESpawnActorCollisionHandlingMethod__AlwaysSpawn = 1,
	ESpawnActorCollisionHandlingMethod__AdjustIfPossibleButAlwaysSpawn = 2,
	ESpawnActorCollisionHandlingMethod__AdjustIfPossibleButDontSpawnIfColliding = 3,
	ESpawnActorCollisionHandlingMethod__DontSpawnIfColliding = 4,
	ESpawnActorCollisionHandlingMethod__ESpawnActorCollisionHandlingMethod_MAX = 5
};

// ScriptStruct Engine.ActorPtr
// 0x0008
struct FActorPtr
{
	class AActor* Actor;                                                    // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
};

struct FScriptMulticastDelegate {

};

// 
// 
//			968
//
// Class Engine.Actor
// 0x03A0 (0x03c8 - 0x0028)
class AActor : public UObject
{
public:
	char				pad_0028[86]; //0x0028
	int16_t				IsMounted; //0x007E
	char				pad_0080[72]; //0x0080
	struct				FRepAttachment AttachmentReplication; //0x00C8
	char				pad_0110[696]; //0x0110

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.Actor");
		return ptr;
	}

	class AActor* GetAttachParentActor();
};

// Function Engine.Actor.GetDistanceTo
struct AActor_GetDistanceTo_Params
{
	class AActor* OtherActor;                                               // (ConstParm, Parm, ZeroConstructor, IsPlainOldData)
	float                                              ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
};

// Function Engine.Actor.K2_GetActorLocation
struct AActor_K2_GetActorLocation_Params
{
	struct FVector                                     ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
};

// Function Engine.Actor.K2_GetRootComponent
struct AActor_K2_GetRootComponent_Params
{
	class USceneComponent* ReturnValue;                                              // (ExportObject, Parm, OutParm, ZeroConstructor, ReturnParm, InstancedReference, IsPlainOldData)
};

// Function Engine.Actor.GetAttachParentActor
struct AActor_GetAttachParentActor_Params
{
	class AActor* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
};

// Class Engine.Info
// 0x0000 (0x03D0 - 0x03D0)
class AInfo : public AActor
{
public:

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.Info");
		return ptr;
	}
};

// ScriptStruct Engine.UniqueNetIdRepl
// Size: 0x18 (Inherited: 0x00)
struct FUniqueNetIdRepl {
	char UnknownData_0[0x18]; // 0x00(0x18)
};

// Class Engine.PlayerState
// 0x0088 (0x0458 - 0x03D0)
class APlayerState : public AInfo
{
public:
	float Score; // 0x3c8(0x04)
	char Ping; // 0x3cc(0x01)
	char UnknownData_3CD[0x3]; // 0x3cd(0x03)
	struct FString PlayerName; // 0x3d0(0x10)
	char UnknownData_3E0[0x10]; // 0x3e0(0x10)
	int32_t PlayerId; // 0x3f0(0x04)
	char bIsSpectator : 1; // 0x3f4(0x01)
	char bOnlySpectator : 1; // 0x3f4(0x01)
	char bIsABot : 1; // 0x3f4(0x01)
	char UnknownData_3F4_3 : 1; // 0x3f4(0x01)
	char bIsInactive : 1; // 0x3f4(0x01)
	char bFromPreviousLevel : 1; // 0x3f4(0x01)
	char UnknownData_3F4_6 : 2; // 0x3f4(0x01)
	char UnknownData_3F5[0x3]; // 0x3f5(0x03)
	int32_t StartTime; // 0x3f8(0x04)
	char UnknownData_3FC[0x4]; // 0x3fc(0x04)
	class UClass* EngineMessageClass; // 0x400(0x08)
	char UnknownData_408[0x18]; // 0x408(0x18)
	struct FUniqueNetIdRepl UniqueId; // 0x420(0x18)
	char UnknownData_438[0x18]; // 0x438(0x18)
};

// Class Engine.Pawn
// 0x0078 (0x0440 - 0x03C8)
class APawn : public AActor
{
public:
	char pad_03C8[120]; //0x03C8

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.Pawn");
		return ptr;
	}
};




// Class Engine.Controller
// 0x0060 (0x0430 - 0x03D0)
class AController : public AActor
{
public:
	char UnknownData_3C8[0x8]; // 0x3c8(0x08)
	struct APawn* Pawn; // 0x3d0(0x08)
	char UnknownData_3D8[0x8]; // 0x3d8(0x08)
	struct ACharacter* Character; // 0x3e0(0x08)
	struct APlayerState* PlayerState; // 0x3e8(0x08)
	struct USceneComponent* TransformComponent; // 0x3f0(0x08)
	struct FRotator ControlRotation; // 0x3f8(0x0c)
	char bAttachToPawn : 1; // 0x404(0x01)
	char UnknownData_404_1 : 7; // 0x404(0x01)
	char UnknownData_405[0xb]; // 0x405(0x0b)
	struct FName StateName; // 0x410(0x08)
	unsigned char padding[0x10];

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.Controller");
		return ptr;
	}
};

// Class Engine.InterpTrackInst
// 0x0000 (0x0028 - 0x0028)
class UInterpTrackInst : public UObject {
public:

};

// Class Engine.InterpTrackInstDirector
// 0x0008 (0x0030 - 0x0028)
class UInterpTrackInstDirector : public UInterpTrackInst {
public:
	unsigned char padding[0x08];
};

// Class Engine.ScriptViewportClient
// 0x0008 (0x0030 - 0x0028)
class UScriptViewportClient : public UObject
{
public:
	unsigned char                                      UnknownData00[0x8];                                       // 0x0028(0x0008) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.ScriptViewportClient");
		return ptr;
	}

};

// Class Engine.Console
// 0x00C8 (0x00F0 - 0x0028)
class UConsole : public UObject
{
public:
	unsigned char padding[0xc8];
};

// ScriptStruct Engine.DebugDisplayProperty
// 0x0020
struct FDebugDisplayProperty
{
	class UObject* Obj;                                                      // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
	class UClass* WithinClass;                                              // 0x0008(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x10];                                      // 0x0010(0x0010) MISSED OFFSET
};

// Class Engine.GameViewportClient
// 0x0220 (0x0250 - 0x0030)
class UGameViewportClient : public UScriptViewportClient
{
public:
	unsigned char                                      UnknownData00[0x8];                                       // 0x0030(0x0008) MISSED OFFSET
	class UConsole* ViewportConsole;                                          // 0x0038(0x0008) (ZeroConstructor, IsPlainOldData)
	TArray<struct FDebugDisplayProperty>               DebugProperties;                                          // 0x0040(0x0010) (ZeroConstructor)
	unsigned char                                      UnknownData01[0x30];                                      // 0x0050(0x0030) MISSED OFFSET
	class UWorld* World;                                                    // 0x0080(0x0008) (ZeroConstructor, IsPlainOldData)
	class UGameInstance* GameInstance;                                             // 0x0088(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData02[0x1C0];                                     // 0x0090(0x01C0) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.GameViewportClient");
		return ptr;
	}
};

// Class Engine.CheatManager
// 0x0050 (0x0078 - 0x0028)
class UCheatManager : public UObject {
public:
	unsigned char padding[0x50];

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class AthenaCheat.AthenaCheatManager");
		return ptr;
	}
};

// Class Engine.PlayerInput
// 0x0400 (0x0428 - 0x0028)
class UPlayerInput : public UObject {
public:
	unsigned char padding[0x400];
};

// ScriptStruct Engine.ActiveForceFeedbackEffect
// 0x0018
struct FActiveForceFeedbackEffect
{
	unsigned char padding[0x18];
};


// Class Engine.TouchInterface
// 0x0030 (0x0058 - 0x0028)
class UTouchInterface : public UObject {
public:
	unsigned char padding[0x30];
};

// Class Engine.DefaultPawn
// 0x0028 (0x0470 - 0x0448)
class ADefaultPawn : public APawn {
public:
	unsigned char padding[0x28];
};

// Class Engine.SpectatorPawn
// 0x0000 (0x0470 - 0x0470)
class ASpectatorPawn : public ADefaultPawn {
public:

};





// Class Engine.PlayerCameraManager
// 0x1880 (0x1C50 - 0x03D0)
class APlayerCameraManager : public AActor
{
public:
	unsigned char padding[0x1880];

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.PlayerCameraManager");
		return ptr;
	}
};


// Class Engine.PlayerController
// 0x02F8 (0x0728 - 0x0430)
class APlayerController : public AController
{
public:
	class UPlayer* Player; //0x0428
	char pad_0430[8]; //0x0430
	class APawn* AcknowledgedPawn; //0x0438
	char pad_0440[304]; //0x0440
	int8_t isWaiting; //0x0570
	char pad_0571[511]; //0x0571

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.PlayerController");
		return ptr;
	}

	bool ProjectWorldLocationToScreen(const struct FVector& WorldLocation, struct FVector2D* ScreenLocation);
};

// Function Engine.PlayerController.ProjectWorldLocationToScreen
struct APlayerController_ProjectWorldLocationToScreen_Params
{
	struct FVector                                     WorldLocation;                                            // (Parm, ZeroConstructor, IsPlainOldData)
	struct FVector2D                                   ScreenLocation;                                           // (Parm, OutParm, ZeroConstructor, IsPlainOldData)
	bool                                               ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
};

// Class Engine.Player
// 0x0020 (0x0048 - 0x0028)
class UPlayer : public UObject
{
public:
	unsigned char                                      UnknownData00[0x8];                                       // 0x0028(0x0008) MISSED OFFSET
	class APlayerController* PlayerController;										 // 0x0030(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
	int                                                CurrentNetSpeed;                                          // 0x0038(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                                ConfiguredInternetSpeed;                                  // 0x003C(0x0004) (ZeroConstructor, Config, GlobalConfig, IsPlainOldData)
	int                                                ConfiguredLanSpeed;                                       // 0x0040(0x0004) (ZeroConstructor, Config, GlobalConfig, IsPlainOldData)
	unsigned char                                      UnknownData01[0x4];                                       // 0x0044(0x0004) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.Player");
		return ptr;
	}

};

// Class Engine.NetConnection
// 0x34540 (0x34588 - 0x0048)
class UNetConnection : public UPlayer {
	unsigned char padding[0x34540];
};


// Enum Engine.EAspectRatioAxisConstraint
enum class EAspectRatioAxisConstraint : uint8_t
{
	AspectRatio_MaintainYFOV = 0,
	AspectRatio_MaintainXFOV = 1,
	AspectRatio_MajorAxisFOV = 2,
	AspectRatio_MaintainYFOV_UnitIsXFovIn16By9 = 3,
	AspectRatio_MAX = 4
};

// Class Engine.LocalPlayer
// 0x01C8 (0x0210 - 0x0048)
class ULocalPlayer : public UPlayer
{
public:
	unsigned char                                      UnknownData00[0x18];                                      // 0x0048(0x0018) MISSED OFFSET
	class UGameViewportClient* ViewportClient;                                           // 0x0060(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x40];                                      // 0x0068(0x0040) MISSED OFFSET
	TEnumAsByte<EAspectRatioAxisConstraint>            AspectRatioAxisConstraint;                                // 0x00A8(0x0001) (ZeroConstructor, Config, IsPlainOldData)
	unsigned char                                      UnknownData02[0x7];                                       // 0x00A9(0x0007) MISSED OFFSET
	class UClass* PendingLevelPlayerControllerClass;                        // 0x00B0(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      bSentSplitJoin : 1;                                       // 0x00B8(0x0001) (Edit, Transient, EditConst)
	unsigned char                                      UnknownData03[0x3];                                       // 0x00B9(0x0003) MISSED OFFSET
	float                                              MinimumAspectRatio;                                       // 0x00BC(0x0004) (ZeroConstructor, Config, IsPlainOldData)
	unsigned char                                      UnknownData04[0x150];                                     // 0x00C0(0x0150) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.LocalPlayer");
		return ptr;
	}

};

// Class Engine.LevelActorContainer
// Size: 0x38 (Inherited: 0x28)
class ULevelActorContainer : public UObject {
public:
	struct TArray<class AActor*> Actors; // 0x28(0x10)
};

//size 0x300 (768)
class ULevel : public UObject {
public:
	char pad_0028[120];
	struct TArray<class AActor*> Actors;
	class UWorld* OwningWorld; // 0xd0(0x08)
	class UModel* Model; // 0xd8(0x08)
	struct TArray<class UModelComponent*> ModelComponents; // 0xe0(0x10)
	class ALevelScriptActor* LevelScriptActor; // 0xf0(0x08)
	class ANavigationObjectBase* NavListStart; // 0xf8(0x08)
	class ANavigationObjectBase* NavListEnd; // 0x100(0x08)
	struct TArray<class UNavigationDataChunk*> NavDataChunks; // 0x108(0x10)
	float LightmapTotalSize; // 0x118(0x04)
	float ShadowmapTotalSize; // 0x11c(0x04)
	struct TArray<struct FVector> StaticNavigableGeometry; // 0x120(0x10)
	char UnknownData_130[0x130]; // 0x130(0x130)
	bool LevelVisibility; // 0x260(0x01)
	char UnknownData_261[0xf]; // 0x261(0x0f)
	char UnknownData_270_0 : 3; // 0x270(0x01)
	char Blocked : 1; // 0x270(0x01)
	char UnknownData_270_4 : 4; // 0x270(0x01)
	char UnknownData_271[0x67]; // 0x271(0x67)
	struct TArray<class UAssetUserData*> AssetUserData; // 0x2d8(0x10)
	char UnknownData_2E8[0x18]; // 0x2e8(0x18)
};

class UNetDriver {
public:
	unsigned char padding[0x0450];
};

// Class Engine.ActorComponent
// 0x00A0 (0x00C8 - 0x0028)
class UActorComponent : public UObject
{
public:
	unsigned char paddingp[0xA0];
};

// Class Engine.InputComponent
// 0x0088 (0x0150 - 0x00C8)
class UInputComponent : public UActorComponent {
public:
	unsigned char padding[0x88];
};

//size 0x2e0
class USceneComponent : public UActorComponent {
public:
	unsigned char padding[0x2e0];
};

class UPrimitiveComponent : public USceneComponent {
public:
	unsigned char padding[0x02E0];
};

// Class Engine.MeshComponent
// 0x0010 (0x05D0 - 0x05C0)
class UMeshComponent : public UPrimitiveComponent
{
public:
	unsigned char padding[0x0010];
};

class ULineBatchComponent : public UPrimitiveComponent {
public:
	unsigned char padding[0x0030];
};

// ScriptStruct StatusEffects.StatusDescriptor
// Size: 0x04 (Inherited: 0x00)
struct FStatusDescriptor {
	float Intensity; // 0x00(0x04)
};

// ScriptStruct StatusEffects.Status
// Size: 0x18 (Inherited: 0x00)
struct FStatus {
	struct TArray<struct UClass*> Type; // 0x00(0x10)
	struct FStatusDescriptor Descriptor; // 0x10(0x04)
	char UnknownData_14[0x4]; // 0x14(0x04)
};

class AGameNetworkManager : public AInfo {
public:
	unsigned char padding[0x70];
};

// Class Engine.PhysicsCollisionHandler
// 0x0018 (0x0040 - 0x0028)
class UPhysicsCollisionHandler : public UObject {
public:
	unsigned char padding[0x18];
};

// Class Engine.LevelStreaming
// 0x01E8 (0x0210 - 0x0028)
class ULevelStreaming : public UObject {
public:
	unsigned char padding[0x1E8];
};

// Class Engine.ParticleEventManager
// 0x0000 (0x03D0 - 0x03D0)
class AParticleEventManager : public AActor {
public:

};

// Class Engine.Brush
// 0x0038 (0x0408 - 0x03D0)
class ABrush : public AActor {
public:
	unsigned char padding[0x38];
};

// Class Engine.Volume
// 0x0000 (0x0408 - 0x0408)
class AVolume : public ABrush {
public:

};

// Class Engine.PhysicsVolume
// 0x0010 (0x0418 - 0x0408)
class APhysicsVolume : public AVolume {
public:
	unsigned char padding[0x10];
};

// Class Engine.BlueprintFunctionLibrary
// 0x0000 (0x0028 - 0x0028)
class UBlueprintFunctionLibrary : public UObject {
public:

};

// Class Engine.NavigationSystem
// 0x03B0 (0x03D8 - 0x0028)
class UNavigationSystem : public UBlueprintFunctionLibrary {
	unsigned char padding[0x3B0];
};

// Class Engine.GameMode
// 0x00F8 (0x04C8 - 0x03D0)
class AGameMode : public AInfo {
	unsigned char padding[0xF8];
};

// Class Engine.AISystemBase
// 0x0018 (0x0040 - 0x0028)
class UAISystemBase : public UObject {
public:
	unsigned char padding[0x18];
};

// Class Engine.AvoidanceManager
// 0x00A0 (0x00C8 - 0x0028)
class UAvoidanceManager : public UObject {
public:
	unsigned char padding[0xA0];
};

// Class Engine.MaterialParameterCollectionInstance
// 0x00C8 (0x00F0 - 0x0028)
class UMaterialParameterCollectionInstance : public UObject {
public:
	unsigned char padding[0xC8];
};

// Class Engine.WorldComposition
// 0x0090 (0x00B8 - 0x0028)
class UWorldComposition : public UObject {
public:
	unsigned char padding[0x90];
};



// Class Engine.Character
// 0x01A0 (0x05E0 - 0x0448)
class ACharacter : public APawn {
public:
	char pad_0440[416]; //0x0440

	float GetTargetFOV(class AAthenaPlayerCharacter* Character);
	void SetTargetFOV(class AAthenaPlayerCharacter* Character, float TargetFOV);
};

// ScriptStruct Athena.ServerWieldSet
// 0x001C
struct FServerWieldSet
{
	unsigned char padding[0x1C];
};

// Class Athena.WieldableItemSetComponent
// 0x0028 (0x00F0 - 0x00C8)
class UWieldableItemSetComponent : public UActorComponent
{
public:
	unsigned char padding[0x0028];
};

// Class Engine.DataAsset
// 0x0000 (0x0028 - 0x0028)
class UDataAsset : public UObject
{
public:

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.DataAsset");
		return ptr;
	}

};

// Class Animation.AnimationDataStoreAsset
// 0x0010 (0x0038 - 0x0028)
class UAnimationDataStoreAsset : public UDataAsset
{
public:
	unsigned char paddingp[0x10];
};

// Class Athena.LookAtOffsetComponent
// 0x00B8 (0x0180 - 0x00C8)
class ULookAtOffsetComponent : public UActorComponent
{
public:
	unsigned char padding[0x00B8];
};

// Class Athena.AthenaAnimationSittingDataAsset
// 0x0018 (0x0040 - 0x0028)
class UAthenaAnimationSittingDataAsset : public UDataAsset
{
public:
	unsigned char paddingp[0x18];
};

// Class Engine.MovementComponent
// 0x0048 (0x0110 - 0x00C8)
class UMovementComponent : public UActorComponent
{
public:
	unsigned char paddingp[0x0048];
};

// Class Engine.NavMovementComponent
// 0x0048 (0x0158 - 0x0110)
class UNavMovementComponent : public UMovementComponent
{
public:
	unsigned char paddingp[0x0048];
};

// Class Engine.PawnMovementComponent
// 0x0008 (0x0160 - 0x0158)
class UPawnMovementComponent : public UNavMovementComponent
{
public:
	unsigned char paddingp[0x0008];
};

// Class Engine.CharacterMovementComponent
// 0x0400 (0x0560 - 0x0160)
class UCharacterMovementComponent : public UPawnMovementComponent
{
public:
	unsigned char padding[0x400];
};

// Class Athena.AthenaCharacterMovementComponent
// 0x02A0 (0x0800 - 0x0560)
class UAthenaCharacterMovementComponent : public UCharacterMovementComponent
{
public:
	unsigned char padding[0x2A0];
};

// ScriptStruct Animation.Docker
// 0x0050
struct FDocker
{
	unsigned char                                      UnknownData00[0x50];                                      // 0x0000(0x0050) MISSED OFFSET
};

// Class ActionStateMachine.ActionStateMachineComponent
// 0x0790 (0x0858 - 0x00C8)
class UActionStateMachineComponent : public UActorComponent
{
public:
	unsigned char paddingp[0x0790];
};

// Class ActionStateMachine.ActionStatePriorityTableData
// 0x0020 (0x0048 - 0x0028)
class UActionStatePriorityTableData : public UDataAsset
{
public:
	unsigned char paddingp[0x0020];
};



// Class Athena.WieldableItem
// 0x03B8 (0x0780 - 0x03C8)
class AWieldableItem : public AActor
{
public:
	unsigned char paddingp[0x3B0];
};

// Class Athena.SkeletalMeshWieldableItem
// 0x0000 (0x0780 - 0x0780)
class ASkeletalMeshWieldableItem : public AWieldableItem
{
public:

};


// ScriptStruct Athena.ProjectileShotParams
// 0x001C
struct FProjectileShotParams
{
	unsigned char paddingp[0x1C];
};

// ScriptStruct Athena.WeaponProjectileParams
// 0x00B0
struct FWeaponProjectileParams
{
	unsigned char padding[0xB0];
};

// Enum Athena.EAimSensitivitySettingCategory
enum class EAimSensitivitySettingCategory : uint8_t
{
	EAimSensitivitySettingCategory__Invalid = 0,
	EAimSensitivitySettingCategory__Blunderbuss = 1,
	EAimSensitivitySettingCategory__Flintlock = 2,
	EAimSensitivitySettingCategory__EyeOfReach = 3,
	EAimSensitivitySettingCategory__Spyglass = 4,
	EAimSensitivitySettingCategory__TridentOfDarkTides = 5,
	EAimSensitivitySettingCategory__EAimSensitivitySettingCategory_MAX = 6
};

// Class WwiseAudio.WwiseEvent
// 0x0028 (0x0050 - 0x0028)
class UWwiseEvent : public UObject
{
public:
	unsigned char padding[0x28];
};

// ScriptStruct Athena.AudioEmitterParameters
// 0x0010
struct FAudioEmitterParameters
{
	unsigned char padding[0x10];
};

// ScriptStruct Athena.KnockBackInfo
// 0x0050
struct FKnockBackInfo
{
	unsigned char padding[0x50];
};

// ScriptStruct Athena.ProjectileWeaponParameters
// 0x01E8
struct FProjectileWeaponParameters
{
	int                                                AmmoClipSize;                                             // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	int                                                AmmoCostPerShot;                                          // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              EquipDuration;                                            // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              IntoAimingDuration;                                       // 0x000C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              RecoilDuration;                                           // 0x0010(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              ReloadDuration;                                           // 0x0014(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FProjectileShotParams                       HipFireProjectileShotParams;                              // 0x0018(0x001C) (Edit, BlueprintVisible)
	struct FProjectileShotParams                       AimDownSightsProjectileShotParams;                        // 0x0034(0x001C) (Edit, BlueprintVisible)
	int                                                InaccuracySeed;                                           // 0x0050(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              ProjectileDistributionMaxAngle;                           // 0x0054(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	int                                                NumberOfProjectiles;                                      // 0x0058(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              ProjectileMaximumRange;                                   // 0x005C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              ProjectileHitScanMaximumRange;                            // 0x0060(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              ProjectileDamage;                                         // 0x0064(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              ProjectileDamageMultiplierAtMaximumRange;                 // 0x0068(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x4];                                       // 0x006C(0x0004) MISSED OFFSET
	class UClass* DamagerType;                                              // 0x0070(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	class UClass* ProjectileId;                                             // 0x0078(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FWeaponProjectileParams                     AmmoParams;                                               // 0x0080(0x00B0) (Edit, BlueprintVisible)
	bool                                               UsesScope;                                                // 0x0130(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x3];                                       // 0x0131(0x0003) MISSED OFFSET
	float                                              ZoomedRecoilDurationIncrease;                             // 0x0134(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              SecondsUntilZoomStarts;                                   // 0x0138(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              SecondsUntilPostStarts;                                   // 0x013C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              WeaponFiredAINoiseRange;                                  // 0x0140(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              MaximumRequestPositionDelta;                              // 0x0144(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              MaximumRequestAngleDelta;                                 // 0x0148(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              TimeoutTolerance;                                         // 0x014C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              AimingMoveSpeedScalar;                                    // 0x0150(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	TEnumAsByte<EAimSensitivitySettingCategory>        AimSensitivitySettingCategory;                            // 0x0154(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData02[0x3];                                       // 0x0155(0x0003) MISSED OFFSET
	float                                              InAimFOV;                                                 // 0x0158(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	float                                              BlendSpeed;                                               // 0x015C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	class UWwiseEvent* DryFireSfx;                                               // 0x0160(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FAudioEmitterParameters                     AudioEmitterParameters;                                   // 0x0168(0x0010) (Edit, BlueprintVisible)
	struct FName                                       RumbleTag;                                                // 0x0178(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	bool                                               KnockbackEnabled;                                         // 0x0180(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData03[0x3];                                       // 0x0181(0x0003) MISSED OFFSET
	struct FKnockBackInfo                              KnockbackParams;                                          // 0x0184(0x0050) (Edit, BlueprintVisible)
	bool                                               StunEnabled;                                              // 0x01D4(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData04[0x3];                                       // 0x01D5(0x0003) MISSED OFFSET
	float                                              StunDuration;                                             // 0x01D8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
	struct FVector                                     TargetingOffset;                                          // 0x01DC(0x000C) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData)
};

// Enum Athena.EProjectileWeaponState
enum class EProjectileWeaponState : uint8_t
{
	EProjectileWeaponState__Equipping = 0,
	EProjectileWeaponState__Idle = 1,
	EProjectileWeaponState__Aiming = 2,
	EProjectileWeaponState__Recoil = 3,
	EProjectileWeaponState__Reloading = 4,
	EProjectileWeaponState__InterruptedReload = 5,
	EProjectileWeaponState__EProjectileWeaponState_MAX = 6
};

// Class Athena.AimSensitivityComponent
// 0x0040 (0x0108 - 0x00C8)
class UAimSensitivityComponent : public UActorComponent
{
public:
	unsigned char padding[0x40];
};

// Class Athena.InventoryItemComponent
// 0x0048 (0x0110 - 0x00C8)
class UInventoryItemComponent : public UActorComponent
{
public:
	unsigned char padding[0x48];
};

// Enum Athena.EProjectileWeaponType
enum class EProjectileWeaponType : uint8_t
{
	EProjectileWeaponType__Pistol = 0,
	EProjectileWeaponType__Blunderbuss = 1,
	EProjectileWeaponType__EyeOfReach = 2,
	EProjectileWeaponType__MAX = 3,
	EProjectileWeaponType__EProjectileWeaponType_MAX = 4
};

// Class Athena.ProjectileWeapon
// 0x02D0 (0x0A50 - 0x0780)
class AProjectileWeapon : public ASkeletalMeshWieldableItem
{
public:
	unsigned char                                      UnknownData00[0x30];                                      // 0x0780(0x0030) MISSED OFFSET
	class UInventoryItemComponent* InventoryItem;                                            // 0x07B0(0x0008) (Edit, BlueprintVisible, ExportObject, ZeroConstructor, EditConst, InstancedReference, IsPlainOldData)
	unsigned char									   padding[0x14];
	struct FProjectileWeaponParameters                 WeaponParameters;                                         // 0x07C8(0x01E8) (Edit, BlueprintVisible)
	TEnumAsByte<EProjectileWeaponState>                State;                                                    // 0x09B0(0x0001) (Net, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x3];                                       // 0x09B1(0x0003) MISSED OFFSET
	int                                                AmmoLeft;                                                 // 0x09B4(0x0004) (Net, ZeroConstructor, IsPlainOldData)
	class UAimSensitivityComponent* AimSensitivityComponent;                                  // 0x09B8(0x0008) (Edit, ExportObject, ZeroConstructor, EditConst, InstancedReference, IsPlainOldData)
	unsigned char                                      UnknownData02[0x10];                                      // 0x09C0(0x0010) MISSED OFFSET
	TEnumAsByte<EProjectileWeaponType>                 ProjectileWeaponType;                                     // 0x09D0(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	unsigned char                                      UnknownData03[0x3];                                       // 0x09D1(0x0003) MISSED OFFSET
	float                                              MaxAcceptableAttackTimeStampDelta;                        // 0x09D4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData04[0x4];                                       // 0x09D8(0x0004) MISSED OFFSET
	float                                              DistanceBeforeGravity;                                    // 0x09DC(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              NameplateVisibilityRangeExtensionFactorWhileAiming;       // 0x09E0(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	unsigned char                                      UnknownData05[0x35];                                      // 0x09E4(0x0035) MISSED OFFSET
	bool                                               ShouldShowAmmoUI;                                         // 0x0A19(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	unsigned char                                      UnknownData06[0x36];                                      // 0x0A1A(0x0036) MISSED OFFSET
};

// BlueprintGeneratedClass BP_FlintlockPistol.BP_FlintlockPistol_C
// 0x0038 (0x0A88 - 0x0A50)
class ABP_FlintlockPistol_C : public AProjectileWeapon
{
public:
	unsigned char paddingp[0x38];
};

// Class Athena.WieldedItemComponent
// 0x0130 (0x0410 - 0x02E0)
class UWieldedItemComponent
{
public:
	char pad_0000[752]; //0x0000
	class AActor* CurrentlyWieldedItem; //0x02F0
	char pad_02F8[288]; //0x02F8

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Athena.WieldedItemComponent");
		return ptr;
	}
};

// Class Engine.PostProcessComponent
// 0x0560 (0x0840 - 0x02E0)
class UPostProcessComponent : public USceneComponent
{
public:
	unsigned char padding[0x0560];
};

// Class Engine.CurveBase
// 0x0008 (0x0030 - 0x0028)
class UCurveBase : public UObject
{
public:
	unsigned char                                      UnknownData00[0x8];                                       // 0x0028(0x0008) MISSED OFFSET
};

// Class Engine.CurveFloat
// 0x0080 (0x00B0 - 0x0030)
class UCurveFloat : public UCurveBase
{
public:
	unsigned char padding[0x0080];
};

// Class Engine.MaterialInterface
// 0x0030 (0x0058 - 0x0028)
class UMaterialInterface : public UObject
{
public:
	unsigned char padding[0x0030];
};

// Class Engine.MaterialInstance
// 0x0160 (0x01B8 - 0x0058)
class UMaterialInstance : public UMaterialInterface
{
public:
	unsigned char padding[0x0160];
};

// Class Engine.MaterialInstanceDynamic
// 0x0000 (0x01B8 - 0x01B8)
class UMaterialInstanceDynamic : public UMaterialInstance
{
public:

};

// Class Engine.StaticMeshComponent
// 0x0050 (0x0620 - 0x05D0)
class UStaticMeshComponent : public UMeshComponent
{
public:
	unsigned char padding[0x0050];
};

// Class Engine.Material
// 0x09C0 (0x0A18 - 0x0058)
class UMaterial : public UMaterialInterface
{
public:
	unsigned char padding[0x09C0];
};

//2152
// Class Athena.Spyglass
// 0x00F0 (0x0870 - 0x0780)
class ASpyglass
{
public:
	char pad_0000[1988]; //0x0000
	float InAimFOV; //0x07C4
	char pad_07C8[96]; //0x07C8
	bool IsAiming; //0x0828
	char pad_0829[71]; //0x0829
};

// ScriptStruct ObjectMessaging.ObjectMessagingDispatcher
// 0x00A0
struct FObjectMessagingDispatcher
{
	unsigned char                                      UnknownData00[0xA0];                                      // 0x0000(0x00A0) MISSED OFFSET
};


// Class Athena.AthenaCharacter
// 0x05D0 (0x0BB0 - 0x05E0)
class AAthenaCharacter : public ACharacter
{
public:
	char pad_05E0[592]; //0x05E0
	class UWieldedItemComponent* WieldedItemComponent; //0x0830
	char pad_0838[888]; //0x0838

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Athena.AthenaCharacter");
		return ptr;
	}
};


// Class Engine.SkinnedMeshComponent
// 0x0120 (0x06F0 - 0x05D0)
class USkinnedMeshComponent
{
public:
	char pad_0000[1480]; //0x0000
	class USkeletalMesh* SkeletalMesh; //0x05C8
	char pad_05D0[288]; //0x05D0
};

// Class Engine.SkeletalMeshComponent
// 0x02D0 (0x09C0 - 0x06F0)
class USkeletalMeshComponent : public USkinnedMeshComponent
{
public:
	unsigned char padding[0x2D0];
};

// Class Athena.InteractableBase
// 0x0028 (0x03F0 - 0x03C8)
class AInteractableBase : public AActor
{
public:
	unsigned char padding[0x0028];
};

// Class Athena.ControllableObject
// 0x0110 (0x0500 - 0x03F0)
class AControllableObject : public AInteractableBase
{
public:
	unsigned char padding[0x0110];
};

// Class Athena.StaticMeshMemoryConstraintComponent
// 0x0070 (0x0690 - 0x0620)
class UStaticMeshMemoryConstraintComponent : public UStaticMeshComponent
{
public:
	unsigned char padding[0x70];
};

// Class Athena.LoadableComponent
// 0x0128 (0x01F0 - 0x00C8)
class ULoadableComponent : public UActorComponent
{
public:
	unsigned char padding[0x0128];
};

// Class Athena.Cannon
// 0x0790 (0x0C90 - 0x0500)
class ACannon
{
public:
	char pad_0000[1664]; //0x0000
	float DefaultFOV; //0x0680
	float AimFOV; //0x0684
	char pad_0688[64]; //0x0688

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Athena.Cannon");
		return ptr;
	}
};

// BlueprintGeneratedClass BP_Cannon.BP_Cannon_C
// 0x0018 (0x0CA8 - 0x0C90)
class ABP_Cannon_C : public ACannon
{
public:
	unsigned char padding[0x0018];

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("BlueprintGeneratedClass BP_Cannon.BP_Cannon_C");
		return ptr;
	}

};


// Class Engine.CameraComponent
// 0x0570 (0x0850 - 0x02E0)
class UCameraComponent : public USceneComponent
{
public:
	unsigned char padding[0x570];
};

// Class Athena.AthenaCameraComponent
// 0x0080 (0x08D0 - 0x0850)
class UAthenaCameraComponent : public UCameraComponent
{
public:
	unsigned char padding[0x0080];
};

// Class Athena.LookAtEmoteCameraComponent
// 0x0048 (0x0110 - 0x00C8)
class ULookAtEmoteCameraComponent : public UActorComponent
{
public:
	unsigned char padding[0x0048];
};

// Class Engine.SpringArmComponent
// 0x0090 (0x0370 - 0x02E0)
class USpringArmComponent : public USceneComponent
{
public:
	unsigned char padding[0x0090];
};

// Class Athena.AthenaSpringArmComponent
// 0x0090 (0x0400 - 0x0370)
class UAthenaSpringArmComponent : public USpringArmComponent
{
public:
	unsigned char padding[0x0090];
};

// Class Engine.ShapeComponent
// 0x0020 (0x05E0 - 0x05C0)
class UShapeComponent : public UPrimitiveComponent
{
public:
	unsigned char pdding[0x0020];
};

// Class Engine.BoxComponent
// 0x0010 (0x05F0 - 0x05E0)
class UBoxComponent : public UShapeComponent
{
public:
	unsigned char padding[0x0010];
};

// Class Interaction.CharacterInteractionComponent
// 0x00C0 (0x06B0 - 0x05F0)
class UCharacterInteractionComponent : public UBoxComponent
{
public:
	unsigned char padding[0x00C0];
};

// Class Athena.IntentComponent
// 0x00C0 (0x0188 - 0x00C8)
class UIntentComponent : public UActorComponent
{
public:
	unsigned char padding[0x00C0];
};

// Class Athena.ClimbingComponent
// 0x0018 (0x01A0 - 0x0188)
class UClimbingComponent : public UIntentComponent
{
public:
	unsigned char padding[0x0018];
};

// Class Athena.DrunkennessComponent
// 0x0190 (0x0258 - 0x00C8)
class UDrunkennessComponent : public UActorComponent
{
public:
	unsigned char padding[0x0190];
};

// Class Athena.StaggerComponent
// 0x0148 (0x0210 - 0x00C8)
class UStaggerComponent : public UActorComponent
{
public:
	unsigned char padding[0x0148];
};

// Class Athena.DrowningComponent
// 0x00F8 (0x01C0 - 0x00C8)
class UDrowningComponent : public UActorComponent
{
public:
	unsigned char padding[0x00F8];
};

// Class Athena.RumbleHandlerComponent
// 0x0050 (0x0118 - 0x00C8)
class URumbleHandlerComponent : public UActorComponent
{
public:
	unsigned char padding[0x0050];
};

// Class Athena.ScreenSpaceVfxComponent
// 0x04D8 (0x05A0 - 0x00C8)
class UScreenSpaceVfxComponent : public UActorComponent
{
public:
	unsigned char padding[0x04D8];
};

// Class Athena.TreasureMapCollectionComponent
// 0x00F0 (0x01B8 - 0x00C8)
class UTreasureMapCollectionComponent : public UActorComponent
{
public:
	unsigned char padding[0x00F0];
};

// Class Athena.PhrasesRadialComponent
// 0x0068 (0x0130 - 0x00C8)
class UPhrasesRadialComponent : public UActorComponent
{
public:
	unsigned char padding[0x0068];
};

// Class Athena.QuickInventoryComponent
// 0x0068 (0x0130 - 0x00C8)
class UQuickInventoryComponent : public UActorComponent
{
public:
	unsigned char padding[0x0068];
};

// Class Athena.PointOfInterestTrackerComponent
// 0x00F8 (0x01C0 - 0x00C8)
class UPointOfInterestTrackerComponent : public UActorComponent
{
public:
	unsigned char padding[0x00F8];
};

// Class Athena.LimpingComponent
// 0x00B8 (0x0180 - 0x00C8)
class ULimpingComponent : public UActorComponent
{
public:
	unsigned char padding[0x00B8];
};

// Class Athena.PacingComponent
// 0x0128 (0x01F0 - 0x00C8)
class UPacingComponent : public UActorComponent
{
public:
	unsigned char padding[0x0128];
};

// Class Athena.BurpComponent
// 0x0068 (0x0130 - 0x00C8)
class UBurpComponent : public UActorComponent
{
public:
	unsigned char padding[0x0068];
};

// Class Athena.MeleeAttackLockOnComponent
// 0x0190 (0x0258 - 0x00C8)
class UMeleeAttackLockOnComponent : public UActorComponent
{
public:
	unsigned char padding[0x0190];
};

// Class Interaction.InteractableComponent
// 0x0050 (0x0118 - 0x00C8)
class UInteractableComponent : public UActorComponent
{
public:
	unsigned char padding[0x0050];
};

// Class Reviving.DeathSelfInteractionComponent
// 0x0060 (0x0178 - 0x0118)
class UDeathSelfInteractionComponent : public UInteractableComponent
{
public:
	unsigned char padding[0x0060];
};

// Class Athena.DoubleIntentComponent
// 0x0018 (0x00E0 - 0x00C8)
class UDoubleIntentComponent : public UActorComponent
{
public:
	unsigned char padding[0x0018];
};

// Class WaterSlide.WaterSlidePlayerComponent
// 0x0038 (0x0118 - 0x00E0)
class UWaterSlidePlayerComponent : public UDoubleIntentComponent
{
public:
	unsigned char padding[0x0038];
};

// Class Reviving.ReviveDebugComponent
// 0x0098 (0x0160 - 0x00C8)
class UReviveDebugComponent : public UActorComponent
{
public:
	unsigned char padding[0x0098];
};

// Class Athena.FlashbangEffectComponent
// 0x0048 (0x0110 - 0x00C8)
class UFlashbangEffectComponent : public UActorComponent
{
public:
	unsigned char padding[0x0048];
};

// Class WorldLocationPopUpFramework.WorldLocationVisitorComponent
// 0x0040 (0x0108 - 0x00C8)
class UWorldLocationVisitorComponent : public UActorComponent
{
public:
	unsigned char padding[0x0040];
};

// Class Athena.MeleeAttackLockOnComponentSettings
// 0x0018 (0x0040 - 0x0028)
class UMeleeAttackLockOnComponentSettings : public UDataAsset
{
public:
	unsigned char padding[0x0018];
};

// Class StatusEffects.StatusEffectManagerComponent
// 0x0040 (0x0108 - 0x00C8)
class UStatusEffectManagerComponent : public UActorComponent
{
public:
	unsigned char padding[0x0040];
};

// Class StatusEffects.StatusEffectPersistenceComponent
// 0x0008 (0x00D0 - 0x00C8)
class UStatusEffectPersistenceComponent : public UActorComponent
{
public:
	unsigned char padding[0x0008];
};

// Class Athena.PlayerCurseComponent
// 0x00C0 (0x0188 - 0x00C8)
class UPlayerCurseComponent : public UActorComponent
{
public:
	unsigned char padding[0x00C0];
};

// Class StatusEffects.MaterialStatusSusceptibilityComponent
// 0x0030 (0x00F8 - 0x00C8)
class UMaterialStatusSusceptibilityComponent : public UActorComponent
{
public:
	unsigned char padding[0x0030];
};

// Class Athena.HealthRegenerationPoolComponent
// 0x0060 (0x0128 - 0x00C8)
class UHealthRegenerationPoolComponent : public UActorComponent
{
public:
	unsigned char padding[0x0060];
};

// Class Athena.SicknessComponent
// 0x0190 (0x0258 - 0x00C8)
class USicknessComponent : public UActorComponent
{
public:
	unsigned char padding[0x0190];
};

// Class Athena.RewindComponent
// 0x00F8 (0x01C0 - 0x00C8)
class URewindComponent : public UActorComponent
{
public:
	unsigned char padding[0x00F8];
};

// Class Engine.CapsuleComponent
// 0x0000 (0x05E0 - 0x05E0)
class UCapsuleComponent : public UShapeComponent
{
public:

};

// Class MerchantContracts.PlayerMerchantContractsComponent
// 0x0018 (0x00E0 - 0x00C8)
class UPlayerMerchantContractsComponent : public UActorComponent
{
public:
	unsigned char padding[0x0018];
};

// Class Athena.ShroudBreakerTrackerComponent
// 0x0010 (0x00D8 - 0x00C8)
class UShroudBreakerTrackerComponent : public UActorComponent
{
public:
	unsigned char padding[0x0010];
};

// Class Athena.PlayerShroudBreakerTrackerComponent
// 0x0000 (0x00D8 - 0x00D8)
class UPlayerShroudBreakerTrackerComponent : public UShroudBreakerTrackerComponent
{
public:

};

// Class Pets.PetOwnerComponent
// 0x0020 (0x00E8 - 0x00C8)
class UPetOwnerComponent : public UActorComponent
{
public:
	unsigned char padding[0x0020];
};

// Class AthenaAI.AISpawnContextContainerComponent
// 0x0018 (0x00E0 - 0x00C8)
class UAISpawnContextContainerComponent : public UActorComponent
{
public:
	unsigned char padding[0x0018];
};

// Class Athena.RadialManagerComponent
// 0x01F0 (0x02B8 - 0x00C8)
class URadialManagerComponent : public UActorComponent
{
public:
	unsigned char padding[0x01F0];
};

// Class Engine.CurveVector
// 0x0168 (0x0198 - 0x0030)
class UCurveVector : public UCurveBase
{
public:
	unsigned char paddingp[0x168];
};

// ScriptStruct Athena.WalkConditionsParams
// 0x0008
struct FWalkConditionsParams
{
	float                                              WalkSpeedMultiplier;                                      // 0x0000(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	float                                              WalkStopThreshold;                                        // 0x0004(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
};

// ScriptStruct Athena.SprintConditionsParams
// 0x000C
struct FSprintConditionsParams
{
	float                                              ForwardInputVectorTolerance;                              // 0x0000(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	float                                              MinVelocityForwardDot;                                    // 0x0004(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
	float                                              MinVelocityMagnitude;                                     // 0x0008(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData)
};

// Class Athena.AthenaSpringArmComponentParams
// 0x0068 (0x0090 - 0x0028)
class UAthenaSpringArmComponentParams : public UDataAsset
{
public:
	unsigned char padding[0x0068];
};

// Class Athena.EmotesRadialDataAsset
// 0x0010 (0x0038 - 0x0028)
class UEmotesRadialDataAsset : public UDataAsset
{
public:
	unsigned char padding[0x0010];
};

// ScriptStruct Athena.AchievementKillOpponentFromBehindWithBlunderbuss
// 0x0004
struct FAchievementKillOpponentFromBehindWithBlunderbuss
{
	float                                              FacingDirAngleTolerance;                                  // 0x0000(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
};

// ScriptStruct Athena.AchievementHeavyAttackRivalOffTheirShip
// 0x0010
struct FAchievementHeavyAttackRivalOffTheirShip
{
	class AAthenaCharacter* Character;                                                // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
	float                                              TimeWindowInSeconds;                                      // 0x0008(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	unsigned char                                      UnknownData00[0x4];                                       // 0x000C(0x0004) MISSED OFFSET
};

// ScriptStruct Athena.AchievementBlockThenHitAndGetBlockedThenTakeHit
// 0x0010
struct FAchievementBlockThenHitAndGetBlockedThenTakeHit
{
	class AAthenaPlayerCharacter* Character;                                                // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
	float                                              TimeWindowInSeconds;                                      // 0x0008(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	unsigned char                                      UnknownData00[0x4];                                       // 0x000C(0x0004) MISSED OFFSET
};

// ScriptStruct Athena.AchievementHaveADrinkWithAnotherCrewAtAnOutpost
// 0x0020
struct FAchievementHaveADrinkWithAnotherCrewAtAnOutpost
{
	class AAthenaPlayerCharacter* Character;                                                // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x10];                                      // 0x0008(0x0010) MISSED OFFSET
	float                                              RangeOfCheckInMetres;                                     // 0x0018(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
	float                                              DrinkConsumedVolumeThreshold;                             // 0x001C(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData)
};

// ScriptStruct Athena.AchievementKillOpponentInContest
// 0x0008
struct FAchievementKillOpponentInContest
{
	unsigned char                                      UnknownData00[0x8];                                       // 0x0000(0x0008) MISSED OFFSET
};

// Class AthenaAI.AICreatureMovementModifierParamsDataAsset
// 0x0010 (0x0038 - 0x0028)
class UAICreatureMovementModifierParamsDataAsset : public UDataAsset
{
public:
	unsigned char padding[0x0010];
};

// Class Athena.AsyncLoadoutRepairComponent
// 0x0058 (0x0120 - 0x00C8)
class UAsyncLoadoutRepairComponent : public UActorComponent
{
public:
	unsigned char padding[0x0058];
};

// Class SeasonProgressionUI.SeasonProgressionUIComponent
// 0x00E8 (0x01B0 - 0x00C8)
class USeasonProgressionUIComponent : public UActorComponent
{
public:
	unsigned char padding[0x00E8];
};

// Class Athena.AimAssistComponent
// 0x0088 (0x0150 - 0x00C8)
class UAimAssistComponent : public UActorComponent
{
public:
	unsigned char padding[0x0088];
};

// Class Athena.SequencerCameraDockingComponent
// 0x0010 (0x00D8 - 0x00C8)
class USequencerCameraDockingComponent : public UActorComponent
{
public:
	unsigned char padding[0x0010];
};

// Class EmotingFramework.EmotePropData
// 0x0000 (0x0028 - 0x0028)
class UEmotePropData : public UDataAsset
{
public:
};

// Class Animation.AnimationDataStoreWeakReferenceAsset
// 0x0010 (0x0038 - 0x0028)
class UAnimationDataStoreWeakReferenceAsset : public UDataAsset
{
public:
	unsigned char padding[0x0010];
};

// Enum Athena.ECharacterType
enum class ECharacterType : uint8_t
{
	ECharacterType__Ghost = 0,
	ECharacterType__Character = 1,
	ECharacterType__ECharacterType_MAX = 2
};

// Class Athena.ClothingLoadoutComponent
// 0x01A0 (0x0268 - 0x00C8)
class UClothingLoadoutComponent : public UActorComponent
{
public:
	unsigned char padding[0x01A0];
};

// Class Athena.PetLoadoutComponent
// 0x0098 (0x0160 - 0x00C8)
class UPetLoadoutComponent : public UActorComponent
{
public:
	unsigned char padding[0x0098];
};

// Class Athena.VoyageParticipantComponent
// 0x02B0 (0x0378 - 0x00C8)
class UVoyageParticipantComponent : public UActorComponent
{
public:
	unsigned char padding[0x02B0];
};

// Class Athena.PlayerWorldLocationComponent
// 0x0008 (0x00D0 - 0x00C8)
class UPlayerWorldLocationComponent : public UActorComponent
{
public:
	unsigned char padding[0x0008];
};

// Class Athena.ChainLightningReceiverComponent
// 0x0080 (0x0148 - 0x00C8)
class UChainLightningReceiverComponent : public UActorComponent
{
public:
	unsigned char padding[0x0080];
};

// ScriptStruct Athena.AthenaPlayerCharacterModelSwap
// 0x0028
struct FAthenaPlayerCharacterModelSwap
{
	unsigned char padding[0x0028];
};

// Class Athena.AthenaPlayerCharacter
// 0x0F10 (0x1AB0 - 0x0BA0)
class AAthenaPlayerCharacter : public AAthenaCharacter {
public:
	char pad_0BB0[624]; //0x0BB0
	float CameraFOVWhenSprinting; //0x0E20
	char pad_0E24[3232]; //0x0E24
	bool ReplicatedIsSprinting; //0x1AC4
	char pad_1AC5[43]; //0x1AC5
};

// ScriptStruct Engine.PointerToUberGraphFrame
// Size: 0x08 (Inherited: 0x00)
struct FPointerToUberGraphFrame {
	char UnknownData_0[0x8]; // 0x00(0x08)
};

// ScriptStruct CoreUObject.BoxSphereBounds
// Size: 0x1c (Inherited: 0x00)
struct FBoxSphereBounds
{
	struct FVector Origin; // 0x00(0x0c)
	struct FVector BoxExtent; // 0x0c(0x0c)
	float SphereRadius; // 0x18(0x04)
};

// Class Engine.SkeletalMesh
// 0x0210 (0x0238 - 0x0028)
class USkeletalMesh
{
public:
	char pad_0000[88]; //0x0000
	struct FBoxSphereBounds Bounds; //0x0058
	char pad_0074[452]; //0x0074
};

// Function Athena.FOVHandlerFunctions.SetTargetFOV
struct UFOVHandlerFunctions_SetTargetFOV_Params
{
	class AAthenaPlayerCharacter* Character;                                                // (ConstParm, Parm, ZeroConstructor, IsPlainOldData)
	float                                              TargetFOV;                                                // (ConstParm, Parm, ZeroConstructor, IsPlainOldData)
};

// Function Athena.FOVHandlerFunctions.GetTargetFOV
struct UFOVHandlerFunctions_GetTargetFOV_Params
{
	AAthenaPlayerCharacter* Character;                                                // (ConstParm, Parm, ZeroConstructor, IsPlainOldData)
	float                                              ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
};



// Class Engine.World
// Size: 0x7c8 (Inherited: 0x28)
class UWorld : public UObject
{
public:
	char		UnknownData_28[0x8]; // 0x28(0x08)
	class		ULevel* PersistentLevel; // 0x30(0x08)
	class		UNetDriver* NetDriver; // 0x38(0x08)
	class		ULineBatchComponent* LineBatcher; // 0x40(0x08)
	class		ULineBatchComponent* PersistentLineBatcher; // 0x48(0x08)
	class		ULineBatchComponent* ForegroundLineBatcher; // 0x50(0x08)
	class		AGameState* GameState; // 0x58(0x08)
	class		AGameNetworkManager* NetworkManager; // 0x60(0x08)
	class		UPhysicsCollisionHandler* PhysicsCollisionHandler; // 0x68(0x08)
	struct		TArray<class UObject*> ExtraReferencedObjects; // 0x70(0x10)
	struct		TArray<class UObject*> PerModuleDataObjects; // 0x80(0x10)
	struct		TArray<class ULevelStreaming*> StreamingLevels; // 0x90(0x10)
	struct		FString StreamingLevelsPrefix; // 0xa0(0x10)
	class		ULevel* CurrentLevelPendingVisibility; // 0xb0(0x08)
	class		AParticleEventManager* MyParticleEventManager; // 0xb8(0x08)
	class		APhysicsVolume* DefaultPhysicsVolume; // 0xc0(0x08)
	struct		TArray<class ULevelStreaming*> DirtyStreamingLevels; // 0xc8(0x10)
	char		UnknownData_D8[0x1c]; // 0xd8(0x1c)
	struct		FName Feature; // 0xf4(0x08)
	char		UnknownData_FC[0x4]; // 0xfc(0x04)
	struct		TArray<struct FName> FeatureReferences; // 0x100(0x10)
	bool		ParticleLOD_bUseGameThread; // 0x110(0x01)
	bool		ParticleLOD_bUseMultipleViewportCase; // 0x111(0x01)
	char		UnknownData_112[0x2]; // 0x112(0x02)
	struct		FVector ParticleLOD_PlayerViewpointLocation; // 0x114(0x0c)
	struct		FString TestMetadata; // 0x120(0x10)
	class		UNavigationSystem* NavigationSystem; // 0x130(0x08)
	class		AGameMode* AuthorityGameMode; // 0x138(0x08)
	class		UAISystemBase* AISystem; // 0x140(0x08)
	class		UAvoidanceManager* AvoidanceManager; // 0x148(0x08)
	struct		TArray<class ULevel*> Levels; // 0x150(0x10)
	char		UnknownData_160[0x50]; // 0x160(0x50)
	class		ULevel* CurrentLevel; // 0x1b0(0x08)
	char		UnknownData_1B8[0x8]; // 0x1b8(0x08)
	class		UGameInstance* OwningGameInstance; // 0x1c0(0x08)
	struct		TArray<class UMaterialParameterCollectionInstance*> ParameterCollectionInstances; // 0x1c8(0x10)
	char		UnknownData_1D8[0x520]; // 0x1d8(0x520)
	class		UWorldComposition* WorldComposition; // 0x6f8(0x08)
	char		UnknownData_700[0x3d]; // 0x700(0x3d)
	char		UnknownData_73D_0 : 7; // 0x73d(0x01)
	char		bAreConstraintsDirty : 1; // 0x73d(0x01)
	char		UnknownData_73E[0x8a]; // 0x73e(0x8a)

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.World");
		return ptr;
	}

};

// Class Engine.OnlineSession
// 0x0000 (0x0028 - 0x0028)
class UOnlineSession : public UObject {
public:

};

// Class Engine.GameInstance
// 0x0060 (0x0088 - 0x0028)
class UGameInstance : public UObject
{
public:
	unsigned char                                      UnknownData00[0x10];                                      // 0x0028(0x0010) MISSED OFFSET
	TArray<class ULocalPlayer*>                        LocalPlayers;                                             // 0x0038(0x0010) (ZeroConstructor)
	unsigned char                                      UnknownData01[0x8];                                       // 0x0048(0x0008) MISSED OFFSET
	class UOnlineSession*							   OnlineSession;                                            // 0x0050(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData02[0x30];                                      // 0x0058(0x0030) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.GameInstance");
		return ptr;
	}
};

// Class Athena.AthenaGameViewportClient
// 0x0010 (0x0260 - 0x0250)
class UAthenaGameViewportClient : public UGameViewportClient
{
public:
	unsigned char                                      UnknownData00[0x10];                                      // 0x0250(0x0010) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Athena.AthenaGameViewportClient");
		return ptr;
	}
};
bool InitializeObjects();
bool InitializeNames();