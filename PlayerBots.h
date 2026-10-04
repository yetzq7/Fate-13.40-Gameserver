#pragma once
#include "pch.h"
#include "Utils.h"
#include "Inventory.h"
#include "LateGame.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <algorithm>

inline std::vector<UAthenaCharacterItemDefinition*> PlayerBotCIDs{};
inline std::vector<UAthenaPickaxeItemDefinition*> PlayerBotPickaxes{};
inline std::vector<UAthenaBackpackItemDefinition*> PlayerBotBackpacks{};
inline std::vector<UAthenaGliderItemDefinition*> PlayerBotGliders{};
inline std::vector<UAthenaSkyDiveContrailItemDefinition*> PlayerBotContrails{};
inline std::vector<UAthenaDanceItemDefinition*> PlayerBotDances{};
struct FPendingBotDeathContext
{
	AFortPlayerControllerAthena* KillerPC = nullptr;
	AFortPlayerStateAthena* KillerPlayerState = nullptr;
	AFortPlayerPawnAthena* KillerPawn = nullptr;
	AActor* DamageCauser = nullptr;
};

inline std::unordered_map<AFortAthenaAIBotController*, FPendingBotDeathContext> PendingBotDeathContexts{};

namespace PlayerBots
{
	inline void RecordDeathContext(AFortAthenaAIBotController* DeadBotController, AController* InstigatedBy, AActor* DamageCauser)
	{
		if (!DeadBotController)
			return;

		FPendingBotDeathContext Context{};
		Context.DamageCauser = DamageCauser;

		auto* KillerPC = InstigatedBy ? InstigatedBy->Cast<AFortPlayerControllerAthena>() : nullptr;
		if (KillerPC)
		{
			Context.KillerPC = KillerPC;
			Context.KillerPlayerState = KillerPC->PlayerState ? (AFortPlayerStateAthena*)KillerPC->PlayerState : nullptr;
			Context.KillerPawn = KillerPC->MyFortPawn ? (AFortPlayerPawnAthena*)KillerPC->MyFortPawn : (KillerPC->Pawn ? (AFortPlayerPawnAthena*)KillerPC->Pawn : nullptr);
		}

		PendingBotDeathContexts[DeadBotController] = Context;
	}

	inline FPendingBotDeathContext ConsumeDeathContext(AFortAthenaAIBotController* DeadBotController)
	{
		FPendingBotDeathContext Context{};
		if (!DeadBotController)
			return Context;

		auto Found = PendingBotDeathContexts.find(DeadBotController);
		if (Found == PendingBotDeathContexts.end())
			return Context;

		Context = Found->second;
		PendingBotDeathContexts.erase(Found);
		return Context;
	}
}
// Not right for this season but good enough for testing :)
inline std::vector<FVector> PlayerBotDropZoneLocations{
	{ 106902, -84901, -1834 },
	{ 17085.8, 112747.0, 0.0 },      // Dirty Docks
	{ 81989.6, 96710.3, 2000.0 },    // Steamy Stacks
	{ 35365.6, 45193.9, 0.0 },       // Frenzy Farm
	{ 97044.2, 26667.1, 0.0 },       // Craggy Cliffs
	{ 33805.2, -72550.6, 0.0 },      // Sweaty Sands
	{ -13311.9, -81033.7, 0.0 },     // Holly Hedges
	{ -64881.4, -46495.3, 0.0 },     // Slurpy Swamp
	{ -31581.6, -27604.8, 0.0 },     // Weeping Woods
	{ 12554.8, -24382.9, 0.0 },      // Salty Springs
	{ 31790.20, 11209.48, 0.0 },     // Risky
	{ 62435.3, -16136.0, 0.0 },      // Pleasant Park
	{ -46128.3, 52456.6, 9000.0 },   // Lazy Lake
	{ -40262.6, 95645.5, 5000.0 },   // Retail Row
	{ 93628.7, 51110.4, 4691.0 },    // Outskirt 1
	{ -92899.5, 85380.9, 17392.9 },  // Outskirt 2
	{ -92470.2, 77704.0, 9370.9 },   // Outskirt 3
	{ -115316.3, 80257.6, -2469.1 }, // Outskirt 4
	{ -110473.8, 45607.16, 7876.7 }, // Outskirt 5
	{ -28723.2, 4098.7, 6108.8 },    // Outskirt 6
	{ -89870.1, 26228.7, 2000.0 },   // Misty Meadows
	{ 107834, -78584, -783 },
	{ 120009, -84032, -3370 },
	{ 112255, -91220, -3011 },
	{ 99820, -82200, -3370 },
	{ 96764, 29166, -2226 },
	{ 104078, 26670, -1834 },
	{ 94032, 51513, 4693 },
	{ 76334, 90345, -1450 },
	{ 81669, 90762, 469 },
	{ 80258, 95433, 69 },
	{ 86691, 102076, 69 },
	{ 113312, 113547, -1837 },
	{ 116567, 113665, -2602 },
	{ 109895, 113636, -2986 },
	{ 106233, 108428, -3762 },
	{ 64240, -16323, 69 },
	{ 60290, -16240, 69 },
	{ 31257, -77599, 69 },
	{ 13637, -24219, 69 },
	{ 30808, 10669, 69 },
	{ 30536, 42295, 69 },
	{ 30437, 69691, 69 },
	{ 17963, 112955, 69 },
	{ 6364, 4866, 69 },
	{ 6426, 7100, 69 },
	{ 6245, 1741, 69 },
	{ 11188, 4863, 69 },
	{ 657, 4334, 69 },
	{ 4084, 7167, 69 },
	{ -13311, -81033, 69 },
	{ -28997, -31787, 69 },
	{ -64881, -46495, 69 },
	{ -77143, -80634, 69 },
	{ -73305, -90910, 69 },
	{ -88282, 23038, 69 },
	{ -88329, 31859, 69 },
	{ -68041, 29906, 69 },
	{ -43937, 52030, 69 },
	{ -54818, 57400, 69 },
	{ -68550, 80804, 69 },
	{ -92971, 78709, 69 },
	{ -39867, 91323, 69 },
	{ -38750, 102176, 69 },
	{ -19544, 105594, 69 },
	{ -17383, 112993, 69 },
	{ -22500, 112479, 69 },
};

enum class EPlayerBotState : uint8
{
	Warmup,
	Bus,
	Skydiving,
	Gliding,
	Landed,
	Looting,
	LookingForPlayers,
	MovingToSafeZone,
	Stuck
};

enum class EPlayerBotStrafeType : uint8
{
	Left,
	Right
};

enum class ELootableType : uint8
{
	None = 255,
	Chest = 0,
	Pickup = 1
};

static bool IsActorValid(AActor* Actor)
{
	return Actor != nullptr && !Actor->bHidden;
}

static bool IsPawnAlive(AFortPlayerPawnAthena* Pawn)
{
	return Pawn != nullptr && !Pawn->bIsDying && !Pawn->bIsDBNO && !Pawn->bHidden;
}

namespace GlobalPickupCache
{
	inline std::vector<AFortPickupAthena*> Pickups{};
	inline float                           Timestamp = -1.f;
	static constexpr float                 kLifetime = 3.0f;

	inline void Refresh(UWorld* World)
	{
		Pickups.clear();
		TArray<AActor*> Actors;
		UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(World, AFortPickupAthena::StaticClass(), &Actors);
		Pickups.reserve(Actors.Num());
		for (int32 i = 0; i < Actors.Num(); ++i)
		{
			if (auto* P = (AFortPickupAthena*)Actors[i])
				Pickups.push_back(P);
		}
		Actors.Free();
		Timestamp = UGameplayStatics::GetTimeSeconds(World);
	}

	inline void EnsureFresh(UWorld* World)
	{
		if (!World) return;
		auto Now = UGameplayStatics::GetTimeSeconds(World);
		if (Timestamp < 0.f || (Now - Timestamp) > kLifetime)
			Refresh(World);
	}

	inline void Invalidate() { Timestamp = -1.f; }
}

namespace GlobalChestNodes
{
	struct FChestNode { FVector Location{}; };

	inline bool                     bLoaded = false;
	inline std::vector<FChestNode>  Nodes{};

	inline void EnsureLoaded()
	{
		if (bLoaded) return;
		bLoaded = true;

		std::ifstream GraphDump("C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotGraph.txt");
		std::string Line;
		while (std::getline(GraphDump, Line))
		{
			if (Line.rfind("NODE|", 0) != 0) continue;
			if (Line.find("|Type=CHEST|") == std::string::npos &&
				Line.find("|Type=FACTION_CHEST|") == std::string::npos)
				continue;

			auto ReadFloatField = [&](const char* Key, float& Out) -> bool
				{
					auto Pos = Line.find(Key);
					if (Pos == std::string::npos) return false;
					Pos += strlen(Key);
					auto End = Line.find('|', Pos);
					Out = (float)atof(Line.substr(Pos, End == std::string::npos ? std::string::npos : End - Pos).c_str());
					return true;
				};

			FChestNode Node{};
			if (ReadFloatField("|X=", Node.Location.X) &&
				ReadFloatField("|Y=", Node.Location.Y) &&
				ReadFloatField("|Z=", Node.Location.Z))
			{
				Nodes.push_back(Node);
			}
		}
	}
}

struct PlayerBot
{
	AFortPlayerPawnAthena* Pawn = nullptr;
	ABP_PhoebePlayerController_C* PC = nullptr;
	AFortPlayerStateAthena* PlayerState = nullptr;
	EPlayerBotState              State = EPlayerBotState::Warmup;
	bool bHasThankedBusDriver = false;
	bool bHasJumpedFromBus = false;
	bool bLoggedBusState = false;
	bool bUseConfiguredDropZone = false;
	bool bIsDead = false;
	bool bIsCurrentlyStrafing = false;
	bool bIsStressed = false;
	bool bWakeAIInitialized = false;
	bool bLoggedMissingNavData = false;
	bool bLoggedMissingPathComponent = false;
	bool bLoggedMissingBrainComponent = false;
	AActor* TargetLootable = nullptr;
	AActor* CurrentEnemy = nullptr;
	AActor* NearestPlayerActor = nullptr;
	AActor* LastMoveTargetActor = nullptr;
	FVector TargetDropZone = FVector();
	FVector TargetLootLocation = FVector();
	FVector LastStuckCheckLocation = FVector();
	FVector LastGoalLocation = FVector();
	FRotator CurrentAimRotation = FRotator();
	float   ClosestDistToDropZone = FLT_MAX;
	float   StrafeEndTime = 0.f;
	float   LootTargetStartTime = 0.f;
	float   LastLootTargetDistance = 0.f;
	float   TimeToNextAction = 0.f;
	float   AirStallStartTime = 0.f;
	float   LastProgressTime = 0.f;
	float   NextLootScanTime = 0.f;
	float   NextPlayerScanTime = 0.f;
	float   NextNearbyPickupCollectTime = 0.f;
	float   NextMoveCommandTime = 0.f;
	float   NextWakeAITime = 0.f;
	float   LastDebugLogTime = 0.f;
	int32   PickupScanCursor = 0;
	int32   ChestScanCursor = 0;
	int32   WakeAIAttemptCount = 0;
	ELootableType       TargetLootableType = ELootableType::None;
	EPlayerBotStrafeType StrafeType = EPlayerBotStrafeType::Left;
	EPlayerBotState     CachedState = EPlayerBotState::Warmup;
	EPlayerBotState     LastLoggedState = EPlayerBotState::Warmup;
	uint64              TickCounter = 0;

	std::vector<FVector>       IgnoredChestLocations{};
	std::unordered_set<int32>  IgnoredChestNodeIndices{};

	// -------------------------------------------------------------------------
	void EnsureInventory()
	{
		if (!PC || PC->Inventory)
			return;

		auto* InventoryActor = Utils::SpawnActor<AFortInventory>(FVector{ 0, 0, -99999 }, {});
		if (!InventoryActor)
			return;

		InventoryActor->Owner = PC;
		InventoryActor->OnRep_Owner();
		PC->Inventory = InventoryActor;
	}

	void GiveItemBot(UFortItemDefinition* Def, int Count = 1, int LoadedAmmo = 0)
	{
		EnsureInventory();
		if (!Def || !PC || !PC->Inventory)
			return;

		auto* Item = (UFortWorldItem*)Def->CreateTemporaryItemInstanceBP(Count, 0);
		if (!Item)
			return;

		Item->OwnerInventory = PC->Inventory;
		Item->ItemEntry.LoadedAmmo = LoadedAmmo;
		PC->Inventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
		PC->Inventory->Inventory.ItemInstances.Add(Item);
		PC->Inventory->Inventory.MarkItemDirty(Item->ItemEntry);
		PC->Inventory->Inventory.MarkArrayDirty();
		PC->Inventory->HandleInventoryLocalUpdate();
	}

	void Emote()
	{
		if (!Pawn || !PC || Pawn->CosmeticLoadout.Dances.Num() <= 0)
			return;

		auto* EmoteDef = Pawn->CosmeticLoadout.Dances[UKismetMathLibrary::RandomIntegerInRange(0, Pawn->CosmeticLoadout.Dances.Num() - 1)];
		if (!EmoteDef)
			return;

		auto* ASC = PC->PlayerState ? ((AFortPlayerStateAthena*)PC->PlayerState)->AbilitySystemComponent : nullptr;
		if (!ASC)
			return;

		UClass* Ability = nullptr;
		if (auto* Dance = EmoteDef->Cast<UAthenaDanceItemDefinition>())
		{
			auto DanceAbility = Dance->CustomDanceAbility.Get();
			Ability = DanceAbility ? DanceAbility : UGAB_Emote_Generic_C::StaticClass();
			Pawn->bMovingEmote = Dance->bMovingEmote;
			Pawn->bMovingEmoteForwardOnly = Dance->bMoveForwardOnly;
			Pawn->EmoteWalkSpeed = Dance->WalkForwardSpeed;
		}

		if (!Ability)
			return;

		FGameplayAbilitySpec Spec{};
		((void (*)(FGameplayAbilitySpec*, UObject*, int, int, UObject*))ConstructAbilitySpec)(&Spec, Ability->DefaultObject, 1, -1, EmoteDef);
		FGameplayAbilitySpecHandle Handle;
		((void (*)(UFortAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec*, void*))(ImageBase + 0x9a5f70))((UFortAbilitySystemComponent*)ASC, &Handle, &Spec, nullptr);
	}

	FFortItemEntry* GetEntry(UFortItemDefinition* Def)
	{
		if (!PC || !PC->Inventory || !Def)
			return nullptr;

		for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); ++i)
		{
			if (PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition == Def)
				return &PC->Inventory->Inventory.ReplicatedEntries[i];
		}
		return nullptr;
	}

	void EquipPickaxe()
	{
		if (!Pawn || !PC || !PC->Inventory)
			return;
		if (State == EPlayerBotState::Bus || State == EPlayerBotState::Skydiving || State == EPlayerBotState::Gliding)
			return;
		if (IsPickaxeEquipped())
			return;

		for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); ++i)
		{
			auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition && Entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>())
			{
				Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
				return;
			}
		}

		static auto DefaultPickaxe = Utils::FindObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		if (DefaultPickaxe)
		{
			GiveItemBot(DefaultPickaxe);
			for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); ++i)
			{
				auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
				if (Entry.ItemDefinition == DefaultPickaxe)
				{
					Pawn->EquipWeaponDefinition(DefaultPickaxe, Entry.ItemGuid);
					return;
				}
			}
		}
	}

	bool IsPickaxeEquipped() const
	{
		return Pawn
			&& Pawn->CurrentWeapon
			&& Pawn->CurrentWeapon->WeaponData
			&& Pawn->CurrentWeapon->WeaponData->IsA<UFortWeaponMeleeItemDefinition>();
	}

	bool HasGun() const
	{
		if (!PC || !PC->Inventory)
			return false;

		for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); ++i)
		{
			auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition && Entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>())
				return true;
		}
		return false;
	}

	void LookAt(AActor* Actor)
	{
		if (!PC) return;
		if (!Actor) { PC->K2_ClearFocus(); return; }
		if (PC->GetFocusActor() != Actor)
			PC->K2_SetFocus(Actor);
	}

	void FaceLocation(const FVector& TargetLocation)
	{
		if (!Pawn || !PC) return;
		auto LookRotation = UKismetMathLibrary::FindLookAtRotation(Pawn->K2_GetActorLocation(), TargetLocation);
		PC->SetControlRotation(LookRotation);
		PC->K2_SetActorRotation(LookRotation, true);
	}

	void FaceActor(AActor* Actor)
	{
		if (!Actor) return;
		FaceLocation(Actor->K2_GetActorLocation());
		LookAt(Actor);
	}

	bool IsThreatUsable(AActor* Threat)
	{
		if (!Threat || Threat == Pawn || Threat->bHidden)
			return false;
		if (auto* ThreatPawn = Threat->Cast<AFortPlayerPawnAthena>())
			if (ThreatPawn->bIsDying || ThreatPawn->bIsDBNO)
				return false;
		return true;
	}

	bool ReactToRearThreat(AActor* Threat, float TriggerDistance = 1400.f)
	{
		if (!Pawn || !PC || !IsThreatUsable(Threat))
			return false;

		auto BotLocation = Pawn->K2_GetActorLocation();
		auto ThreatLocation = Threat->K2_GetActorLocation();
		auto Distance = UKismetMathLibrary::Vector_Distance(BotLocation, ThreatLocation);
		if (Distance > TriggerDistance)
			return false;

		auto ToThreat = ThreatLocation - BotLocation;
		auto PlanarLength = sqrtf((ToThreat.X * ToThreat.X) + (ToThreat.Y * ToThreat.Y));
		if (PlanarLength <= 1.f)
			return false;

		auto Forward = Pawn->GetActorForwardVector();
		auto Dot = ((ToThreat.X / PlanarLength) * Forward.X) + ((ToThreat.Y / PlanarLength) * Forward.Y);
		if (Dot > -0.1f && Distance > 350.f)
			return false;

		FaceLocation(ThreatLocation);
		LookAt(Threat);

		if (HasGun())
		{
			SimpleSwitchToWeapon();
			State = EPlayerBotState::LookingForPlayers;
			MoveToActorOnNav(Threat, UKismetMathLibrary::RandomFloatInRange(450.f, 900.f), true);
		}
		else
		{
			State = EPlayerBotState::Looting;
			MoveToActorOnNav(Threat, 75.f, true);
		}
		return true;
	}

	bool MoveToLocationOnNav(const FVector& Destination, float AcceptanceRadius = 75.f, bool bAllowPartialPath = true)
	{
		if (!PC || !Pawn) return false;

		auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
		if (CurrentTime < NextMoveCommandTime &&
			UKismetMathLibrary::Vector_DistanceSquared(LastGoalLocation, Destination) <= 400.f)
			return true;

		auto Result = PC->MoveToLocation(Destination, AcceptanceRadius, true, true, true, true, nullptr, bAllowPartialPath);


		auto BotLocation = Pawn->K2_GetActorLocation();
		auto Direction = Destination - BotLocation;
		auto Length = sqrtf(Direction.X * Direction.X + Direction.Y * Direction.Y + Direction.Z * Direction.Z);
		if (Length > AcceptanceRadius)
		{
			Direction.X /= Length;
			Direction.Y /= Length;
			Direction.Z = 0.f;
			Pawn->AddMovementInput(Direction, 1.f, true);
		}

		if (Result != EPathFollowingRequestResult::Failed)
		{
			LastGoalLocation = Destination;
			NextMoveCommandTime = CurrentTime + 0.2f;
		}
		return Result != EPathFollowingRequestResult::Failed;
	}

	bool MoveToActorOnNav(AActor* TargetActor, float AcceptanceRadius = 75.f, bool bAllowPartialPath = true)
	{
		if (!PC || !TargetActor || TargetActor == Pawn || TargetActor->bHidden)
			return false;

		if (auto* TargetPawn = TargetActor->Cast<AFortPlayerPawnAthena>())
			if (TargetPawn->bIsDying || TargetPawn->bIsDBNO)
				return false;

		auto TargetLocation = TargetActor->K2_GetActorLocation();

		// Direct movement input fallback
		if (Pawn)
		{
			auto BotLocation = Pawn->K2_GetActorLocation();
			auto Direction = TargetLocation - BotLocation;
			auto Length = sqrtf(Direction.X * Direction.X + Direction.Y * Direction.Y);
			if (Length > AcceptanceRadius)
			{
				Direction.X /= Length;
				Direction.Y /= Length;
				Direction.Z = 0.f;
				Pawn->AddMovementInput(Direction, 1.f, true);
			}
		}

		LastMoveTargetActor = TargetActor;
		return MoveToLocationOnNav(TargetLocation, AcceptanceRadius, bAllowPartialPath);
	}

	void EquipGun()
	{
		if (!Pawn || !PC || !PC->Inventory)
			return;
		if (State == EPlayerBotState::Bus || State == EPlayerBotState::Skydiving || State == EPlayerBotState::Gliding)
			return;
		if (Pawn->CurrentWeapon && Pawn->CurrentWeapon->WeaponData && Pawn->CurrentWeapon->WeaponData->IsA<UFortWeaponRangedItemDefinition>())
			return;

		for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); ++i)
		{
			auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition && Entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>())
			{
				Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
				return;
			}
		}
	}

	void SimpleSwitchToWeapon()
	{
		if (!HasGun() || !IsPickaxeEquipped())
			return;
		EquipGun();
	}

	bool IsIgnoredChestLocation(const FVector& Location) const
	{
		for (const auto& IgnoredLocation : IgnoredChestLocations)
			if (UKismetMathLibrary::Vector_DistanceSquared(IgnoredLocation, Location) <= 160000.f)
				return true;
		return false;
	}
	void IgnoreChestLocation(const FVector& Location)
	{
		if (Location.IsZero() || IsIgnoredChestLocation(Location))
			return;

		IgnoredChestLocations.push_back(Location);

		// Mark the matching node index so the scanner can skip it
		GlobalChestNodes::EnsureLoaded();
		const auto& Nodes = GlobalChestNodes::Nodes;
		for (int32 Idx = 0; Idx < (int32)Nodes.size(); ++Idx)
		{
			if (UKismetMathLibrary::Vector_DistanceSquared(Nodes[Idx].Location, Location) <= 160000.f)
				IgnoredChestNodeIndices.insert(Idx);
		}
	}

	void ClearLootTarget(bool bPreserveChestLocation = false)
	{
		TargetLootable = nullptr;
		TargetLootableType = ELootableType::None;
		LootTargetStartTime = 0.f;
		TimeToNextAction = 0.f;
		LastMoveTargetActor = nullptr;
		if (!bPreserveChestLocation)
			TargetLootLocation = FVector();
	}

	const char* GetStateName(EPlayerBotState InState) const
	{
		switch (InState)
		{
		case EPlayerBotState::Warmup:            return "Warmup";
		case EPlayerBotState::Bus:               return "Bus";
		case EPlayerBotState::Skydiving:         return "Skydiving";
		case EPlayerBotState::Gliding:           return "Gliding";
		case EPlayerBotState::Landed:            return "Landed";
		case EPlayerBotState::Looting:           return "Looting";
		case EPlayerBotState::LookingForPlayers: return "LookingForPlayers";
		case EPlayerBotState::MovingToSafeZone:  return "MovingToSafeZone";
		case EPlayerBotState::Stuck:             return "Stuck";
		default:                                  return "Unknown";
		}
	}

	std::string GetDebugName() const
	{
		if (Pawn) return Pawn->Name.ToString().c_str();
		if (PC)   return PC->Name.ToString().c_str();
		return "UnknownBot";
	}

	void DebugLog(const std::string& Message, bool bForce = false)
	{
		auto* World = UWorld::Get();
		auto  CurrentTime = World ? UGameplayStatics::GetTimeSeconds(World) : 0.f;
		if (!bForce && CurrentTime < LastDebugLogTime + 0.25f)
			return;

		LastDebugLogTime = CurrentTime;

		std::ofstream DebugFile("C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotDebug.log", std::ios::app);
		if (!DebugFile.is_open())
			return;

		auto Location = Pawn ? Pawn->K2_GetActorLocation() : FVector();
		DebugFile
			<< "[t=" << CurrentTime << "]"
			<< "[bot=" << GetDebugName() << "]"
			<< "[state=" << GetStateName(State) << "]"
			<< "[wake=" << (bWakeAIInitialized ? 1 : 0) << "]"
			<< "[wakeAttempts=" << WakeAIAttemptCount << "]"
			<< "[loc=" << Location.X << "," << Location.Y << "," << Location.Z << "] "
			<< Message
			<< "\n";
	}

	void DebugSnapshot(const char* Reason, bool bForce = true)
	{
		auto TargetLocation = TargetLootable ? TargetLootable->K2_GetActorLocation() : TargetLootLocation;
		std::ostringstream Stream;
		Stream
			<< "snapshot reason=" << Reason
			<< " targetType=" << (int)TargetLootableType
			<< " targetActor=" << (TargetLootable ? TargetLootable->Name.ToString().c_str() : "None")
			<< " targetLoc=" << TargetLocation.X << "," << TargetLocation.Y << "," << TargetLocation.Z
			<< " enemy=" << (CurrentEnemy ? CurrentEnemy->Name.ToString().c_str() : "None")
			<< " moveGoal=" << LastGoalLocation.X << "," << LastGoalLocation.Y << "," << LastGoalLocation.Z;

		if (PC)
		{
			Stream
				<< " pathComp=" << (PC->PathFollowingComponent ? 1 : 0)
				<< " brainComp=" << (PC->BrainComponent ? 1 : 0);

			if (PC->PathFollowingComponent)
			{
				Stream
					<< " pathActive=" << (PC->PathFollowingComponent->bIsActive ? 1 : 0)
					<< " hasNavData=" << (PC->PathFollowingComponent->MyNavData ? 1 : 0);
			}

			if (PC->BrainComponent)
				Stream << " brainActive=" << (PC->BrainComponent->bIsActive ? 1 : 0);
		}

		DebugLog(Stream.str(), bForce);
	}

	void Run()
	{
		if (!PlayerState || !PlayerState->AbilitySystemComponent)
			return;

		auto* ASC = PlayerState->AbilitySystemComponent;
		for (int32 i = 0; i < ASC->ActivatableAbilities.Items.Num(); ++i)
		{
			auto& Spec = ASC->ActivatableAbilities.Items[i];
			if (!Spec.Ability || !Spec.Ability->IsA(UFortGameplayAbility_Sprint::StaticClass()))
				continue;
			if (Spec.ActivationInfo.PredictionKeyWhenActivated.bIsStale)
				continue;
			if (!ASC->CanActivateAbilityWithMatchingTag(Spec.Ability->AbilityTags))
				continue;

			ASC->ServerTryActivateAbility(Spec.Handle, Spec.InputPressed, Spec.ActivationInfo.PredictionKeyWhenActivated);
			break;
		}
	}

	void GiveSiphon(AFortPlayerPawnAthena* KillerPawn)
	{
		if (!KillerPawn) return;

		float Health = KillerPawn->GetHealth();
		float Shield = KillerPawn->GetShield();

		if (Health < 0.f || Health > 100.f) return;
		if (Shield < 0.f || Shield > 100.f) return;

		if (Health >= 100.f)
		{
			Shield += 50.f;
		}
		else if (Health + 50.f > 100.f)
		{
			auto Overflow = (Health + 50.f) - 100.f;
			Health = 100.f;
			Shield += Overflow;
		}
		else
		{
			Health += 50.f;
		}

		if (Shield > 100.f) Shield = 100.f;

		KillerPawn->SetHealth(Health);
		KillerPawn->SetShield(Shield);
	}

	void AwardKillCredit(AFortPlayerControllerAthena* KillerPC, AFortPlayerStateAthena* KillerState, AFortPlayerPawnAthena* KillerPawn)
	{
		if (!KillerPC || !KillerState || !PlayerState || !PC)
			return;

		if (KillerState == PlayerState || KillerPC == (AFortPlayerControllerAthena*)PC)
			return;

		KillerState->KillScore++;
		KillerState->OnRep_Kills();

		if (KillerState->PlayerTeam)
		{
			for (int32 i = 0; i < KillerState->PlayerTeam->TeamMembers.Num(); ++i)
			{
				auto* TeamMemberController = KillerState->PlayerTeam->TeamMembers[i];
				if (!TeamMemberController || !TeamMemberController->PlayerState)
					continue;

				auto* TeamMember = (AFortPlayerStateAthena*)TeamMemberController->PlayerState;
				if (!TeamMember)
					continue;

				TeamMember->TeamKillScore++;
				TeamMember->OnRep_TeamKillScore();
			}
		}
		else
		{
			KillerState->TeamKillScore++;
			KillerState->OnRep_TeamKillScore();
		}

		KillerState->ClientReportKill(PlayerState);
		KillerState->ClientReportTeamKill(KillerState->KillScore);

		if (KillerPC->MatchReport)
			KillerPC->MatchReport->MatchStats.Stats[3] = KillerState->KillScore;
	}

	void DropInventory()
	{
		if (!PC || !PC->Inventory || !Pawn)
			return;

		for (int32 i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); ++i)
		{
			auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
			if (!Entry.ItemDefinition) continue;
			if (Entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>() || Entry.ItemDefinition->IsA<UFortAmmoItemDefinition>())
				continue;

			Inventory::SpawnPickup(Pawn->K2_GetActorLocation(), Entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, Pawn);
		}
	}

	void OnDied()
	{
		if (bIsDead)
			return;

		bIsDead = true;

		if (!PlayerState || !Pawn)
			return;

		auto DeathContext = PlayerBots::ConsumeDeathContext((AFortAthenaAIBotController*)PC);
		auto* KillerPC = DeathContext.KillerPC;
		auto* KillerPS = DeathContext.KillerPlayerState;
		auto* KillerPawn = DeathContext.KillerPawn;
		auto* DamageCauserActor = DeathContext.DamageCauser;

		PlayerState->PawnDeathLocation = Pawn->K2_GetActorLocation();
		PlayerState->DeathInfo.bDBNO = Pawn->bWasDBNOOnDeath;
		PlayerState->DeathInfo.DeathLocation = PlayerState->PawnDeathLocation;
		PlayerState->DeathInfo.DeathTags = Pawn->DeathTags;
		PlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(PlayerState->DeathInfo.DeathTags, PlayerState->DeathInfo.bDBNO);
		PlayerState->DeathInfo.Distance = (KillerPawn && PlayerState->DeathInfo.DeathCause != EDeathCause::FallDamage) ? KillerPawn->GetDistanceTo(Pawn) : 0.f;
		PlayerState->DeathInfo.Downer = PlayerState->DeathInfo.bDBNO ? KillerPS : nullptr;
		PlayerState->DeathInfo.FinisherOrDowner = KillerPS;
		PlayerState->DeathInfo.bInitialized = true;
		PlayerState->OnRep_DeathInfo();

		DropInventory();

		if (KillerPC && KillerPS && KillerPawn)
		{
			AwardKillCredit(KillerPC, KillerPS, KillerPawn);
			GiveSiphon(KillerPawn);

			auto* GM = AFortGameModeAthena::Get();
			AFortWeapon* DamageCauser = nullptr;
			if (auto* Projectile = DamageCauserActor ? DamageCauserActor->Cast<AFortProjectileBase>() : nullptr)
				DamageCauser = Projectile->GetOwnerWeapon();
			else if (auto* Weapon = DamageCauserActor ? DamageCauserActor->Cast<AFortWeapon>() : nullptr)
				DamageCauser = Weapon;

			if (GM)
			{
				((void (*)(AFortGameModeAthena*, AFortPlayerControllerAthena*, APlayerState*, AFortPlayerPawn*, UFortWeaponItemDefinition*, EDeathCause, char))(ImageBase + 0x1ec8680))(
					GM,
					(AFortPlayerControllerAthena*)PC,
					KillerPS,
					KillerPawn,
					DamageCauser ? DamageCauser->WeaponData : nullptr,
					PlayerState->DeathInfo.DeathCause,
					0);
			}

			DebugLog(std::string("elim credit -> ") + KillerPS->GetPlayerName().ToString().c_str(), true);
		}
	}

	AActor* GetNearestPlayerActor() const
	{
		if (!Pawn || !PlayerState) return nullptr;

		auto* GameMode = AFortGameModeAthena::Get();
		if (!GameMode) return nullptr;

		AActor* NearestPlayer = nullptr;
		float   BestDistance = FLT_MAX;

		auto TryCandidate = [&](AActor* CandidateActor, AFortPlayerStateAthena* CandidateState)
			{
				if (!CandidateActor || CandidateActor == Pawn || !CandidateState) return;
				if (CandidateState->TeamIndex == PlayerState->TeamIndex) return;

				auto* CandidatePawn = CandidateActor->Cast<AFortPlayerPawnAthena>();
				if (CandidatePawn && (CandidatePawn->bIsDying || CandidatePawn->bIsDBNO)) return;

				auto Distance = UKismetMathLibrary::Vector_Distance(Pawn->K2_GetActorLocation(), CandidateActor->K2_GetActorLocation());
				if (Distance < BestDistance)
				{
					BestDistance = Distance;
					NearestPlayer = CandidateActor;
				}
			};

		for (int32 i = 0; i < GameMode->AlivePlayers.Num(); ++i)
		{
			auto* CandidatePC = (AFortPlayerControllerAthena*)GameMode->AlivePlayers[i];
			if (!CandidatePC || CandidatePC == (AFortPlayerControllerAthena*)PC || !CandidatePC->Pawn) continue;
			auto* CandidatePawn = (AFortPlayerPawnAthena*)CandidatePC->Pawn;
			auto* CandidateState = CandidatePC->PlayerState ? (AFortPlayerStateAthena*)CandidatePC->PlayerState : nullptr;
			if (!CandidatePawn) continue;
			TryCandidate(CandidatePawn, CandidateState);
		}

		for (int32 i = 0; i < GameMode->AliveBots.Num(); ++i)
		{
			auto* CandidatePC = (AFortPlayerControllerAthena*)GameMode->AliveBots[i];
			if (!CandidatePC || CandidatePC == (AFortPlayerControllerAthena*)PC || !CandidatePC->Pawn) continue;
			auto* CandidatePawn = (AFortPlayerPawnAthena*)CandidatePC->Pawn;
			auto* CandidateState = CandidatePC->PlayerState ? (AFortPlayerStateAthena*)CandidatePC->PlayerState : nullptr;
			if (!CandidatePawn) continue;
			TryCandidate(CandidatePawn, CandidateState);
		}

		return NearestPlayer;
	}

	void PickupNearbyItems(float Range = 300.f)
	{
		if (!Pawn || !PC) return;

		auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
		if (CurrentTime < NextNearbyPickupCollectTime)
			return;
		NextNearbyPickupCollectTime = CurrentTime + 0.75f;

		// Global cache must already be fresh (ensured by shitty tick before bot loop)
		for (auto* Pickup : GlobalPickupCache::Pickups)
		{
			if (!Pickup || Pickup->bHidden || Pickup->bPickedUp) continue;
			if (Pickup->GetDistanceTo(Pawn) > Range) continue;
			if (!Pickup->PrimaryPickupItemEntry.ItemDefinition) continue;
			if (Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA<UFortAmmoItemDefinition>()) continue;

			GiveItemBot(Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo);
			Pickup->PickupLocationData.bPlayPickupSound = true;
			Pickup->PickupLocationData.FlyTime = 0.3f;
			Pickup->PickupLocationData.ItemOwner = Pawn;
			Pickup->PickupLocationData.PickupGuid = Pickup->PrimaryPickupItemEntry.ItemGuid;
			Pickup->PickupLocationData.PickupTarget = Pawn;
			Pickup->OnRep_PickupLocationData();
			Pickup->bPickedUp = true;
			Pickup->OnRep_bPickedUp();
		}
	}

	void Pickup(AFortPickupAthena* Pickup)
	{
		if (!Pickup || !Pickup->PrimaryPickupItemEntry.ItemDefinition) return;
		if (Pickup->bPickedUp || Pickup->bHidden) return;

		GiveItemBot(Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo);
		Pickup->PickupLocationData.bPlayPickupSound = true;
		Pickup->PickupLocationData.FlyTime = 0.3f;
		Pickup->PickupLocationData.ItemOwner = Pawn;
		Pickup->PickupLocationData.PickupGuid = Pickup->PrimaryPickupItemEntry.ItemGuid;
		Pickup->PickupLocationData.PickupTarget = Pawn;
		Pickup->OnRep_PickupLocationData();
		Pickup->bPickedUp = true;
		Pickup->OnRep_bPickedUp();
	}

	void PickupAllItemsInRange(float Range = 320.f)
	{
		PickupNearbyItems(Range);
	}

	AFortPickupAthena* FindNearestPickup(float MaxRange = 2500.f)
	{
		if (!Pawn) return nullptr;

		// Global cache must already be fresh (ensured by Tick before bot loop)
		const auto& Pickups = GlobalPickupCache::Pickups;
		if (Pickups.empty())
		{
			PickupScanCursor = 0;
			return nullptr;
		}

		auto PawnLocation = Pawn->K2_GetActorLocation();
		auto MaxRangeSquared = MaxRange * MaxRange;
		const int32 TotalCount = (int32)Pickups.size();
		const int32 ScanCount = (TotalCount < 192) ? TotalCount : 192;

		PickupScanCursor = PickupScanCursor % TotalCount;
		int32 StartIndex = PickupScanCursor;

		AFortPickupAthena* NearestPickup = nullptr;
		float              NearestDistance = FLT_MAX;

		for (int32 Offset = 0; Offset < ScanCount; ++Offset)
		{
			int32  i = (StartIndex + Offset) % TotalCount;
			auto* Pickup = Pickups[i];

			if (!Pickup || Pickup->bHidden || Pickup->bPickedUp) continue;
			if (!Pickup->PrimaryPickupItemEntry.ItemDefinition) continue;
			if (Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA<UFortAmmoItemDefinition>()) continue;

			auto  PickupLocation = Pickup->K2_GetActorLocation();
			auto  Delta = PickupLocation - PawnLocation;
			auto  DistSq = (Delta.X * Delta.X) + (Delta.Y * Delta.Y) + (Delta.Z * Delta.Z);
			if (DistSq > MaxRangeSquared) continue;

			auto Dist = sqrtf(DistSq);
			if (!NearestPickup || Dist < NearestDistance)
			{
				NearestPickup = Pickup;
				NearestDistance = Dist;
			}
		}

		PickupScanCursor = (StartIndex + ScanCount) % TotalCount;
		return NearestPickup;
	}

	const FVector* FindNearestChestLocation(float MaxRange = 3500.f)
	{
		if (!Pawn) return nullptr;

		GlobalChestNodes::EnsureLoaded();
		const auto& Nodes = GlobalChestNodes::Nodes;

		if (Nodes.empty())
		{
			ChestScanCursor = 0;
			return nullptr;
		}

		auto PawnLocation = Pawn->K2_GetActorLocation();
		const int32 TotalCount = (int32)Nodes.size();
		const int32 ScanCount = (TotalCount < 128) ? TotalCount : 128;

		ChestScanCursor = ChestScanCursor % TotalCount;
		int32 StartIndex = ChestScanCursor;

		const FVector* NearestChestLocation = nullptr;
		float          NearestDistanceSquared = MaxRange * MaxRange;

		for (int32 Offset = 0; Offset < ScanCount; ++Offset)
		{
			int32 i = (StartIndex + Offset) % TotalCount;

			// FIXED
			if (IgnoredChestNodeIndices.count(i))
				continue;

			const auto& ChestLocation = Nodes[i].Location;
			auto Delta = ChestLocation - PawnLocation;
			auto DistSq = (Delta.X * Delta.X) + (Delta.Y * Delta.Y) + (Delta.Z * Delta.Z);
			if (DistSq > NearestDistanceSquared) continue;

			NearestChestLocation = &Nodes[i].Location;
			NearestDistanceSquared = DistSq;
		}

		ChestScanCursor = (StartIndex + ScanCount) % TotalCount;
		return NearestChestLocation;
	}

	ABuildingContainer* ResolveChestAtLocation(const FVector& ChestLocation, float SearchRadius = 450.f)
	{
		auto* World = UWorld::Get();
		if (!World || ChestLocation.IsZero()) return nullptr;

		static auto ChestClass = Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena.Tiered_Chest_Athena_C");
		static auto FactionChestClass = Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena_FactionChest.Tiered_Chest_Athena_FactionChest_C");

		ABuildingContainer* BestChest = nullptr;
		float               BestDistSq = SearchRadius * SearchRadius;

		auto SearchClass = [&](UClass* Class)
			{
				if (!Class) return;
				TArray<AActor*> Actors;
				UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(World, Class, &Actors);
				for (int32 i = 0; i < Actors.Num(); ++i)
				{
					auto* Container = Actors[i] ? Actors[i]->Cast<ABuildingContainer>() : nullptr;
					if (!Container || Container->bHidden || Container->bAlreadySearched) continue;

					auto DistSq = UKismetMathLibrary::Vector_DistanceSquared(Container->K2_GetActorLocation(), ChestLocation);
					if (DistSq <= BestDistSq)
					{
						BestChest = Container;
						BestDistSq = DistSq;
					}
				}
				Actors.Free();
			};

		SearchClass(ChestClass);
		SearchClass(FactionChestClass);
		return BestChest;
	}

	void SearchLoot()
	{
		if (!Pawn || !PC) return;

		auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());

		// Gated pickup scan — the gate is inside PickupNearbyItems itself
		PickupNearbyItems();

		// Validate current chest target
		if (TargetLootableType == ELootableType::Chest)
		{
			if (auto* Container = TargetLootable ? TargetLootable->Cast<ABuildingContainer>() : nullptr)
			{
				if (Container->bAlreadySearched || Container->bHidden)
				{
					IgnoreChestLocation(TargetLootLocation.IsZero() ? Container->K2_GetActorLocation() : TargetLootLocation);
					ClearLootTarget();
				}
			}
		}
		else if (TargetLootable && TargetLootable->bHidden)
		{
			ClearLootTarget();
		}

		// Try to resolve chest actor from stored location
		if (!TargetLootable && TargetLootableType == ELootableType::Chest && !TargetLootLocation.IsZero())
		{
			auto DistanceToChest = UKismetMathLibrary::Vector_Distance(Pawn->K2_GetActorLocation(), TargetLootLocation);
			if (DistanceToChest <= 500.f)
			{
				TargetLootable = ResolveChestAtLocation(TargetLootLocation);
				if (!TargetLootable && DistanceToChest <= 225.f)
				{
					IgnoreChestLocation(TargetLootLocation);
					ClearLootTarget();
					return;
				}
			}
		}

		// Find a new chest target
		if (!TargetLootable && TargetLootLocation.IsZero())
		{
			if (const auto* ChestLocation = FindNearestChestLocation())
			{
				TargetLootLocation = *ChestLocation;
				TargetLootableType = ELootableType::Chest;
				LootTargetStartTime = 0.f;
				TimeToNextAction = 0.f;
			}
		}

		auto TargetLocation = !TargetLootLocation.IsZero() ? TargetLootLocation : FVector();
		if (TargetLootable)
			TargetLocation = TargetLootable->K2_GetActorLocation();

		if (TargetLocation.IsZero())
			return;

		auto Distance = UKismetMathLibrary::Vector_Distance(Pawn->K2_GetActorLocation(), TargetLocation);
		if (LootTargetStartTime == 0.f)
		{
			LootTargetStartTime = CurrentTime;
			LastLootTargetDistance = Distance;
		}
		else if ((CurrentTime - LootTargetStartTime) > 8.f && Distance > LastLootTargetDistance - 100.f)
		{
			if (TargetLootableType == ELootableType::Chest)
				IgnoreChestLocation(TargetLocation);
			ClearLootTarget();
			return;
		}

		LastLootTargetDistance = Distance;
		auto LookRotation = UKismetMathLibrary::FindLookAtRotation(Pawn->K2_GetActorLocation(), TargetLocation);
		PC->SetControlRotation(LookRotation);
		PC->K2_SetActorRotation(LookRotation, true);

		if (TargetLootable) LookAt(TargetLootable);
		else                LookAt(nullptr);

		if (TargetLootableType == ELootableType::Pickup || (TargetLootable && TargetLootable->IsA<AFortPickupAthena>()))
		{
			MoveToActorOnNav(TargetLootable, 65.f, true);
			if (Distance < 150.f)
			{
				if (auto* TargetPickup = TargetLootable->Cast<AFortPickupAthena>())
					Pickup(TargetPickup);
				PickupAllItemsInRange(250.f);
				ClearLootTarget();
				SimpleSwitchToWeapon();
			}
			return;
		}

		if (!TargetLootable)
		{
			MoveToLocationOnNav(TargetLocation, 110.f, true);
			return;
		}

		MoveToActorOnNav(TargetLootable, 75.f, true);
		if (Distance < 300.f)
		{
			if (!IsPickaxeEquipped())
				EquipPickaxe();

			if (TimeToNextAction == 0.f)
			{
				TimeToNextAction = CurrentTime;
				Pawn->PawnStopFire(0);
				Pawn->bStartedInteractSearch = true;
				Pawn->OnRep_StartedInteractSearch();
				return;
			}

			if ((CurrentTime - TimeToNextAction) < 1.25f)
				return;

			if (auto* Container = TargetLootable->Cast<ABuildingContainer>())
			{
				Container->BP_SpawnLoot(Pawn);
				Container->bAlreadySearched = true;
				Container->OnRep_bAlreadySearched();
				Container->BounceContainer();
				TargetLootable->bHidden = true;
			}

			Pawn->PawnStopFire(0);
			Pawn->bStartedInteractSearch = false;
			Pawn->OnRep_StartedInteractSearch();
			IgnoreChestLocation(TargetLootLocation.IsZero() ? TargetLootable->K2_GetActorLocation() : TargetLootLocation);
			ClearLootTarget();
			PickupAllItemsInRange(350.f);
			SimpleSwitchToWeapon();
		}
	}

	bool CanFire() const
	{
		if (!Pawn || !Pawn->CurrentWeapon || !Pawn->CurrentWeapon->WeaponData)
			return false;
		if (Pawn->CurrentWeapon->WeaponData->IsA<UFortWeaponMeleeItemDefinition>())
			return false;
		if (Pawn->CurrentWeapon->AmmoCount <= 0 &&
			!Pawn->CurrentWeapon->WeaponData->bUsesPhantomReserveAmmo)
			return false;
		return true;
	}

	void ForceStrafe()

	{
		if (!Pawn) return;

		auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
		if (!bIsCurrentlyStrafing)
		{
			bIsCurrentlyStrafing = true;
			StrafeType = UKismetMathLibrary::RandomBool() ? EPlayerBotStrafeType::Left : EPlayerBotStrafeType::Right;
			StrafeEndTime = CurrentTime + UKismetMathLibrary::RandomFloatInRange(1.0f, 2.5f);
		}

		if (CurrentTime < StrafeEndTime)
		{
			auto RightVector = Pawn->GetActorRightVector();
			Pawn->AddMovementInput(StrafeType == EPlayerBotStrafeType::Left ? (RightVector * -1.f) : RightVector, 1.f, true);
		}
		else
		{
			bIsCurrentlyStrafing = false;
		}
	}
};

inline std::vector<PlayerBot*> PlayerBotArray{};

namespace PlayerBots
{
	inline void SetBotToBus(PlayerBot* Bot);
	inline void SetBotsToBus();
	inline void ForceJumpFromBus(PlayerBot* Bot);

	inline FVector GetBotGoalLocation(PlayerBot* Bot)
	{
		if (!Bot || !Bot->Pawn) return FVector();

		switch (Bot->State)
		{
		case EPlayerBotState::Skydiving:
		case EPlayerBotState::Gliding:
			return Bot->TargetDropZone;
		case EPlayerBotState::Looting:
			return Bot->TargetLootable ? Bot->TargetLootable->K2_GetActorLocation() : Bot->TargetLootLocation;
		case EPlayerBotState::LookingForPlayers:
			return Bot->CurrentEnemy ? Bot->CurrentEnemy->K2_GetActorLocation()
				: (Bot->NearestPlayerActor ? Bot->NearestPlayerActor->K2_GetActorLocation() : FVector());
		case EPlayerBotState::MovingToSafeZone:
		{
			auto* GameState = AFortGameStateAthena::Get();
			return (GameState && GameState->SafeZoneIndicator) ? GameState->SafeZoneIndicator->NextCenter : FVector();
		}
		default:
			return FVector();
		}
	}

	inline void UpdateBotEvaluator(PlayerBot* Bot)
	{
		if (!Bot || !Bot->Pawn || !Bot->PC) return;
		if (Bot->State < EPlayerBotState::Landed) return;

		auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
		auto CurrentLocation = Bot->Pawn->K2_GetActorLocation();

		bool bCurrentTargetValid = false;
		if (Bot->TargetLootableType == ELootableType::Chest)
		{
			bCurrentTargetValid = !Bot->TargetLootLocation.IsZero();
			if (auto* Container = Bot->TargetLootable ? Bot->TargetLootable->Cast<ABuildingContainer>() : nullptr)
			{
				if (Container->bAlreadySearched || Container->bHidden)
				{
					Bot->IgnoreChestLocation(Bot->TargetLootLocation.IsZero() ? Container->K2_GetActorLocation() : Bot->TargetLootLocation);
					Bot->ClearLootTarget();
					bCurrentTargetValid = false;
				}
			}
		}
		else if (Bot->TargetLootable && !Bot->TargetLootable->bHidden)
		{
			bCurrentTargetValid = true;
		}

		if (!bCurrentTargetValid || CurrentTime >= Bot->NextLootScanTime)
		{
			Bot->NextLootScanTime = CurrentTime + (bCurrentTargetValid ? 3.5f : 1.5f);

			AActor* NearestLootable = nullptr;
			ELootableType NearestType = ELootableType::None;
			FVector       NearestLootLocation = FVector();
			float         BestDistance = FLT_MAX;

			if (const auto* NearestChestLocation = Bot->FindNearestChestLocation())
			{
				auto Distance = UKismetMathLibrary::Vector_Distance(CurrentLocation, *NearestChestLocation);
				if (Distance < BestDistance)
				{
					NearestLootable = nullptr;
					NearestType = ELootableType::Chest;
					NearestLootLocation = *NearestChestLocation;
					BestDistance = Distance;
				}
			}

			if (!bCurrentTargetValid)
			{
				Bot->TargetLootable = NearestLootable;
				Bot->TargetLootableType = NearestType;
				Bot->TargetLootLocation = NearestType == ELootableType::Chest ? NearestLootLocation : FVector();
			}
			else if (NearestType == ELootableType::Chest &&
				Bot->TargetLootableType == ELootableType::Chest &&
				!NearestLootLocation.IsZero() &&
				UKismetMathLibrary::Vector_DistanceSquared(Bot->TargetLootLocation, NearestLootLocation) > 400.f)
			{
				auto CurrentDistance = UKismetMathLibrary::Vector_Distance(CurrentLocation, Bot->TargetLootLocation);
				auto NewDistance = UKismetMathLibrary::Vector_Distance(CurrentLocation, NearestLootLocation);
				if (NewDistance + 250.f < CurrentDistance)
				{
					Bot->TargetLootable = nullptr;
					Bot->TargetLootableType = ELootableType::Chest;
					Bot->TargetLootLocation = NearestLootLocation;
					Bot->LootTargetStartTime = 0.f;
					Bot->TimeToNextAction = 0.f;
				}
			}
		}

		if (CurrentTime >= Bot->NextPlayerScanTime)
		{
			Bot->NextPlayerScanTime = CurrentTime + 1.0f;
			Bot->NearestPlayerActor = Bot->GetNearestPlayerActor();
			Bot->PC->K2_ClearFocus();
		}

		if (Bot->Pawn->bIsCrouched && (Bot->TickCounter % 30) == 0)
			Bot->Pawn->UnCrouch(false);

		Bot->bIsStressed = Bot->Pawn->GetHealth() <= 75.f;

		if (Bot->bIsCurrentlyStrafing)
			Bot->ForceStrafe();

		auto Velocity = Bot->Pawn->GetVelocity();
		auto HorizontalSpeed = sqrtf((Velocity.X * Velocity.X) + (Velocity.Y * Velocity.Y));
		if ((Bot->TickCounter % 90) == 0 && HorizontalSpeed >= 100.f && Bot->State >= EPlayerBotState::Landed)
			Bot->Run();

		if (Bot->Pawn->bIsInWaterVolume)
		{
			Bot->LastProgressTime = CurrentTime;
			Bot->LastStuckCheckLocation = CurrentLocation;
			Bot->LastGoalLocation = GetBotGoalLocation(Bot);
			if (Bot->State == EPlayerBotState::Stuck)
				Bot->State = Bot->CachedState;
			return;
		}

		if (Bot->State >= EPlayerBotState::Looting && Bot->State != EPlayerBotState::Stuck)
		{
			auto GoalLocation = GetBotGoalLocation(Bot);
			auto BotVelocity = Bot->Pawn->GetVelocity();
			auto HSpeed = sqrtf((BotVelocity.X * BotVelocity.X) + (BotVelocity.Y * BotVelocity.Y));
			auto DistanceMoved = Bot->LastStuckCheckLocation.IsZero() ? 99999.f : UKismetMathLibrary::Vector_Distance(CurrentLocation, Bot->LastStuckCheckLocation);
			auto GoalDistance = GoalLocation.IsZero() ? 0.f : UKismetMathLibrary::Vector_Distance(CurrentLocation, GoalLocation);
			auto GoalChanged = Bot->LastGoalLocation.IsZero() ? true : UKismetMathLibrary::Vector_Distance(GoalLocation, Bot->LastGoalLocation) > 250.f;

			if (GoalChanged || DistanceMoved > 180.f || HSpeed > 120.f || GoalDistance < 250.f)
			{
				Bot->LastProgressTime = CurrentTime;
				Bot->LastStuckCheckLocation = CurrentLocation;
				Bot->LastGoalLocation = GoalLocation;
			}
			else if (!GoalLocation.IsZero() && (CurrentTime - Bot->LastProgressTime) > 3.f)
			{
				Bot->CachedState = Bot->State;
				Bot->State = EPlayerBotState::Stuck;
				Bot->PC->StopMovement();
				Bot->LastProgressTime = CurrentTime;
			}
		}
	}

	struct DropZoneState
	{
		FVector              CurrentGroupDropZone{};
		int                  RemainingBotsForCurrentDropZone = 0;
		std::vector<FVector> ActiveConfiguredDropZones{};
	};
	static DropZoneState GDropZoneState{};

	inline void ResetDropZoneState()
	{
		GDropZoneState = DropZoneState{};
	}

	inline void ChooseDropZone(PlayerBot* Bot)
	{
		if (!Bot) return;

		if (Bot->bUseConfiguredDropZone && !PlayerBotDropZoneLocations.empty())
		{
			if (GDropZoneState.ActiveConfiguredDropZones.empty())
			{
				auto ShuffledDropZones = PlayerBotDropZoneLocations;
				for (int i = (int)ShuffledDropZones.size() - 1; i > 0; --i)
				{
					auto SwapIndex = UKismetMathLibrary::RandomIntegerInRange(0, i);
					auto Temp = ShuffledDropZones[i];
					ShuffledDropZones[i] = ShuffledDropZones[SwapIndex];
					ShuffledDropZones[SwapIndex] = Temp;
				}

				auto ActiveDropZoneCount = UKismetMathLibrary::RandomIntegerInRange(2, 4);
				for (int i = 0; i < ActiveDropZoneCount && i < (int)ShuffledDropZones.size(); ++i)
					GDropZoneState.ActiveConfiguredDropZones.push_back(ShuffledDropZones[i]);
			}

			if (GDropZoneState.RemainingBotsForCurrentDropZone <= 0 || GDropZoneState.CurrentGroupDropZone.IsZero())
			{
				const int PoolSize = (int)GDropZoneState.ActiveConfiguredDropZones.size();
				if (PoolSize > 0)
				{
					GDropZoneState.CurrentGroupDropZone = GDropZoneState.ActiveConfiguredDropZones[UKismetMathLibrary::RandomIntegerInRange(0, PoolSize - 1)];
					GDropZoneState.RemainingBotsForCurrentDropZone = UKismetMathLibrary::RandomIntegerInRange(1, 3);
				}
			}

			Bot->TargetDropZone = GDropZoneState.CurrentGroupDropZone;
			Bot->TargetDropZone.X += UKismetMathLibrary::RandomFloatInRange(-2500.f, 2500.f);
			Bot->TargetDropZone.Y += UKismetMathLibrary::RandomFloatInRange(-2500.f, 2500.f);
			Bot->TargetDropZone.Z += UKismetMathLibrary::RandomFloatInRange(-100.f, 100.f);
			GDropZoneState.RemainingBotsForCurrentDropZone--;
			return;
		}

		auto PlayerStarts = Utils::GetAll<AFortPlayerStart>();
		if (PlayerStarts.Num() <= 0) return;

		auto* Start = PlayerStarts[UKismetMathLibrary::RandomIntegerInRange(0, PlayerStarts.Num() - 1)];
		if (Start)
		{
			Bot->TargetDropZone = Start->K2_GetActorLocation();
			Bot->TargetDropZone.X += UKismetMathLibrary::RandomFloatInRange(-15000.f, 15000.f);
			Bot->TargetDropZone.Y += UKismetMathLibrary::RandomFloatInRange(-15000.f, 15000.f);
		}
		PlayerStarts.Free();
	}

	inline void EnsureCosmeticPools()
	{
		if (!PlayerBotCIDs.empty()) return;

		auto CIDs = Utils::GetAllObjectsOfClass<UAthenaCharacterItemDefinition>();
		auto Pickaxes = Utils::GetAllObjectsOfClass<UAthenaPickaxeItemDefinition>();
		auto Backpacks = Utils::GetAllObjectsOfClass<UAthenaBackpackItemDefinition>();
		auto Gliders = Utils::GetAllObjectsOfClass<UAthenaGliderItemDefinition>();
		auto Contrails = Utils::GetAllObjectsOfClass<UAthenaSkyDiveContrailItemDefinition>();
		auto Dances = Utils::GetAllObjectsOfClass<UAthenaDanceItemDefinition>();

		for (auto* Item : CIDs)
		{
			if (!Item || !Item->HeroDefinition) continue;
			bool bHasUsableParts = false;
			for (auto& Specialization : Item->HeroDefinition->Specializations)
			{
				if (Specialization && Specialization->CharacterParts.Num() > 0)
				{
					bHasUsableParts = true;
					break;
				}
			}
			if (!bHasUsableParts) continue;
			PlayerBotCIDs.push_back(Item);
		}

		for (auto* Item : Pickaxes) { if (Item && Item->WeaponDefinition) PlayerBotPickaxes.push_back(Item); }
		for (auto* Item : Backpacks) { if (Item) PlayerBotBackpacks.push_back(Item); }
		for (auto* Item : Gliders) { if (Item) PlayerBotGliders.push_back(Item); }
		for (auto* Item : Contrails) { if (Item) PlayerBotContrails.push_back(Item); }
		for (auto* Item : Dances) { if (Item) PlayerBotDances.push_back(Item); }
	}

	inline void WakeAI(PlayerBot* Bot)
	{
		if (!Bot || !Bot->PC) return;

		auto* World = UWorld::Get();
		if (!World) return;

		auto CurrentTime = UGameplayStatics::GetTimeSeconds(World);
		if (Bot->bWakeAIInitialized || CurrentTime < Bot->NextWakeAITime) return;

		Bot->NextWakeAITime = CurrentTime + 1.0f;
		++Bot->WakeAIAttemptCount;

		auto* NavSystem = (UAthenaNavSystem*)World->NavigationSystem;
		if (!NavSystem || !NavSystem->MainNavData)
		{
			if (!Bot->bLoggedMissingNavData)
			{
				Bot->bLoggedMissingNavData = true;
				Bot->DebugLog("WakeAI deferred: nav data not ready", true);
			}
			return;
		}

		if (!Bot->PC->PathFollowingComponent)
		{
			if (!Bot->bLoggedMissingPathComponent)
			{
				Bot->bLoggedMissingPathComponent = true;
				Bot->DebugLog("WakeAI deferred: missing PathFollowingComponent", true);
			}
			return;
		}

		Bot->PC->PathFollowingComponent->MyNavData = NavSystem->MainNavData;
		Bot->PC->PathFollowingComponent->Activate(true);
		Bot->PC->PathFollowingComponent->SetActive(true, true);
		Bot->PC->PathFollowingComponent->OnRep_IsActive();

		if (!Bot->PC->BrainComponent)
		{
			Bot->PC->BrainComponent = (UBrainComponent*)UGameplayStatics::SpawnObject(UBrainComponent::StaticClass(), Bot->PC);
			if (!Bot->PC->BrainComponent)
			{
				if (!Bot->bLoggedMissingBrainComponent)
				{
					Bot->bLoggedMissingBrainComponent = true;
					Bot->DebugLog("WakeAI deferred: failed to create BrainComponent", true);
				}
				return;
			}
		}

		Bot->PC->BrainComponent->Activate(false);
		Bot->PC->BrainComponent->SetActive(true, false);
		Bot->PC->BrainComponent->OnRep_IsActive();
		Bot->PC->BrainComponent->RestartLogic();

		Bot->bWakeAIInitialized = true;
		Bot->DebugSnapshot("WakeAI initialized");
	}

	inline void SpawnPlayerBots(AActor* SpawnLocator)
	{
		if (!Globals::bBotsEnabled || !SpawnLocator) return;

		auto* GameMode = AFortGameModeAthena::Get();
		if (!GameMode || !GameMode->ServerBotManager || !GameMode->ServerBotManager->CachedBotMutator) return;

		EnsureCosmeticPools();

		static auto BotBP = Utils::FindObject<UClass>("/Game/Athena/AI/Phoebe/BP_PlayerPawn_Athena_Phoebe.BP_PlayerPawn_Athena_Phoebe_C");
		static auto BehaviorTree = Utils::FindObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");
		static auto DefaultCID = Utils::FindObject<UAthenaCharacterItemDefinition>("/Game/Athena/Items/Cosmetics/Characters/CID_001_Athena_Commando_F_Default.CID_001_Athena_Commando_F_Default");
		static auto DefaultPickaxe = Utils::FindObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		static auto MANGAnimBP = Utils::FindObject<UClass>("/Game/Athena/AI/MANG/AnimSet/MANG_PatrolLayerAnimBP.MANG_PatrolLayerAnimBP_C");

		if (!BotBP || !BehaviorTree) return;

		auto* BotMutator = (AFortAthenaMutator_Bots*)GameMode->ServerBotManager->CachedBotMutator;
		auto* Pawn = BotMutator->SpawnBot(BotBP, SpawnLocator, SpawnLocator->K2_GetActorLocation(), SpawnLocator->K2_GetActorRotation(), false);
		if (!Pawn || !Pawn->Controller) return;

		auto* PC = Pawn->Controller ? Pawn->Controller->Cast<ABP_PhoebePlayerController_C>() : nullptr;
		auto* PlayerState = PC && PC->PlayerState ? PC->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
		if (!PC || !PlayerState) return;

		auto* Bot = new PlayerBot{};
		Bot->Pawn = Pawn;
		Bot->PC = PC;
		Bot->PlayerState = PlayerState;
		Bot->LastLoggedState = Bot->State;
		Bot->bUseConfiguredDropZone = UKismetMathLibrary::RandomBoolWithWeight(0.7f);
		Bot->EnsureInventory();
		Bot->DebugSnapshot("Spawned bot!!!!!!!!!!!!!!!!!!");
		
		

		UAthenaCharacterItemDefinition* SelectedCID = DefaultCID;
		if (!PlayerBotCIDs.empty())
		{
			auto* RandomCID = PlayerBotCIDs[rand() % PlayerBotCIDs.size()];
			if (RandomCID && RandomCID->HeroDefinition)
				SelectedCID = RandomCID;
		}

		FFortAthenaLoadout BotLoadout{};
		BotLoadout.Character = SelectedCID;
		if (!PlayerBotBackpacks.empty()) BotLoadout.Backpack = PlayerBotBackpacks[rand() % PlayerBotBackpacks.size()];
		if (!PlayerBotGliders.empty())   BotLoadout.Glider = PlayerBotGliders[rand() % PlayerBotGliders.size()];
		if (!PlayerBotContrails.empty()) BotLoadout.SkyDiveContrail = PlayerBotContrails[rand() % PlayerBotContrails.size()];
		for (auto* Dance : PlayerBotDances) BotLoadout.Dances.Add(Dance);

		PC->CosmeticLoadoutBC = BotLoadout;

		xmap<EFortCustomPartType, UCustomCharacterPart*> VariantOverrides;
		for (auto& CVC : BotLoadout.CharacterVariantChannels)
		{
			auto* CosmeticDef = (UAthenaCosmeticItemDefinition*)CVC.ItemVariantIsUsedFor;
			if (!CosmeticDef) continue;

			FPartVariantDef* MatchingVariant = nullptr;
			auto Variant = CosmeticDef->ItemVariants.Search([&CVC, &MatchingVariant](UFortCosmeticVariant* Variant) mutable
				{
					if (auto* CharacterPartVariant = Variant ? Variant->Cast<UFortCosmeticCharacterPartVariant>() : nullptr)
					{
						MatchingVariant = CharacterPartVariant->PartOptions.Search([&CVC](FPartVariantDef& Def)
							{ return CVC.ActiveVariantTag.TagName == Def.CustomizationVariantTag.TagName; });
						return MatchingVariant != nullptr;
					}
					return false;
				});

			if (!Variant || !MatchingVariant) continue;

			for (auto& VariantPart : MatchingVariant->VariantParts)
				if (VariantPart) VariantOverrides[VariantPart->CharacterPartType] = VariantPart;
		}

		auto ApplyHeroParts = [&](UAthenaCharacterItemDefinition* CharacterDef) -> int
			{
				int AppliedParts = 0;
				if (!CharacterDef || !CharacterDef->HeroDefinition) return AppliedParts;

				PlayerState->HeroType = CharacterDef->HeroDefinition;
				for (auto& Specialization : CharacterDef->HeroDefinition->Specializations)
				{
					if (!Specialization) continue;
					for (auto& Part : Specialization->CharacterParts)
					{
						if (!Part) continue;
						auto PartOverride = VariantOverrides.contains(Part->CharacterPartType) ? VariantOverrides[Part->CharacterPartType] : Part;
						if (!PartOverride) continue;
						Pawn->ServerChoosePart(PartOverride->CharacterPartType, PartOverride);
						++AppliedParts;
					}
				}
				return AppliedParts;
			};

		int AppliedCharacterParts = ApplyHeroParts(SelectedCID);
		if (AppliedCharacterParts <= 0 && DefaultCID && DefaultCID->HeroDefinition)
		{
			BotLoadout.Character = DefaultCID;
			PC->CosmeticLoadoutBC = BotLoadout;
			AppliedCharacterParts = ApplyHeroParts(DefaultCID);
		}

		Pawn->CosmeticLoadout = BotLoadout;

		if (BotLoadout.Backpack)
		{
			for (auto& Part : BotLoadout.Backpack->CharacterParts)
				if (Part) Pawn->ServerChoosePart(Part->CharacterPartType, Part);
		}

		Pawn->OnRep_CosmeticLoadout();

		if (MANGAnimBP && Pawn->Mesh)
		{
			Pawn->Mesh->AnimBlueprintGeneratedClass = MANGAnimBP;
			Pawn->Mesh->AnimClass = MANGAnimBP;
			Pawn->OnRep_AnimBPOverride();
		}

		UFortWeaponItemDefinition* PickaxeDef = DefaultPickaxe;
		if (!PlayerBotPickaxes.empty())
		{
			auto* Pickaxe = PlayerBotPickaxes[rand() % PlayerBotPickaxes.size()];
			if (Pickaxe && Pickaxe->WeaponDefinition) PickaxeDef = Pickaxe->WeaponDefinition;
		}
		if (PickaxeDef)
		{
			Bot->GiveItemBot(PickaxeDef);
			Bot->EquipPickaxe();
		}

		static auto AR = Utils::FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_C_Ore_T02.WID_Assault_Auto_Athena_C_Ore_T02");
		static auto SG = Utils::FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03");
		static auto LightAmmo = Utils::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
		static auto MediumAmmo = Utils::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
		static auto HeavyAmmo = Utils::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
		static auto Shells = Utils::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
		static auto Wood = Utils::FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
		static auto Stone = Utils::FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		static auto Metal = Utils::FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

		if (AR) Bot->GiveItemBot(AR, 1, 1150); // ignore
		if (SG) Bot->GiveItemBot(SG, 1, 3);
		if (LightAmmo) Bot->GiveItemBot(LightAmmo, 50);
		if (MediumAmmo) Bot->GiveItemBot(MediumAmmo, 40);
		if (HeavyAmmo) Bot->GiveItemBot(HeavyAmmo, 10);
		if (Shells) Bot->GiveItemBot(Shells, 6);
		if (Wood) Bot->GiveItemBot(Wood, 50);
		if (Stone) Bot->GiveItemBot(Stone, 50);
		if (Metal) Bot->GiveItemBot(Metal, 50);
	
		for (auto& Item : GameMode->StartingItems)
		{
			if (Item.Item)
				Bot->GiveItemBot(Item.Item, Item.Count);
		}
		Bot->EquipGun();
		

		for (auto* SkillSet : PC->DigestedBotSkillSets)
		{
			if (!SkillSet) continue;
			if (auto* Aiming = SkillSet->Cast<UFortAthenaAIBotAimingDigestedSkillSet>())     PC->CacheAimingDigestedSkillSet = Aiming;
			if (auto* Attacking = SkillSet->Cast<UFortAthenaAIBotAttackingDigestedSkillSet>())  PC->CacheAttackingSkillSet = Attacking;
			if (auto* Harvest = SkillSet->Cast<UFortAthenaAIBotHarvestDigestedSkillSet>())    PC->CacheHarvestDigestedSkillSet = Harvest;
			if (auto* InvSet = SkillSet->Cast<UFortAthenaAIBotInventoryDigestedSkillSet>())  PC->CacheInventoryDigestedSkillSet = InvSet;
			if (auto* LootingSet = SkillSet->Cast<UFortAthenaAIBotLootingDigestedSkillSet>())    PC->CacheLootingSkillSet = LootingSet;
			if (auto* Movement = SkillSet->Cast<UFortAthenaAIBotMovementDigestedSkillSet>())   PC->CacheMovementSkillSet = Movement;
			if (auto* Perception = SkillSet->Cast<UFortAthenaAIBotPerceptionDigestedSkillSet>()) PC->CachePerceptionDigestedSkillSet = Perception;
			if (auto* Playstyle = SkillSet->Cast<UFortAthenaAIBotPlayStyleDigestedSkillSet>())  PC->CachePlayStyleSkillSet = Playstyle;
		}

		PC->BehaviorTree = BehaviorTree;
		PC->RunBehaviorTree(BehaviorTree);
		PC->UseBlackboard(BehaviorTree->BlackboardAsset, &PC->Blackboard);
		PC->UseBlackboard(BehaviorTree->BlackboardAsset, &PC->Blackboard1);
		PC->OnUsingBlackBoard(PC->Blackboard, BehaviorTree->BlackboardAsset);
		PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhaseStep")), (uint8)EAthenaGamePhaseStep::Warmup);
		PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhase")), (uint8)EAthenaGamePhase::Warmup);
		PC->Blackboard->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_IsMovementBlocked")), false);

		WakeAI(Bot);

		Pawn->CapsuleComponent->SetGenerateOverlapEvents(true);
		Pawn->CharacterMovement->bCanWalkOffLedges = true;
		Pawn->SetMaxHealth(100);
		Pawn->SetHealth(100);
		Pawn->SetMaxShield(100);
		Pawn->SetShield(0);
		PC->Skill = 3000.f;
		PlayerBotArray.push_back(Bot);

		auto* GameState = AFortGameStateAthena::Get();
		if (GameState)
		{
			if (GameState->GetAircraft(0) || GameState->GamePhase == EAthenaGamePhase::Aircraft)
				SetBotToBus(Bot);
			else if (GameState->GamePhase > EAthenaGamePhase::Aircraft)
			{
				SetBotToBus(Bot);
				ForceJumpFromBus(Bot);
			}
		}
	}

	inline void SetBotToBus(PlayerBot* Bot)
	{
		if (!Bot || !Bot->PC || !Bot->PC->Blackboard || Bot->bHasJumpedFromBus) return;

		Bot->State = EPlayerBotState::Bus;
		Bot->bLoggedBusState = false;
		Bot->ClosestDistToDropZone = FLT_MAX;
		if (Bot->TargetDropZone.IsZero()) ChooseDropZone(Bot);

		Bot->PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhaseStep")), (uint8)EAthenaGamePhaseStep::BusFlying);
		Bot->PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhase")), (uint8)EAthenaGamePhase::Aircraft);
		Bot->EquipPickaxe();
	}

	inline void SetBotsToBus()
	{
		for (auto* Bot : PlayerBotArray) SetBotToBus(Bot);
	}

	inline void ForceJumpFromBus(PlayerBot* Bot)
	{
		if (!Bot || !Bot->Pawn || !Bot->PC || Bot->bHasJumpedFromBus) return;

		if (!Bot->bHasThankedBusDriver)
		{
			Bot->bHasThankedBusDriver = true;
			Bot->PC->ThankBusDriver();
			Bot->DebugLog("thanked bus driver on jump", true);
		}

		auto* GameState = AFortGameStateAthena::Get();
		if (!GameState) return;

		if (Bot->PC->Blackboard)
		{
			Bot->PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhaseStep")), (uint8)EAthenaGamePhaseStep::BusFlying);
			Bot->PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhase")), (uint8)EAthenaGamePhase::Aircraft);
		}

		auto JumpLocation = Bot->Pawn->K2_GetActorLocation();
		if (GameState->GetAircraft(0))
		{
			JumpLocation = GameState->GetAircraft(0)->K2_GetActorLocation();
			if (!Bot->TargetDropZone.IsZero()) Bot->TargetDropZone.Z = JumpLocation.Z;
		}
		else
		{
			if (!Bot->TargetDropZone.IsZero())
			{
				JumpLocation = Bot->TargetDropZone;
				JumpLocation.Z += 2500.f;
			}
			else
			{
				JumpLocation.Z += 5000.f;
			}
		}

		JumpLocation.Z = fmaxf(JumpLocation.Z, 10000.f);

		Bot->Pawn->K2_TeleportTo(JumpLocation, Bot->Pawn->K2_GetActorRotation());
		Bot->Pawn->BeginSkydiving(true);
		Bot->Pawn->bIsSkydiving = true;
		Bot->Pawn->bIsSkydivingFromBus = true;
		Bot->Pawn->OnRep_IsSkydiving(false);
		Bot->Pawn->OnRep_IsSkydivingFromBus();
		Bot->Pawn->SetHealth(100);
		Bot->Pawn->SetShield(100);

		Bot->bHasJumpedFromBus = true;
		Bot->State = EPlayerBotState::Skydiving;
		Utils::Log("Bot jump -> phase=", (int)GameState->GamePhase,
			" skydiving=", Bot->Pawn->bIsSkydiving ? 1 : 0,
			" bus=", GameState->GetAircraft(0) ? 1 : 0,
			" loc=", JumpLocation.X, ",", JumpLocation.Y, ",", JumpLocation.Z);
	}
    
	// Main tick shit
	inline void Tick()
	{
		auto* GameState = AFortGameStateAthena::Get();
		if (!GameState) return;

		// Single GetAllActorsOfClass(AFortPickupAthena) per frame, shared by all bots
		auto* World = UWorld::Get();
		if (World)
			GlobalPickupCache::EnsureFresh(World);

		for (size_t i = 0; i < PlayerBotArray.size();)
		{
			auto* Bot = PlayerBotArray[i];

			if (!Bot || !Bot->Pawn || !Bot->PC || !Bot->PlayerState)
			{
				++i;
				continue;
			}

			// One-shot death handling. After OnDied sets bIsDead, skip this bot forever.
			// Do NOT delete/erase inside the same tick; other UE systems may still reference it.
			if (!Bot->bIsDead && Bot->Pawn->bIsDying)
			{
				Bot->OnDied();
				++i;
				continue;
			}

			if (Bot->bIsDead)
			{
				++i;
				continue;
			}

			WakeAI(Bot);
			Bot->EnsureInventory();
			UpdateBotEvaluator(Bot);

			if (Bot->State != Bot->LastLoggedState)
			{
				std::ostringstream StateMessage;
				StateMessage << "state change " << Bot->GetStateName(Bot->LastLoggedState) << " -> " << Bot->GetStateName(Bot->State);
				Bot->DebugLog(StateMessage.str(), true);
				Bot->DebugSnapshot("State transition");
				Bot->LastLoggedState = Bot->State;
			}

			if (Bot->CurrentEnemy && !Bot->IsThreatUsable(Bot->CurrentEnemy))
				Bot->CurrentEnemy = nullptr;

			if (Bot->TargetDropZone.IsZero())
				ChooseDropZone(Bot);

			if (GameState->GamePhase <= EAthenaGamePhase::Warmup)
			{
				Bot->State = EPlayerBotState::Warmup;
				Bot->PC->StopMovement();
				Bot->LookAt(nullptr);
				if (!Bot->IsPickaxeEquipped()) Bot->EquipPickaxe();
				if ((Bot->TickCounter % 180) == 0) Bot->Emote();
				++Bot->TickCounter;
				++i;
				continue;
			}

			if (Bot->State == EPlayerBotState::Bus)
			{
				if (!Bot->bLoggedBusState)
				{
					Bot->bLoggedBusState = true;
					Utils::Log("Bot entered bus -> phase=", (int)GameState->GamePhase, " hasAircraft=", GameState->GetAircraft(0) ? 1 : 0);
				}

				auto* Bus = GameState->GetAircraft(0);
				if (!Bus)
				{
					if (GameState->GamePhase > EAthenaGamePhase::Aircraft && !Bot->bHasJumpedFromBus)
						ForceJumpFromBus(Bot);
				}
				else
				{
					auto BusLocation = Bus->K2_GetActorLocation();
					auto DropTarget = Bot->TargetDropZone;
					DropTarget.Z = BusLocation.Z;

					if (GameState->GamePhase > EAthenaGamePhase::Aircraft)
					{
						Bot->TargetDropZone.Z = Bot->Pawn->K2_GetActorLocation().Z;
						ForceJumpFromBus(Bot);
					}
					else
					{
						auto DistanceToDrop = UKismetMathLibrary::Vector_Distance(BusLocation, DropTarget);
						if (DistanceToDrop < Bot->ClosestDistToDropZone)
						{
							Bot->ClosestDistToDropZone = DistanceToDrop;
						}
						else
						{
							Utils::Log("Bot bus passed drop -> dist=", DistanceToDrop, " best=", Bot->ClosestDistToDropZone);
							ForceJumpFromBus(Bot);
						}
					}
				}
			}

			else if (Bot->State == EPlayerBotState::Warmup && GameState->GamePhase >= EAthenaGamePhase::Aircraft)
			{
				Bot->ClosestDistToDropZone = FLT_MAX;
				SetBotToBus(Bot);
			}

			else if (Bot->State == EPlayerBotState::Skydiving)
			{
				auto BotLocation = Bot->Pawn->K2_GetActorLocation();
				if (!Bot->TargetDropZone.IsZero()) Bot->TargetDropZone.Z = BotLocation.Z;

				if (!Bot->Pawn->bIsSkydiving) Bot->State = EPlayerBotState::Gliding;

				if (!Bot->TargetDropZone.IsZero())
				{
					Bot->Pawn->AddMovementInput(UKismetMathLibrary::NegateVector(Bot->Pawn->GetActorUpVector()), 1.f, true);
					Bot->Pawn->AddMovementInput(Bot->Pawn->GetActorForwardVector(), Bot->bUseConfiguredDropZone ? 1.2f : 0.7f, true);
					auto LookRotation = UKismetMathLibrary::FindLookAtRotation(BotLocation, Bot->TargetDropZone);
					Bot->PC->SetControlRotation(LookRotation);
					Bot->PC->K2_SetActorRotation(LookRotation, true);
				}

				if (fabsf(Bot->Pawn->GetVelocity().Z) < 25.f && BotLocation.Z > 2000.f)
				{
					auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
					if (Bot->AirStallStartTime == 0.f) Bot->AirStallStartTime = CurrentTime;
					else if ((CurrentTime - Bot->AirStallStartTime) > 2.f)
					{
						FVector FallbackLocation = Bot->TargetDropZone.IsZero() ? BotLocation : Bot->TargetDropZone;
						FallbackLocation.Z = fmaxf(BotLocation.Z - 2500.f, 1500.f);
						Bot->Pawn->K2_TeleportTo(FallbackLocation, Bot->Pawn->K2_GetActorRotation());
						Bot->AirStallStartTime = 0.f;
					}
				}
				else
				{
					Bot->AirStallStartTime = 0.f;
				}
			}

			else if (Bot->State == EPlayerBotState::Gliding)
			{
				auto BotLocation = Bot->Pawn->K2_GetActorLocation();
				if (!Bot->TargetDropZone.IsZero()) Bot->TargetDropZone.Z = BotLocation.Z;

				if (Bot->Pawn->bIsSkydiving) Bot->State = EPlayerBotState::Skydiving;

				auto Velocity = Bot->Pawn->GetVelocity();
				if (Velocity.Z == 0.f || Bot->Pawn->bIsInWaterVolume)
				{
					if (Bot->PC->Blackboard)
					{
						Bot->PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhaseStep")), (uint8)EAthenaGamePhaseStep::None);
						Bot->PC->Blackboard->SetValueAsEnum(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_GamePhase")), (uint8)EAthenaGamePhase::SafeZones);
					}
					Bot->EquipPickaxe();
					Bot->State = EPlayerBotState::Landed;
				}
				else if (!Bot->TargetDropZone.IsZero())
				{
					Bot->Pawn->AddMovementInput(Bot->Pawn->GetActorForwardVector(), Bot->bUseConfiguredDropZone ? 1.2f : 0.8f, true);
					auto LookRotation = UKismetMathLibrary::FindLookAtRotation(BotLocation, Bot->TargetDropZone);
					Bot->PC->SetControlRotation(LookRotation);
					Bot->PC->K2_SetActorRotation(LookRotation, true);
				}
			}

			else if (Bot->State == EPlayerBotState::Landed)
			{
				Bot->PickupNearbyItems();
				Bot->CurrentEnemy = Bot->NearestPlayerActor;
				if (Bot->CurrentEnemy && !Bot->IsThreatUsable(Bot->CurrentEnemy))
					Bot->CurrentEnemy = nullptr;

				if (!Bot->HasGun()) Bot->State = EPlayerBotState::Looting;
				else if (Bot->CurrentEnemy && UKismetMathLibrary::Vector_Distance(Bot->Pawn->K2_GetActorLocation(), Bot->CurrentEnemy->K2_GetActorLocation()) < 5000.f)
					Bot->State = EPlayerBotState::LookingForPlayers;
				else
					Bot->State = EPlayerBotState::Looting;
			}

			else if (Bot->State == EPlayerBotState::Looting)
			{
				Bot->CurrentEnemy = Bot->NearestPlayerActor;
				if (Bot->CurrentEnemy && !Bot->IsThreatUsable(Bot->CurrentEnemy))
					Bot->CurrentEnemy = nullptr;

				auto BotLocation = Bot->Pawn->K2_GetActorLocation();
				auto Distance = Bot->CurrentEnemy ? UKismetMathLibrary::Vector_Distance(BotLocation, Bot->CurrentEnemy->K2_GetActorLocation()) : FLT_MAX;

				if (Bot->CurrentEnemy && Bot->ReactToRearThreat(Bot->CurrentEnemy, 1500.f))
				{
					++Bot->TickCounter;
					++i;
					continue;
				}

				if (Bot->HasGun() && Bot->CurrentEnemy && Distance < 1500.f)
				{
					Bot->State = EPlayerBotState::LookingForPlayers;
				}
				else
				{
					Bot->Pawn->PawnStopFire(0);
					Bot->SearchLoot();

					if (!Bot->TargetLootable)
					{
						if (Bot->CurrentEnemy && Distance < 325.f)
						{
							if (!Bot->IsPickaxeEquipped()) Bot->EquipPickaxe();
							Bot->MoveToActorOnNav(Bot->CurrentEnemy, 75.f, true);
							if (Distance < 250.f) Bot->Pawn->PawnStartFire(0);
						}
						else
						{
							if (Bot->CurrentEnemy)
							{
								Bot->MoveToActorOnNav(Bot->CurrentEnemy, 100.f, true);
								if (Distance > 6000.f) Bot->State = EPlayerBotState::MovingToSafeZone;
							}
							else
							{
								Bot->State = EPlayerBotState::MovingToSafeZone;
							}
						}
					}
				}
			}

			else if (Bot->State == EPlayerBotState::LookingForPlayers)
			{
				if (!Bot->HasGun())
				{
					Bot->State = EPlayerBotState::Looting;
				}
				else
				{
					Bot->CurrentEnemy = Bot->NearestPlayerActor;
					if (Bot->CurrentEnemy && !Bot->IsThreatUsable(Bot->CurrentEnemy))
						Bot->CurrentEnemy = nullptr;

					auto BotLocation = Bot->Pawn->K2_GetActorLocation();
					if (Bot->CurrentEnemy && Bot->ReactToRearThreat(Bot->CurrentEnemy, 1700.f))
					{
						++Bot->TickCounter;
						++i;
						continue;
					}

					if (Bot->CurrentEnemy)
					{
						auto TargetLocation = Bot->CurrentEnemy->K2_GetActorLocation();
						auto Distance = UKismetMathLibrary::Vector_Distance(BotLocation, TargetLocation);
						auto LookRotation = UKismetMathLibrary::FindLookAtRotation(BotLocation, TargetLocation);

						if (Bot->CurrentAimRotation.Pitch == 0.f && Bot->CurrentAimRotation.Yaw == 0.f && Bot->CurrentAimRotation.Roll == 0.f)
							Bot->CurrentAimRotation = Bot->PC->GetControlRotation();

						auto InterpSpeed = UKismetMathLibrary::RandomFloatInRange(3.f, 5.f);
						Bot->CurrentAimRotation = UKismetMathLibrary::RInterpTo(Bot->CurrentAimRotation, LookRotation, UGameplayStatics::GetTimeSeconds(UWorld::Get()) - (UGameplayStatics::GetTimeSeconds(UWorld::Get()) - 0.016f), InterpSpeed);

						auto YawDiff = fabsf(Bot->CurrentAimRotation.Yaw - LookRotation.Yaw);
						auto PitchDiff = fabsf(Bot->CurrentAimRotation.Pitch - LookRotation.Pitch);
						if (YawDiff > 180.f) YawDiff = 360.f - YawDiff;
						if (PitchDiff > 180.f) PitchDiff = 360.f - PitchDiff;
						bool bIsAimedEnough = YawDiff < 40.f && PitchDiff < 40.f;

						Bot->PC->SetControlRotation(Bot->CurrentAimRotation);
						Bot->PC->K2_SetActorRotation(Bot->CurrentAimRotation, true);
						Bot->SimpleSwitchToWeapon();
						Bot->EquipGun();

						if (Bot->PC->LineOfSightTo(Bot->CurrentEnemy, BotLocation, true) && bIsAimedEnough)
						{
							if (UKismetMathLibrary::RandomBoolWithWeight(0.025f)) Bot->Pawn->Crouch(false);
							Bot->ForceStrafe();
							if (Distance < 900.f) Bot->Pawn->AddMovementInput(Bot->Pawn->GetActorForwardVector() * -1.f, 1.f, true);
							if (!Bot->bIsStressed)
								Bot->MoveToActorOnNav(Bot->CurrentEnemy, UKismetMathLibrary::RandomFloatInRange(450.f, 1200.f), true);
							else
								Bot->Pawn->AddMovementInput(Bot->Pawn->GetActorForwardVector() * -1.f, 1.2f, true);
							if (Bot->CanFire())
								Bot->Pawn->PawnStartFire(0);
							else
							{
								Bot->Pawn->PawnStopFire(0);
								Bot->EquipGun();
							}
						}
						else
						{
							Bot->Pawn->PawnStopFire(0);
							if (Distance < 5000.f)
								Bot->MoveToActorOnNav(Bot->CurrentEnemy, 100.f, true);
							else
								Bot->State = EPlayerBotState::MovingToSafeZone;
						}
					}
					else
					{
						Bot->CurrentAimRotation = FRotator();
						Bot->State = EPlayerBotState::Looting;
					}
				}
			}

			else if (Bot->State == EPlayerBotState::MovingToSafeZone)
			{
				Bot->CurrentEnemy = Bot->NearestPlayerActor;
				if (Bot->CurrentEnemy && !Bot->IsThreatUsable(Bot->CurrentEnemy))
					Bot->CurrentEnemy = nullptr;

				auto BotLocation = Bot->Pawn->K2_GetActorLocation();
				auto Distance = Bot->CurrentEnemy ? UKismetMathLibrary::Vector_Distance(BotLocation, Bot->CurrentEnemy->K2_GetActorLocation()) : FLT_MAX;

				if (Bot->CurrentEnemy && Bot->ReactToRearThreat(Bot->CurrentEnemy, 1600.f))
				{
					++Bot->TickCounter;
					++i;
					continue;
				}

				if (Bot->CurrentEnemy && Distance < 3500.f)
				{
					Bot->State = Bot->HasGun() ? EPlayerBotState::LookingForPlayers : EPlayerBotState::Looting;
				}
				else if (GameState->SafeZoneIndicator)
				{
					if (UKismetMathLibrary::RandomBoolWithWeight(0.025f)) Bot->ForceStrafe();
					Bot->MoveToLocationOnNav(GameState->SafeZoneIndicator->NextCenter, GameState->SafeZoneIndicator->Radius, true);
				}
				else if (!Bot->TargetDropZone.IsZero())
				{
					Bot->MoveToLocationOnNav(Bot->TargetDropZone, 200.f, true);
				}
			}

			else if (Bot->State == EPlayerBotState::Stuck)
			{
				if (Bot->Pawn->bIsInWaterVolume)
				{
					Bot->State = Bot->CachedState;
					Bot->LastProgressTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
				}
				else
				{
					auto BotLocation = Bot->Pawn->K2_GetActorLocation();
					auto RecoveryTarget = GetBotGoalLocation(Bot);
					if (RecoveryTarget.IsZero())
						RecoveryTarget = BotLocation + (Bot->Pawn->GetActorForwardVector() * 600.f);

					auto LookRotation = UKismetMathLibrary::FindLookAtRotation(BotLocation, RecoveryTarget);
					auto CurrentRotation = Bot->PC->GetControlRotation();
					auto InterpRotation = UKismetMathLibrary::RInterpTo(CurrentRotation, LookRotation, 0.016f, UKismetMathLibrary::RandomFloatInRange(3.f, 6.f));
					Bot->PC->SetControlRotation(InterpRotation);
					Bot->PC->K2_SetActorRotation(InterpRotation, true);
					Bot->LookAt(nullptr);

					if (UKismetMathLibrary::RandomBoolWithWeight(0.2f))  Bot->Pawn->Jump();
					if (!Bot->Pawn->bIsCrouched && UKismetMathLibrary::RandomBoolWithWeight(0.05f)) Bot->Pawn->Crouch(false);
					if (UKismetMathLibrary::RandomBoolWithWeight(0.05f)) Bot->Pawn->UnCrouch(false);

					Bot->ForceStrafe();

					FVector RecoveryPoint = RecoveryTarget;
					RecoveryPoint.X += UKismetMathLibrary::RandomFloatInRange(-250.f, 250.f);
					RecoveryPoint.Y += UKismetMathLibrary::RandomFloatInRange(-250.f, 250.f);
					Bot->MoveToLocationOnNav(RecoveryPoint, 75.f, true);
					Bot->Pawn->AddMovementInput(Bot->Pawn->GetActorForwardVector(), 1.0f, true);

					auto CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::Get());
					if ((CurrentTime - Bot->LastProgressTime) > 1.0f)
					{
						Bot->State = Bot->CachedState;
						Bot->LastStuckCheckLocation = Bot->Pawn->K2_GetActorLocation();
						Bot->LastGoalLocation = GetBotGoalLocation(Bot);
						Bot->LastProgressTime = CurrentTime;
					}
				}
			}

			++Bot->TickCounter;
			++i;
		}
	}
}
