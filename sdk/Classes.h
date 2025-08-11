#pragma once
#include "Structs.h"
#include "../Encryption/ex.h"
#include <thread>


namespace Offsets {
	uint64_t
		UWorld,
		GameInstance,
		LocalPlayers,
		PlayerController,
		LocalPawn,
		PlayerState,
		RootComponent,
		PersistentLevel,
		ActorCount = 0xA8,
		AActors = 0xA0,
		ReviveFromDBNOTime,
		Mesh,
		TeamIndex,
		Platform,
		RelativeLocation,
		PrimaryPickupItemEntry,
		ItemDefinition,
		Tier,
		ItemName,
		Levels,
		WeaponData,
		AmmoCount,
		ComponentVelocity,
		CurrentWeapon;
}
#define FortPTR uintptr_t

#define CURRENT_CLASS reinterpret_cast<uintptr_t>(this)
#define DECLARE_MEMBER(type, name, offset) type name() { return request->read<type>(CURRENT_CLASS + offset); }
#define DECLARE_MEMBER_DIRECT(type, name, base, offset) type name() { request->read<type>(base + offset); }
#define DECLARE_MEMBER_BITFIELD(type, name, offset, index) type name() { request->read<type>(CURRENT_CLASS + offset) & (1 << index); }
#define APPLY_MEMBER(type, name, offset) void name( type val ) { request->write<type>(CURRENT_CLASS + offset, val); }
#define APPLY_MEMBER_BITFIELD(type, name, offset, index) void name( type val ) { request->write(CURRENT_CLASS + offset, |= val << index); }

int DynamicFOVValue;

DWORD_PTR Uworld_Temp;

class UObject {
public:
	FortPTR GetAddress()
	{
		return (FortPTR)this;
	}

	__forceinline operator uintptr_t() { return (FortPTR)this; }
};

class USceneComponent : public UObject
{
public:
	DECLARE_MEMBER(FVector, RelativeLocation, Offsets::RelativeLocation);
	DECLARE_MEMBER(FVector, ComponentVelocity, Offsets::ComponentVelocity);
	APPLY_MEMBER(FVector, K2_SetActorLocation, Offsets::RelativeLocation);
};

class AActor : public UObject {
public:
	DECLARE_MEMBER(USceneComponent*, RootComponent, Offsets::RootComponent);
};

class USkeletalMeshComponent : public AActor {
public:
	__forceinline FVector GetSocketLocation(int bone_id)
	{
		uintptr_t bone_array = request->read<uintptr_t>((FortPTR)this + 0x598);
		if (bone_array == NULL) bone_array = request->read<uintptr_t>((FortPTR)this + 0x598 + 0x10);
		FTransform bone = request->read<FTransform>(bone_array + (bone_id * 0x60));
		FTransform component_to_world = request->read<FTransform>((FortPTR)this + 0x1c0);
		D3DMATRIX matrix = MatrixMultiplication(bone.ToMatrixWithScale(), component_to_world.ToMatrixWithScale());
		return FVector(matrix._41, matrix._42, matrix._43);
	}
};

struct CameraInfo
{
	FVector location;
	FVector rotation;
	float fov;
};

CameraInfo Copy_CameraInfo;

FortPTR Copy_PlayerController_Camera;

void GetCameraInfo()
{
	for (;;) {
		if (Copy_PlayerController_Camera)
		{
			CameraInfo camera;
			auto location_pointer = request->read<uintptr_t>(Uworld_Temp + 0x110); //110
			auto rotation_pointer = request->read<uintptr_t>(Uworld_Temp + 0x120); //120

			struct RotationInfo
			{
				double pitch;
				char pad_0008[24];
				double yaw;
				char pad_0028[424];
				double roll;
			};
			RotationInfo RotInfo;

			RotInfo.pitch = request->read<double>(rotation_pointer);
			RotInfo.yaw = request->read<double>(rotation_pointer + 0x20);
			RotInfo.roll = request->read<double>(rotation_pointer + 0x1d0);

			camera.location = request->read<FVector>(location_pointer);
			camera.rotation.x = ExternalUtils::custom_asinf(RotInfo.roll) * (180.0 / 3.14159265358979323846264338327950288419716939937510);
			camera.rotation.y = ((ExternalUtils::custom_atan2f(RotInfo.pitch * -1, RotInfo.yaw) * (180.0 / 3.14159265358979323846264338327950288419716939937510)) * -1) * -1;
			camera.fov = request->read<float>(Copy_PlayerController_Camera + 0x394) * 90.f;

			Copy_CameraInfo = camera;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

class APlayerController : public AActor
{
public:
	D3DXMATRIX Matrix(FVector rot, FVector origin = FVector())
	{
		float radPitch = (rot.x * float(3.14159265358979323846264338327950288419716939937510) / 180.f);
		float radYaw = (rot.y * float(3.14159265358979323846264338327950288419716939937510) / 180.f);
		float radRoll = (rot.z * float(3.14159265358979323846264338327950288419716939937510) / 180.f);

		float SP = ExternalUtils::custom_sinf(radPitch);
		float CP = ExternalUtils::custom_cosf(radPitch);
		float SY = ExternalUtils::custom_sinf(radYaw);
		float CY = ExternalUtils::custom_cosf(radYaw);
		float SR = ExternalUtils::custom_sinf(radRoll);
		float CR = ExternalUtils::custom_cosf(radRoll);

		D3DMATRIX matrix;
		matrix.m[0][0] = CP * CY;
		matrix.m[0][1] = CP * SY;
		matrix.m[0][2] = SP;
		matrix.m[0][3] = 0.f;

		matrix.m[1][0] = SR * SP * CY - CR * SY;
		matrix.m[1][1] = SR * SP * SY + CR * CY;
		matrix.m[1][2] = -SR * CP;
		matrix.m[1][3] = 0.f;

		matrix.m[2][0] = -(CR * SP * CY + SR * SY);
		matrix.m[2][1] = CY * SR - CR * SP * SY;
		matrix.m[2][2] = CR * CP;
		matrix.m[2][3] = 0.f;

		matrix.m[3][0] = origin.x;
		matrix.m[3][1] = origin.y;
		matrix.m[3][2] = origin.z;
		matrix.m[3][3] = 1.f;

		return matrix;
	}

	FVector2D ProjectWorldLocationToScreen(FVector WorldLocation)
	{
		CameraInfo viewInfo = Copy_CameraInfo;
		DynamicFOVValue = viewInfo.fov;
		D3DMATRIX tempMatrix = Matrix(viewInfo.rotation);
		FVector vAxisX = FVector(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
		FVector vAxisY = FVector(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
		FVector vAxisZ = FVector(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);
		FVector vDelta = WorldLocation - viewInfo.location;
		FVector vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));
		if (vTransformed.z < 1.f)
			vTransformed.z = 1.f;

		return FVector2D((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / ExternalUtils::custom_tanf(viewInfo.fov * (float)3.14159265358979323846264338327950288419716939937510 / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / ExternalUtils::custom_tanf(viewInfo.fov * (float)3.14159265358979323846264338327950288419716939937510 / 360.f))) / vTransformed.z);
	}

	DECLARE_MEMBER(FortPTR, AcknowledgedPawn, Offsets::LocalPawn);
};

class ULevel : public UObject
{
public:
	DECLARE_MEMBER(DWORD, ActorCount, Offsets::ActorCount);
	DECLARE_MEMBER(FortPTR, AActors, Offsets::AActors);
};

class AFortPlayerState : public AActor
{
public:
	DECLARE_MEMBER(int, TeamIndex, Offsets::TeamIndex);
	DECLARE_MEMBER(DWORD_PTR, Platform, Offsets::Platform);
};

class UFortItemDefinition : public AActor
{
public:
	DECLARE_MEMBER(EFortItemTier, Tier, Offsets::Tier);
	std::string ItemName()
	{
		std::string PlayersWeaponName = "";
		uint64_t ItemName = request->read<uint64_t>((FortPTR)this + Offsets::ItemName);
		if (!ItemName) return "";

		uint64_t FData = request->read<uint64_t>(ItemName + 0x28);
		int FLength = request->read<int>(ItemName + 0x30);

		if (FLength > 0 && FLength < 50) {

			wchar_t* WeaponBuffer = new wchar_t[FLength];
			mem::read((PVOID)FData, (PVOID)WeaponBuffer, FLength * sizeof(wchar_t));
			std::wstring wstr_buf(WeaponBuffer);
			PlayersWeaponName.append(std::string(wstr_buf.begin(), wstr_buf.end()));

			delete[] WeaponBuffer;
		}
		return PlayersWeaponName;
	}
};

class AFortWeapon : public AActor
{
public:
	float GetProjectileSpeed()
	{
		std::string DisplayName = this->WeaponData()->ItemName();

		if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Striker AR").decrypt())) {
			return 80000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Nemesis AR").decrypt())) {
			return 80000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Enforcer AR").decrypt())) {
			return 80000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Reaper Sniper Rifle").decrypt())) {
			return 60000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Ranger Pistol").decrypt())) {
			return 60000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Hyper SMG").decrypt())) {
			return 70000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Burst SMG").decrypt()) && !ExternalUtils::custom_strstr(DisplayName.c_str(), E("Scoped Burst SMG").decrypt())) {
			return 70000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Heavy Sniper Rifle").decrypt()) || ExternalUtils::custom_strstr(DisplayName.c_str(), E("Hunter Bolt-Action Sniper").decrypt())) {
			return 45000;
		}
		if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Dragon's Breath Sniper").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Storm Scout").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Storm Scout Sniper Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Hunting Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Explosive Repeater Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Bolt-Action Sniper Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Suppressed Sniper Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Lever Action Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Boom Sniper Rifle").decrypt()))
		{
			return 30000.f;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("DMR").decrypt())) {
			return 53000;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Automatic Sniper Rifle").decrypt())) {
			return 50000.f;
		}

		return 0;
	}

	float GetGravityScale()
	{
		std::string DisplayName = this->WeaponData()->ItemName();

		if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Striker AR").decrypt())) {
			return 3.0; // maybe 3.5
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Nemesis AR").decrypt())) {
			return 3.0; // maybe 3.5
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Enforcer AR").decrypt())) {
			return 3.0; // maybe 3.5
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Reaper Sniper Rifle").decrypt())) {
			return 3.0;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Ranger Pistol").decrypt())) {
			return 2;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Hyper SMG").decrypt())) {
			return 3;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Burst SMG").decrypt()) && !ExternalUtils::custom_strstr(DisplayName.c_str(), E("Scoped Burst SMG").decrypt())) {
			return 3;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Heavy Sniper Rifle").decrypt()) || ExternalUtils::custom_strstr(DisplayName.c_str(), E("hunter bolt-action sniper").decrypt())) {
			return 0.12;
		}
		if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Dragon's Breath Sniper").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Storm Scout").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Storm Scout Sniper Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Hunting Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Explosive Repeater Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Bolt-Action Sniper Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Suppressed Sniper Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Lever Action Rifle").decrypt()) ||
			ExternalUtils::custom_strstr(DisplayName.c_str(), E("Boom Sniper Rifle").decrypt()))
		{
			return 0.12;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("DMR").decrypt())) {
			return 0.15;
		}
		else if (ExternalUtils::custom_strstr(DisplayName.c_str(), E("Automatic Sniper Rifle").decrypt())) {
			return 0.12;
		}

		return 0;
	}

	DECLARE_MEMBER(UFortItemDefinition*, WeaponData, Offsets::WeaponData);
	DECLARE_MEMBER(int, AmmoCount, Offsets::AmmoCount);
};

class AFortPlayerPawn : public AActor
{
public:
	DECLARE_MEMBER(float, ReviveFromDBNOTime, Offsets::ReviveFromDBNOTime);
	DECLARE_MEMBER(AFortPlayerState*, PlayerState, Offsets::PlayerState);
	DECLARE_MEMBER(USkeletalMeshComponent*, Mesh, Offsets::Mesh);
	DECLARE_MEMBER(AFortWeapon*, CurrentWeapon, Offsets::CurrentWeapon);
};

class ULocalPlayer : public AFortPlayerPawn
{
public:
	DECLARE_MEMBER(APlayerController*, PlayerController, Offsets::PlayerController);
};

class UGameInstance : public AActor
{
public:
	DECLARE_MEMBER(DWORD_PTR, LocalPlayers, Offsets::LocalPlayers);
};

class UWorld : public AActor
{
public:
	DECLARE_MEMBER(UGameInstance*, OwningGameInstance, Offsets::GameInstance);
	DECLARE_MEMBER(ULevel*, PersistentLevel, Offsets::PersistentLevel);
};

class FFortItemEntry : public UFortItemDefinition
{
public:
	DECLARE_MEMBER(UFortItemDefinition*, ItemDefinition, Offsets::ItemDefinition);
};

class AFortPickup : public AActor
{
public:
	DECLARE_MEMBER(FFortItemEntry*, PrimaryPickupItemEntry, Offsets::PrimaryPickupItemEntry);
};


namespace Custom
{
	FLinearColor GetColorByTier(EFortItemTier Tier)
	{
		FLinearColor render_color = FLinearColor(1.f, 1.f, 1.f, 1.f);

		switch (Tier) {
		case EFortItemTier::I:
			render_color = FLinearColor(0.4f, 0.4f, 0.4f, 1.f);
			break;
		case EFortItemTier::II:
			render_color = FLinearColor(0.2f, 0.8f, 0.4f, 1.f);
			break;
		case EFortItemTier::III:
			render_color = FLinearColor(0.f, 0.4f, 0.8f, 1.f);
			break;
		case EFortItemTier::IV:
			render_color = FLinearColor(1.f, 0.f, 1.f, 1.f);
			break;
		case EFortItemTier::V:
			render_color = FLinearColor(0.7f, 0.7f, 0.f, 1.f);
			break;

		case EFortItemTier::VI:
			render_color = FLinearColor(1.f, 1.f, 0.f, 1.f);
			break;
		}

		return render_color;
	}
}