#include "win_utils.h"
#include <random>

namespace Settings
{
	bool ShowMenu = false;
	bool Esp = true;
	bool Esp_box = true;
	bool Esp_line = false;
	bool Esp_distance = false;
	bool SelfESP = false;
	bool Weapons = false;
	bool Aimbot = true;
	bool Platform = false;
	bool LootESP;

	int Smoothing = 5;

	static int VisDist = 500;
	int AimFOV = 250;

	static int aimkey;
	static int hitbox = 109;
	static int aimkeypos = 3;
	static int hitboxpos = 0;
}

ImVec2 GetCenterPos(FVector2D Pos, std::string text, int TextPos)
{
	ImVec2 DistanceSize2 = ImGui::CalcTextSize(text.c_str());

	ImVec2 centerPos(Pos.x - DistanceSize2.x * 0.5f, Pos.y + TextPos);
	return centerPos;
}

ImFont* m_pFont;


uint64_t TargetPawn;
int localplayerID;

RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;

DWORD ScreenCenterX;
DWORD ScreenCenterY;

static void xCreateWindow();
static void xInitD3d();
static void xMainLoop();
static void xShutdown();
static LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static HWND Window = NULL;
IDirect3D9Ex* p_Object = NULL;
static LPDIRECT3DDEVICE9 D3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;

class FKeyInput
{
public:
	FKeyInput() : var(0)
	{

	}

	FKeyInput(int _var) : var(_var)
	{

	}

	~FKeyInput()
	{
		if (var != 0) {
			var = 0;
		}
	}


	void Clear()
	{
		this->var = 0;
	}

	bool IsPressingKey()
	{
		return GetAsyncKeyState_Spoofed(this->var);
	}

	bool WasRecentlyPressed()
	{
		if (GetAsyncKeyState_Spoofed(this->var))
		{
			if (!WasKeyAlreadyPressed)
			{
				return true;
				WasKeyAlreadyPressed = true;
			}
		}
		else {
			if (WasKeyAlreadyPressed)
			{
				WasKeyAlreadyPressed = false;
				return false;
			}
		}
	}

private:
	int var;
	bool WasKeyAlreadyPressed;
};

FKeyInput InsertKey(VK_INSERT);
FKeyInput RMBKey(VK_RBUTTON);
FKeyInput LMBKey(VK_LBUTTON);
FKeyInput ENDKey(0x23);

void DrawStrokeText(int x, int y, const char* str)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);

	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(255, 255, 255, 255)), utf_8_2.c_str());
}

float DrawOutlinedText(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center)
{
	std::stringstream stream(text);
	std::string line;

	float y = 0.0f;
	int i = 0;

	while (std::getline(stream, line))
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

		if (center)
		{
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
		}
		else
		{
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

			ImGui::GetOverlayDrawList()->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
		}

		y = pos.y + textSize.y * (i + 1);
		i++;
	}
	return y;
}

void DrawText1(int x, int y, const char* str, RGBA* color)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), utf_8_2.c_str());
}

void DrawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness)
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), thickness);
}

void DrawCircle(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments);
}

void DrawBox(float X, float Y, float W, float H, RGBA* color)
{
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(X + 1, Y + 1), ImVec2(((X + W) - 1), ((Y + H) - 1)), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(X, Y), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
}

void DrawCorneredBox(int X, int Y, int W, int H, const ImU32& color, int thickness) {
	float lineW = (W / 3);
	float lineH = (H / 3);

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}




static void __RtlSecureZeroMemory(void* pDest, size_t nSize)
{
	unsigned char* p = (unsigned char*)pDest;
	for (size_t i = 0; i < nSize; i++)
	{
		p[i] = 0;
	}
}

class OffsetDumping
{
public:
	OffsetDumping(DSAPI* _var) : API(_var)
	{
		this->API->DownloadContent();
	}

	~OffsetDumping()
	{
		if (this->API != nullptr) {
			this->API = nullptr;
		}
	}

	uint64_t GetFunctionOffset(const char* Class, const char* FunctionName)
	{
		return this->API->GetFunctionOffset(Class, FunctionName);
	}

	uint64_t FindOffset(const char* Class, const char* OffsetName)
	{
		return this->API->GetOffset(Class, OffsetName).offset;
	}

	uint64_t FindOffset(const char* PointerName)
	{
		return this->API->GetOffset(PointerName);
	}

private:
	DSAPI* API;
};


void PrintOffset(const char* Name, uint64_t Addy)
{
	printf("%s -> 0x%llX\n", Name, Addy);
}

float DrawPOSDebugY = 150.0;
float DrawPOSDebugX = 10.0;

FVector PredictPlayerPosition(FVector CurrentLocation, float Distance, FVector Velocity, AFortWeapon* CurrentWeapon)
{
	FVector CalculatedPosition = CurrentLocation;
	float ProjectileSpeed = CurrentWeapon->GetProjectileSpeed();
	float ProjectileGravity = CurrentWeapon->GetGravityScale();
	if (!ProjectileSpeed) return CalculatedPosition;


	float TimeToTarget = Distance / ExternalUtils::custom_fabsf(ProjectileSpeed);
	float CalculatedBulletDrop = (((-2800) * 0.5f * (TimeToTarget))) * 10;


	CalculatedPosition.x += Velocity.x * (TimeToTarget) * 120;
	CalculatedPosition.y += Velocity.y * (TimeToTarget) * 120;
	CalculatedPosition.z += Velocity.z * (TimeToTarget) * 120;
	CalculatedPosition.z += ExternalUtils::custom_fabsf((-49000 / 50) * ProjectileGravity) / 2.0f * (TimeToTarget * 25);


	return CalculatedPosition;
}
__forceinline void FindDumpedOffsets()
{
	OffsetDumping Temp_Dumper(new DSAPI("6b77eceb")); 
	OffsetDumping* Dumper = new OffsetDumping(Temp_Dumper);
	__RtlSecureZeroMemory(&Temp_Dumper, sizeof(Temp_Dumper));

	Offsets::UWorld = Dumper->FindOffset("OFFSET_UWORLD");
	Offsets::GameInstance = Dumper->FindOffset("UWorld", "OwningGameInstance");
	Offsets::LocalPlayers = Dumper->FindOffset("UGameInstance", "LocalPlayers");
	Offsets::PlayerController = Dumper->FindOffset("UPlayer", "PlayerController");
	Offsets::LocalPawn = Dumper->FindOffset("APlayerController", "AcknowledgedPawn");
	Offsets::PlayerState = Dumper->FindOffset("APawn", "PlayerState"); 
	Offsets::RootComponent = Dumper->FindOffset("AActor", "RootComponent");
	Offsets::PersistentLevel = Dumper->FindOffset("UWorld", "PersistentLevel");
	Offsets::ReviveFromDBNOTime = Dumper->FindOffset("AFortPlayerPawnAthena", "ReviveFromDBNOTime");
	Offsets::Mesh = Dumper->FindOffset("ACharacter", "Mesh");
	Offsets::TeamIndex = Dumper->FindOffset("AFortPlayerStateAthena", "TeamIndex");
	Offsets::RelativeLocation = Dumper->FindOffset("USceneComponent", "RelativeLocation");
	Offsets::CurrentWeapon = Dumper->FindOffset("AFortPawn", "CurrentWeapon");
	Offsets::PrimaryPickupItemEntry = Dumper->FindOffset("AFortPickup", "PrimaryPickupItemEntry");
	Offsets::ItemDefinition = Dumper->FindOffset("FFortItemEntry", "ItemDefinition");
	Offsets::Tier = Dumper->FindOffset("UFortItemDefinition", "Tier");
	Offsets::ItemName = Dumper->FindOffset("UItemDefinitionBase", "ItemName");
	Offsets::Levels = Dumper->FindOffset("UWorld", "Levels");
	Offsets::WeaponData = Dumper->FindOffset("AFortWeapon", "WeaponData");
	Offsets::AmmoCount = Dumper->FindOffset("AFortWeapon", "AmmoCount");
	Offsets::ComponentVelocity = Dumper->FindOffset("USceneComponent", "ComponentVelocity");
	Offsets::Platform = Dumper->FindOffset("AFortPlayerState", "Platform");

	__RtlSecureZeroMemory(&Dumper, sizeof(Dumper)); 
}

namespace Cached
{
	APlayerController* PlayerController;
	AFortPlayerState* PlayerState;
	AFortPlayerState* LocalPlayerState;
	AFortPlayerPawn* LocalPawn;
	USceneComponent* LocalRootComponent;
	ULocalPlayer* LocalPlayer;
	ULevel* PersistentLevel;
	UWorld* World;

	float closestDistance = FLT_MAX;
	AFortPlayerPawn* closestPawn = NULL;


	FVector localactorpos;
}

struct item
{
	AFortPlayerPawn* Actor;
	int Distance;
	FVector Location;
	FVector2D ScreenLocation;
};
std::vector<item> item_pawns;

typedef struct players {
	AFortPlayerPawn* actor;
	bool IsDBNO;
	USkeletalMeshComponent* mesh;

	int teamid;
	int myteamid;
}players;
std::vector<players> entitylist;

namespace Misc
{
	int i = 0;
}

void CacheActors() {
	for (;;) {
		std::vector<players> tmpList;

		Cached::World = request->read<UWorld*>(base_address + Offsets::UWorld); Uworld_Temp = Cached::World->GetAddress();

		UGameInstance* GameInstance = Cached::World->OwningGameInstance();
		Cached::LocalPlayer = request->read<ULocalPlayer*>(GameInstance->LocalPlayers());
		Cached::PlayerController = Cached::LocalPlayer->PlayerController(); Copy_PlayerController_Camera = Cached::PlayerController->GetAddress();
		Cached::LocalPawn = (AFortPlayerPawn*)Cached::PlayerController->AcknowledgedPawn();

		Cached::LocalPlayerState = Cached::LocalPawn->PlayerState();
		Cached::LocalRootComponent = Cached::LocalPawn->RootComponent();
		Cached::PersistentLevel = Cached::World->PersistentLevel();
		auto ActorCount = Cached::PersistentLevel->ActorCount();
		auto AActors = Cached::PersistentLevel->AActors();

		for (Misc::i = 0; Misc::i < ActorCount; ++Misc::i) {
			AFortPlayerPawn* Player = request->read<AFortPlayerPawn*>(AActors + Misc::i * 0x8);
			if (Player->ReviveFromDBNOTime() != 10) continue;
			USkeletalMeshComponent* Mesh = Player->Mesh();
			Cached::LocalPlayerState = Cached::LocalPawn->PlayerState();
			int MyTeamId = Cached::LocalPlayerState->TeamIndex();
			Cached::PlayerState = Player->PlayerState();
			int ActorTeamId = Cached::PlayerState->TeamIndex();

			if (Cached::PlayerState->GetAddress())
			{
				players fnlEntity{};
				fnlEntity.actor = Player;
				fnlEntity.mesh = Mesh;
				fnlEntity.myteamid = MyTeamId;
				fnlEntity.teamid = ActorTeamId;
				fnlEntity.IsDBNO = false;

				tmpList.push_back(fnlEntity);
			}
		}

		entitylist.clear();
		entitylist = tmpList;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void LevelsLoop()
{
	for (;;)
	{
		if (!Settings::LootESP)
		{
			item_pawns.clear();
		}

		if (!Cached::World->GetAddress()) continue;
		if (!Cached::PlayerController->GetAddress()) continue;

		std::vector<item> mrxd;
		ULevel* ItemLevels = request->read<ULevel*>(Cached::World->GetAddress() + Offsets::Levels);

		for (int i = 0; i < request->read<DWORD>(Cached::World->GetAddress() + (Offsets::Levels + sizeof(PVOID))); ++i) {
			FortPTR ItemLevel = request->read<FortPTR>(ItemLevels->GetAddress() + (i * sizeof(FortPTR)));

			for (int i = 0; i < request->read<DWORD>(ItemLevel + (Offsets::AActors + sizeof(PVOID))); ++i) {
				FortPTR ItemsPawns = request->read<FortPTR>(ItemLevel + Offsets::AActors);
				AFortPlayerPawn* CurrentItemPawn = request->read<AFortPlayerPawn*>(ItemsPawns + (i * sizeof(FortPTR)));
				FVector ItemPosition = CurrentItemPawn->RootComponent()->RelativeLocation();
				FVector2D ScreenLocation = Cached::PlayerController->ProjectWorldLocationToScreen(ItemPosition);
				int dist = Cached::localactorpos.Distance(ItemPosition) / 100;
				auto ActorCount = ItemLevels->ActorCount();

				item Item{ };
				Item.Actor = CurrentItemPawn;
				Item.Distance = dist;
				Item.Location = ItemPosition;
				Item.ScreenLocation = ScreenLocation;

				std::cout << "X -> " << ItemPosition.x << std::endl;
				std::cout << "Y -> " << ItemPosition.y << std::endl;

				ImGui::GetOverlayDrawList()->AddText(ScreenLocation, ImColor(255, 255, 255), "nerd");

				mrxd.push_back(Item);
			}
		}
		item_pawns.clear();
		item_pawns = mrxd;
	}
}

int main(int argc, const char* argv[])
{
	LI_FN(SetConsoleTitleA)(" ");
	mouse_interface();
	ntmouseinject::Init();

	std::cout << "Online" << std::endl;

	FindDumpedOffsets();
	PrintOffset("UWorld", Offsets::UWorld);
	PrintOffset("OwningGameInstance", Offsets::GameInstance);
	PrintOffset("LocalPlayers", Offsets::LocalPlayers);
	PrintOffset("PlayerController", Offsets::PlayerController);
	PrintOffset("LocalPawn", Offsets::LocalPawn);
	PrintOffset("PlayerState", Offsets::PlayerState);
	PrintOffset("RootComponent", Offsets::RootComponent);
	PrintOffset("PersistentLevel", Offsets::PersistentLevel);
	PrintOffset("ReviveFromDBNOTime", Offsets::ReviveFromDBNOTime);
	PrintOffset("Mesh", Offsets::Mesh);
	PrintOffset("TeamIndex", Offsets::TeamIndex);
	PrintOffset("RelativeLocation", Offsets::RelativeLocation);
	PrintOffset("CurrentWeapon", Offsets::CurrentWeapon);
	PrintOffset("PrimaryPickupItemEntry", Offsets::PrimaryPickupItemEntry);
	PrintOffset("ItemDefinition", Offsets::ItemDefinition);
	PrintOffset("Tier", Offsets::Tier);
	PrintOffset("ItemName", Offsets::ItemName);
	PrintOffset("Levels", Offsets::Levels);
	PrintOffset("WeaponData", Offsets::WeaponData);
	PrintOffset("AmmoCount", Offsets::AmmoCount);
	PrintOffset("ComponentVelocity", Offsets::ComponentVelocity);
	PrintOffset("Platform", Offsets::Platform);
	

	if (mem::SetupDriver())
	{
		std::cout << "Driver Found" << std::endl;
	}
	else {
		std::cout << "Driver Not Found" << std::endl;
	}

	mem::RestartBuffers();
	
	
	while (hwnd == NULL)
	{
		XorS(wind, "Fortnite  ");
		XorS(clas, "UnrealWindow");
		hwnd = FindWindowA_Spoofed(clas.decrypt(), wind.decrypt());
		LI_FN(Sleep)(100);
	}
	LI_FN(GetWindowThreadProcessId)(hwnd, &processID);
	mem::process_id = processID;
	base_address = mem::get_base_address("fortnite");
	mem::GetDTB();


	XorS(base, "Process base address: %p.\n");
	printf(base.decrypt(), (void*)base_address);



	xCreateWindow();
	xInitD3d();

	HANDLE thread1 = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)CacheActors, nullptr, NULL, nullptr);
	HANDLE thread2 = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)GetCameraInfo, nullptr, NULL, nullptr);

	xMainLoop();
	xShutdown();

	return 0;
}

void SetWindowToTarget()
{
	while (true)
	{
		if (hwnd)
		{
			ExternalUtils::custom_memset((&GameRect), 0, (sizeof(GameRect)));
			GetWindowRect_Spoofed(hwnd, &GameRect);
			Width = GameRect.right - GameRect.left;
			Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLongA_Spoofed(hwnd, GWL_STYLE);

			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				Height -= 39;
			}
			ScreenCenterX = Width / 2;
			ScreenCenterY = Height / 2;
			MoveWindow_Spoofed(Window, GameRect.left, GameRect.top, Width, Height, true);
		}
		else
		{
			LI_FN(exit)(0);
		}
	}
}

std::string GenerateRandomUppercaseString(int length) {
	const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const int charsetLength = charset.length();
	std::string result;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, charsetLength - 1);

	for (int i = 0; i < length; ++i) {
		result += charset[dis(gen)];
	}

	return result;
}

const MARGINS Margin = { -1 };

void xCreateWindow()
{
	CreateThread_Spoofed(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEX wc;
	ExternalUtils::custom_memset((&wc), 0, (sizeof(wc)));
	std::string Class = GenerateRandomUppercaseString(31);
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = Class.c_str();
	wc.lpfnWndProc = WinProc;
	RegisterClassExA_Spoofed(&wc);

	if (hwnd)
	{
		GetClientRect_Spoofed(hwnd, &GameRect);
		POINT xy;
		_ClientToScreen(hwnd, &xy);
		GameRect.left = xy.x;
		GameRect.top = xy.y;

		Width = GameRect.right;
		Height = GameRect.bottom;
	}
	else
		LI_FN(exit)(2);

	Window = CreateWindowExA_Spoofed(NULL, Class.c_str(), GenerateRandomUppercaseString(9).c_str(), WS_POPUP | WS_VISIBLE, 0, 0, Width, Height, 0, 0, 0, 0);

	DwmExtendFrameIntoClientArea(Window, &Margin);
	SetWindowLongA_Spoofed(Window, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	ShowWindow_Spoofed(Window, SW_SHOW);
	UpdateWindow_Spoofed(Window);
}

void xInitD3d()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow = Window;
	d3dpp.Windowed = TRUE;

	p_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3dDevice);

	ImGui::StyleColorsClassic();
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	style->WindowTitleAlign.x = 0.50f;
	style->FrameRounding = 2.0f;

	XorS(font, "C:\\Windows\\Fonts\\Arial.ttf");
	m_pFont = io.Fonts->AddFontFromFileTTF(font.decrypt(), 14.0f, nullptr, io.Fonts->GetGlyphRangesDefault());

	p_Object->Release();
}

void SetMouseAbsPosition(DWORD x, DWORD y)
{
	ntmouseinject::move(x, y);
}

void aimbot(float x, float y)
{
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	int AimSpeed = Settings::Smoothing;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(TargetX), static_cast<DWORD>(TargetY), NULL, NULL);

	return;
}

void AimAt(AFortPlayerPawn* entity, AFortWeapon* CurrentWeapon)
{
	USkeletalMeshComponent* currentactormesh = entity->Mesh();
	auto rootHead = currentactormesh->GetSocketLocation(Settings::hitbox);

	auto distance = Cached::localactorpos.Distance(rootHead) / 100;

	rootHead = PredictPlayerPosition(rootHead, distance, entity->RootComponent()->ComponentVelocity(), CurrentWeapon);
	FVector2D rootHeadOut = Cached::PlayerController->ProjectWorldLocationToScreen(rootHead);

	aimbot(rootHeadOut.x, rootHeadOut.y);
}

void LevelDrawing()
{
	auto levelListCopy = item_pawns;

	for (auto entity : levelListCopy)
	{
		if (Cached::LocalPawn->GetAddress() && entity.Actor)
		{
			if (Settings::LootESP)
			{
				AFortPickup* PickupActor = (AFortPickup*)entity.Actor;

				std::string BuiltString = std::to_string(request->read<float>(entity.Actor->GetAddress() + 0x750));

				ImVec2 DistanceSize2 = ImGui::CalcTextSize(BuiltString.c_str());

				ImVec2 centerPos(entity.ScreenLocation.x - DistanceSize2.x * 0.5f, entity.ScreenLocation.y);

				ImGui::GetOverlayDrawList()->AddText(centerPos, ImColor(255, 0, 255), "nerd wax");

				BuiltString.clear();
			}
		}
	}
}

void DrawESP() {
	int DynamicFOVVal = (DynamicFOVValue - 80) + Settings::AimFOV;
	DrawCircle(ScreenCenterX, ScreenCenterY, DynamicFOVVal, &Col.NiggaGreen, DynamicFOVVal / 10);

	XorS(frame, "%.1f Fps\n");
	char dist[64];
	sprintf_s(dist, frame.decrypt(), ImGui::GetIO().Framerate);
	DrawText1(15, 15, dist, &Col.darkgreen_);

	auto entityListCopy = entitylist;

	Cached::closestPawn = NULL;
	Cached::closestDistance = NULL;

	for (auto Entity : entityListCopy)
	{

        if (Entity.myteamid == Entity.teamid) continue;
		if (!Settings::SelfESP)
		{
			if (Entity.actor->GetAddress() == Cached::LocalPawn->GetAddress()) continue;
		}

		auto Mesh = Entity.mesh;
		
		FVector Headpos = Mesh->GetSocketLocation(109);
		Cached::localactorpos = Cached::LocalRootComponent->RelativeLocation();

		float distance = Cached::localactorpos.Distance(Headpos) / 100;

		FVector bone0 = Mesh->GetSocketLocation(0);
		FVector2D bottom = Cached::PlayerController->ProjectWorldLocationToScreen(bone0);
		FVector2D Headbox = Cached::PlayerController->ProjectWorldLocationToScreen(FVector(Headpos.x, Headpos.y, Headpos.z + 15));

		if (!Headbox.IsInScreen() || !bottom.IsInScreen()) continue;

		float BoxHeight = (float)(Headbox.y - bottom.y);
		float BoxWidth = BoxHeight * 0.380f;

		float LeftX = (float)Headbox.x - (BoxWidth / 1);
		float LeftY = (float)bottom.y;

		float CornerHeight = ExternalUtils::custom_fabsf(Headbox.y - bottom.y);
		float CornerWidth = CornerHeight * 0.75;

		double DrawPosY = 15;

		if (distance < Settings::VisDist)
		{
			if (Settings::Esp_box) {
				DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, IM_COL32(255, 255, 0, 200), 1.5);
			}

			if (Settings::Esp_distance) {
				XorS(dst, "Distance [%.fm]");
				char dist[64];
				sprintf_s(dist, dst.decrypt(), distance);
				DrawOutlinedText(m_pFont, dist, ImVec2(Headbox.x, Headbox.y - 35), 16.0f, IM_COL32(2255, 255, 0, 200), true);
			}

			if (Settings::Esp_line) {
				DrawLine(Width / 2, Height, bottom.x, bottom.y, &Col.red, 1.5);
			}

			if (Settings::Weapons)
			{
				if (AFortWeapon* CurrentWeapon = Entity.actor->CurrentWeapon()) {
					if (UFortItemDefinition* WeaponData = CurrentWeapon->WeaponData()) {
						auto ItemName = WeaponData->ItemName();
						if (!ExternalUtils::custom_strstr(ItemName.c_str(), E("Pickaxe"))) {
							if (int AmmoCount = CurrentWeapon->AmmoCount()) {
								ItemName.append(E(" [ "));
								ItemName.append(std::to_string(AmmoCount).c_str());
								ItemName.append(E(" ]"));
							}
						}
						ImVec2 CenterPos = GetCenterPos(Headbox, ItemName, DrawPosY);
						ImGui::GetOverlayDrawList()->AddText(CenterPos, Custom::GetColorByTier(WeaponData->Tier()).GetColor(), ItemName.c_str()); DrawPosY -= 14;
					}
				}
			}

			if (Settings::Platform) {
				DWORD_PTR test_platform = Entity.actor->PlayerState()->Platform();
				wchar_t platform[64];
				mem::read((PVOID)test_platform, reinterpret_cast<PVOID>(platform), sizeof(platform));
				std::wstring balls_sus(platform);
				std::string str_platform(balls_sus.begin(), balls_sus.end());
				ImColor PlatformColor(255, 255, 255);
				char* PlatformName;

				if (str_platform.find(EX("XBL")) != std::string::npos || str_platform.find(EX("XSX")) != std::string::npos) {
					PlatformName = EX("XBox");
					PlatformColor = ImColor(0, 255, 0);
				}
				else if (str_platform.find(EX("XSX")) != std::string::npos) {
					PlatformName = EX("Xbox Series S/X");
					PlatformColor = ImColor(0, 255, 0);
				}
				else if (str_platform.find(EX("PSN")) != std::string::npos) {
					PlatformName = EX("PS4");
					PlatformColor = ImColor(0, 0, 255);
				}
				else if (str_platform.find(EX("PS5")) != std::string::npos) {
					PlatformName = EX("PS5");
					PlatformColor = ImColor(0, 0, 255);
				}
				else if (str_platform.find(EX("WIN")) != std::string::npos) {
					PlatformName = EX("Windows");
					PlatformColor = ImColor(219, 184, 9);
				}
				else if (str_platform.find(EX("SWT")) != std::string::npos) {
					PlatformName = EX("Nintendo Switch");
					PlatformColor = ImColor(255, 0, 0);
				}
				else if (str_platform.find(EX("AND")) != std::string::npos) {
					PlatformName = EX("Android");
					PlatformColor = ImColor(0, 255, 0);
				}
				else if (str_platform.find(EX("MAC")) != std::string::npos) {
					PlatformName = EX("MacOS");
					PlatformColor = ImColor(255, 0, 255);
				}
				else if (str_platform.find(EX("LNX")) != std::string::npos) {
					PlatformName = EX("Linux");
					PlatformColor = ImColor(176, 159, 5);
				}
				else if (str_platform.find(EX("IOS")) != std::string::npos) {
					PlatformName = EX("IOS");
					PlatformColor = ImColor(255, 255, 255);
				}
				else {
					PlatformName = EX("BOT");
					PlatformColor = ImColor(255, 255, 255);
				}

				ImGui::GetOverlayDrawList()->AddText(GetCenterPos(Headbox, PlatformName, DrawPosY), PlatformColor, PlatformName); DrawPosY -= 14;
			}
		}

		auto dx = Headbox.x - (Width / 2);
		auto dy = Headbox.y - (Height / 2);
		auto dist = ExternalUtils::custom_sqrtf(dx * dx + dy * dy);


		if (dist < DynamicFOVVal && dist > Cached::closestDistance) {
			Cached::closestDistance = dist;
			Cached::closestPawn = Entity.actor;
		}
	}
	if (Settings::Aimbot)
	{
		if (Cached::closestPawn && RMBKey.IsPressingKey()) {
			auto Bottom = Cached::PlayerController->ProjectWorldLocationToScreen(Cached::closestPawn->Mesh()->GetSocketLocation(0));
			DrawLine(Width / 2, Height / 2, Bottom.x, Bottom.y, &Col.red, 1.5);
			if (AFortWeapon* CurrentWeapon = Cached::LocalPawn->CurrentWeapon())
			{
				AimAt(Cached::closestPawn, CurrentWeapon);
			}
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void render() {
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (InsertKey.IsPressingKey()) {
		Settings::ShowMenu = !Settings::ShowMenu;
		LI_FN(Sleep)(100);
	}

	if (Settings::ShowMenu)
	{
		ImGui::Begin("broken fn base", &Settings::ShowMenu, ImVec2(430, 400), 1.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Checkbox("Boxes", &Settings::Esp_box);
		ImGui::Checkbox("Snaplines", &Settings::Esp_line);
		ImGui::Checkbox("Distance", &Settings::Esp_distance);
		ImGui::Checkbox("Platform", &Settings::Platform);
		ImGui::Checkbox("Weapon Info", &Settings::Weapons);
		ImGui::Separator();
		ImGui::Checkbox("Loot", &Settings::LootESP);
		ImGui::Separator();
		ImGui::Checkbox("Aimbot", &Settings::Aimbot);
		ImGui::Combo("Aim Key", &Settings::aimkeypos, aimkeys, sizeof(aimkeys) / sizeof(*aimkeys));
		ImGui::Combo("Aim Hitbox", &Settings::hitboxpos, hitboxes, sizeof(hitboxes) / sizeof(*hitboxes));
		ImGui::SliderInt("Fov Size", &Settings::AimFOV, 50, 750);
		ImGui::SliderInt("Smoothing", &Settings::Smoothing, 1, 15);
		ImGui::End();
	}

	DrawESP();

	ImGui::EndFrame();
	D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (D3dDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		D3dDevice->EndScene();
	}
	HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		D3dDevice->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

MSG Message = { NULL };

void xMainLoop()
{
	static RECT old_rc;
	ExternalUtils::custom_memset((&Message), 0, (sizeof(MSG)));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessageA_Spoofed(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage_Spoofed(&Message);
			DispatchMessageA_Spoofed(&Message);
		}

		HWND hwnd_active = GetForegroundWindow_Spoofed();

		if (hwnd_active == hwnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos_Spoofed(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		if (ENDKey.IsPressingKey())
			LI_FN(exit)(8);

		RECT rc;
		POINT xy;

		ExternalUtils::custom_memset((&rc), 0, (sizeof(RECT)));
		ExternalUtils::custom_memset((&xy), 0, (sizeof(POINT)));
		GetClientRect_Spoofed(hwnd, &rc);
		_ClientToScreen(hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPosA_Spoofed(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (LMBKey.IsPressingKey()) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos_Spoofed(Window, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			D3dDevice->Reset(&d3dpp);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		xShutdown();
		PostQuitMessage(0);
		LI_FN(exit)(4);
		break;
	case WM_SIZE:
		if (D3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = D3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return (DefWindowProcA)(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void xShutdown()
{
	TriBuf->Release();
	D3dDevice->Release();
	p_Object->Release();

	DestroyWindow_Spoofed(Window);
	LI_FN(UnregisterClassA).get()("fgers", NULL);
}
