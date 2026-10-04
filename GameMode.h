#pragma once
#include "pch.h"
#include "Looting.h"
#include "Vehicles.h"
#include "Misc.h"
#include "Globals.h"
#include "Abilities.h"
#include "Building.h"
#include "Backend.h"
#include "PlayerBots.h"
#include <thread>
#include <chrono>

class GameMode
{
	static inline bool (*ReadyToStartMatchOG)(AFortGameModeAthena*);
	static bool ReadyToStartMatchHook(AFortGameModeAthena* GM)
	{
		ReadyToStartMatchOG(GM);
		auto GS = AFortGameStateAthena::Get();

		if (!GS->CurrentPlaylistInfo.BasePlaylist)
		{
			SetConsoleTitleA("Fate 13.40 | Setting up");
			GM->WarmupRequiredPlayerCount = 1;

			auto Playlist = Utils::FindObject<UFortPlaylistAthena>(Globals::PlaylistName.data());
			if (!Playlist)
			{
				Utils::Log("Playlist not found, falling back to default!");
				Globals::PlaylistName = Globals::DefaultPlaylistName;
				Globals::PlaylistShortName = Globals::DefaultPlaylistShortName;
				Playlist = Utils::FindObject<UFortPlaylistAthena>(Globals::PlaylistName.data());
			}
			if (!Playlist)
			{
				Utils::Log("Default playlist not found!");
				return false;
			}

			GS->CurrentPlaylistInfo.BasePlaylist = Playlist;
			GS->CurrentPlaylistInfo.OverridePlaylist = Playlist;
			GS->CurrentPlaylistInfo.PlaylistReplicationKey++;
			GS->CurrentPlaylistInfo.MarkArrayDirty();
			GS->OnRep_CurrentPlaylistInfo();

			GM->bAlwaysDBNO = Playlist->MaxSquadSize > 1;
			GS->CurrentPlaylistId = Playlist->PlaylistId;
			GS->OnRep_CurrentPlaylistId();
			GM->CurrentPlaylistName = Playlist->PlaylistName;
			GM->CurrentPlaylistId = Playlist->PlaylistId;
			GM->GameSession->MaxPlayers = Playlist->MaxPlayers;
			GS->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;
			GS->AirCraftBehavior = Playlist->AirCraftBehavior;

			if (auto BotManager = (UFortServerBotManagerAthena*)UGameplayStatics::SpawnObject(UFortServerBotManagerAthena::StaticClass(), GM))
			{
				GM->ServerBotManager = BotManager;
				BotManager->CachedGameState = GS;
				BotManager->CachedGameMode = GM;
			}

			static void (*CreateAIDirector)(AFortGameModeAthena * GameMode) = decltype(CreateAIDirector)(ImageBase + 0x1EA2F80);
			CreateAIDirector(GM);

			if (GM->AIDirector)
				GM->AIDirector->Activate();

			if (Playlist->AISettings && Playlist->AISettings->bAllowAIGoalManager)
				GM->AIGoalManager = Utils::SpawnActor<AFortAIGoalManager>({ 0, 0, -99999 }, {});

			if (Globals::bBotsEnabled)
				PlayerBots::EnsureCosmeticPools();
		}

		if (!UWorld::Get()->NetDriver)
		{
			auto Starts = Globals::bCustomMap
				? (TArray<AActor*>) Utils::GetAll<AFortPlayerStartCreative>()
				: (TArray<AActor*>) Utils::GetAll<AFortPlayerStartWarmup>();
			auto StartsNum = Starts.Num();
			Starts.Free();
			if (StartsNum == 0)
				return false;

			using CreateNetDriverType = UNetDriver * (*)(UEngine*, UWorld*, FName);
			using InitListenType = bool (*)(UNetDriver*, UWorld*, FURL&, bool, FString&);
			using SetWorldType = void (*)(UNetDriver*, UWorld*);

			auto GND = FName(L"GameNetDriver");
			auto NetDriver = ((CreateNetDriverType)CreateNetDriver)(UEngine::GetEngine(), UWorld::Get(), GND);
			NetDriver->World = UWorld::Get();
			NetDriver->NetDriverName = GND;
			UWorld::Get()->NetDriver = NetDriver;

			for (auto& Collection : UWorld::Get()->LevelCollections)
				Collection.NetDriver = NetDriver;

			FString Err;
			FURL URL;
			URL.Port = 7777;

			((InitListenType)InitListen)(NetDriver, UWorld::Get(), URL, false, Err);
			((SetWorldType)SetWorld)(NetDriver, UWorld::Get());
			SetConsoleTitleA("Fate 13.40 | Listening");
			GM->bWorldIsReady = true;

			// Wait until server is fully ready before telling backend
			std::thread([]() {
				std::this_thread::sleep_for(std::chrono::seconds(50));
				Backend::RegisterServer();
				}).detach();
		}

		static bool bWarmupTimerSet = false;
		if (!bWarmupTimerSet)
		{
			bWarmupTimerSet = true;
			auto Time = UGameplayStatics::GetTimeSeconds(UWorld::Get());
			auto WarmupDuration = 120.f; 

			GS->WarmupCountdownStartTime = Time;
			GS->WarmupCountdownEndTime = Time + WarmupDuration;
			GM->WarmupCountdownDuration = WarmupDuration;
			GM->WarmupEarlyCountdownDuration = WarmupDuration;
		}

		return GM->bWorldIsReady;
	}

	static APawn *SpawnDefaultPawnForHook(AFortGameModeAthena *GM, AFortPlayerControllerAthena *PC, AActor *StartSpot)
	{
		FRotator StartRotation;
		StartRotation.Yaw = StartSpot->K2_GetActorRotation().Yaw;
		auto Transform = FTransform(StartSpot->K2_GetActorLocation(), StartRotation);
		auto Pawn = GM->SpawnDefaultPawnAtTransform(PC, Transform);

		PC->WorldInventory->Inventory.ReplicatedEntries.ResetNum();
		PC->WorldInventory->Inventory.ItemInstances.ResetNum();

		for (auto &SI : GM->StartingItems)
			if (SI.Count)
				Inventory::GiveItem(PC, SI.Item, SI.Count);

		Inventory::GiveItem(PC, PC->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);

		return Pawn;
	}

	static inline void (*HandleStartingNewPlayerOG)(AGameModeBase*, APlayerController*);
	static void HandleStartingNewPlayerHook(AGameModeBase* GM, APlayerController* NewPlayer)
	{
		auto GS = AFortGameStateAthena::Get();


		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)NewPlayer->PlayerState;
		AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)NewPlayer;

		for (auto& Modifier : GS->CurrentPlaylistInfo.BasePlaylist->ModifierList)
		{
			for (auto& AbilitySet : Modifier.Get()->PersistentAbilitySets)
				if (!AbilitySet.DeliveryRequirements.bConsiderTeam &&
					AbilitySet.DeliveryRequirements.bApplyToPlayerPawns &&
					AbilitySet.AbilitySets)
				{
					for (auto& Set : AbilitySet.AbilitySets)
						Abilities::GiveAbilitySet(PlayerState, Set.Get());
				}

			for (auto& GameplayEffectSet : Modifier->PersistentGameplayEffects)
				if (!GameplayEffectSet.DeliveryRequirements.bConsiderTeam &&
					GameplayEffectSet.DeliveryRequirements.bApplyToPlayerPawns &&
					GameplayEffectSet.GameplayEffects)
				{
					for (auto& GameplayEffect : GameplayEffectSet.GameplayEffects)
					{
						auto GE = GameplayEffect.GameplayEffect.Get();
						if (!GE)
							continue;

						PlayerState->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(
							GE,
							GameplayEffect.Level,
							FGameplayEffectContextHandle());
					}
				}
		}

		PlayerState->SeasonLevelUIDisplay = PC->XPComponent->CurrentLevel;
		PlayerState->OnRep_SeasonLevelUIDisplay();

		PC->XPComponent->bRegisteredWithQuestManager = true;
		PC->XPComponent->OnRep_bRegisteredWithQuestManager();

		PC->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(PC->Pawn);

		if (!PC->MatchReport)
			PC->MatchReport = (UAthenaPlayerMatchReport*)UGameplayStatics::SpawnObject(
				UAthenaPlayerMatchReport::StaticClass(), PC);

		PlayerState->SquadId = PlayerState->TeamIndex - 3;
		PlayerState->OnRep_SquadId();

		FGameMemberInfo Member;
		Member.MostRecentArrayReplicationKey = -1;
		Member.ReplicationID = -1;
		Member.ReplicationKey = -1;
		Member.TeamIndex = PlayerState->TeamIndex;
		Member.SquadId = PlayerState->SquadId;
		Member.MemberUniqueId = PlayerState->UniqueId;

		GS->GameMemberInfoArray.Members.Add(Member);
		GS->GameMemberInfoArray.MarkArrayDirty();

		return HandleStartingNewPlayerOG(GM, NewPlayer);
	}

	static inline void (*OnAircraftExitedDropZoneOG)(AFortGameModeAthena *, AFortAthenaAircraft *);
	static void OnAircraftExitedDropZone(AFortGameModeAthena *GM, AFortAthenaAircraft *Aircraft)
	{
		PlayerBots::SetBotsToBus();
		for (auto* Bot : PlayerBotArray)
			PlayerBots::ForceJumpFromBus(Bot);

		if (LateGame)
		{
			for (auto &Player : GM->AlivePlayers)
			{
				if (Player->IsInAircraft())
				{
					Player->GetAircraftComponent()->ServerAttemptAircraftJump({});
				}
			}
		}
		return OnAircraftExitedDropZoneOG(GM, Aircraft);
	}

public:
	static inline bool startedBus = false;
	static inline bool dumpedBotMap = false;

	static int GetAliveRealPlayerCount(AFortGameModeAthena* GM)
	{
		return GM ? GM->AlivePlayers.Num() : 0;
	}

	static int GetAliveBotCount(AFortGameModeAthena* GM)
	{
		return GM ? GM->AliveBots.Num() : 0;
	}

	static int GetTotalAlivePlayerCount(AFortGameModeAthena* GM)
	{
		return GetAliveRealPlayerCount(GM) + GetAliveBotCount(GM);
	}

	static int GetBotAllowance(AFortGameModeAthena* GM)
	{
		if (!GM || !GM->GameSession)
			return 0;

		return max(0, GM->GameSession->MaxPlayers - GetAliveRealPlayerCount(GM));
	}
    static inline xmap<AFortPlayerControllerAthena*, xset<xstring>> DiscoveredPois;
    
    static void CheckPoiDiscovery(AFortGameModeAthena* GM)
    {
	auto GS = AFortGameStateAthena::Get();
	if (!GS || !GM) 
		return;
	for (auto PC : GM->AlivePlayers) 
		{
			if (!PC || !PC->MyFortPawn || !PC->DiscoverabilityComponent)
				continue;

			auto PoiTags = GS->GetPoiGridTagsForLocation(PC->MyFortPawn->K2_GetActorLocation());
			auto& Discovered = DiscoveredPois[PC];

			for (auto& Tag : PoiTags.GameplayTags)
				{
					auto TagName = Tag.TagName.ToString();
					if (TagName.find("POI") == xstring::npos || Discovered.count(TagName))
						continue;

					Discovered.insert(TagName);
					PC->DiscoverabilityComponent->SetDiscoverStatusByTag(FString(TagName), true);
					if (auto QuestManager = PC->GetQuestManager(ESubGame::Athena))
					{
						FGameplayTagContainer SourceTags, ContextTags, TargetTags;
						QuestManager->GetSourceAndContextTags(&SourceTags, &ContextTags);
						TargetTags.GameplayTags.Add(Tag);

						XP::SendStatEvent(QuestManager, PC->MyFortPawn, SourceTags, TargetTags, nullptr, nullptr, 1, EFortQuestObjectiveStatEvent::VisitDiscoverPOI);
					}

					Utils::Log("Discovered POI: " + TagName);
				}
		}
	}
	// Not good but who cares ima change later
	static void DumpBotMapData()
	{
		if (dumpedBotMap || !UWorld::Get())
			return;

		dumpedBotMap = true;

		std::ofstream FullDump("C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotMapDump.txt", std::ios::trunc);
		std::ofstream NodeDump("C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotMapNodes.txt", std::ios::trunc);
		std::ofstream GraphDump("C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotGraph.txt", std::ios::trunc);
		if (!FullDump.is_open() || !NodeDump.is_open() || !GraphDump.is_open())
		{
			Utils::Log("Bot map dump failed to open output files.");
			return;
		}

		struct FGraphDumpNode
		{
			int Id;
			std::string Label;
			std::string Name;
			FVector Location;
		};

		xvector<FGraphDumpNode> GraphNodes{};
		std::set<std::string> PickupCells{};
		std::set<std::string> ChestCells{};
		std::set<std::string> AmmoCells{};

		auto WriteActorLine = [](std::ofstream& Stream, const char* Label, AActor* Actor)
		{
			if (!Actor)
				return;

			auto Loc = Actor->K2_GetActorLocation();
			auto Rot = Actor->K2_GetActorRotation();
			Stream << Label
				<< "|Class=" << Actor->Class->Name.ToString()
				<< "|Name=" << Actor->Name.ToString()
				<< "|X=" << Loc.X
				<< "|Y=" << Loc.Y
				<< "|Z=" << Loc.Z
				<< "|Pitch=" << Rot.Pitch
				<< "|Yaw=" << Rot.Yaw
				<< "|Roll=" << Rot.Roll
				<< "\n";
		};

		auto MakeGridKey = [](const FVector& Loc, float CellSize)
		{
			auto GridX = (int)floor(Loc.X / CellSize);
			auto GridY = (int)floor(Loc.Y / CellSize);
			auto GridZ = (int)floor(Loc.Z / CellSize);
			return std::to_string(GridX) + ":" + std::to_string(GridY) + ":" + std::to_string(GridZ);
		};

		auto AddGraphNode = [&](const char* Label, AActor* Actor)
		{
			if (!Actor)
				return;

			auto Loc = Actor->K2_GetActorLocation();
			std::set<std::string>* CellSet = nullptr;
			auto LabelString = std::string(Label);

			float CellSize = 0.f;
			bool bUseCellFilter = false;

			if (LabelString == "PICKUP")
			{
				CellSet = &PickupCells;
				CellSize = 4000.f;
				bUseCellFilter = true;
			}
			else if (LabelString == "CHEST" || LabelString == "FACTION_CHEST")
			{
				CellSet = &ChestCells;
				CellSize = 2500.f;
				bUseCellFilter = true;
			}
			else if (LabelString == "AMMO_BOX")
			{
				CellSet = &AmmoCells;
				CellSize = 2500.f;
				bUseCellFilter = true;
			}

			if (bUseCellFilter && CellSet)
			{
				auto CellKey = MakeGridKey(Loc, CellSize);
				if (CellSet->find(CellKey) != CellSet->end())
					return;
				CellSet->insert(CellKey);
			}

			FGraphDumpNode Node{};
			Node.Id = (int)GraphNodes.size();
			Node.Label = LabelString;
			Node.Name = Actor->Name.ToString().c_str();
			Node.Location = Loc;
			GraphNodes.push_back(Node);
		};

		auto DumpClass = [&](const char* Label, UClass* Class)
		{
			if (!Class)
				return 0;

			auto Actors = Utils::GetAll<AActor>(Class);
			for (int32 i = 0; i < Actors.Num(); ++i)
				WriteActorLine(NodeDump, Label, Actors[i]);
			auto Count = Actors.Num();
			Actors.Free();
			return Count;
		};

		auto AllActors = Utils::GetAll<AActor>(AActor::StaticClass());
		for (int32 i = 0; i < AllActors.Num(); ++i)
			WriteActorLine(FullDump, "ACTOR", AllActors[i]);
		auto TotalActors = AllActors.Num();
		AllActors.Free();

		auto WarmupStarts = DumpClass("WARMUP_START", AFortPlayerStartWarmup::StaticClass());
		auto PlayerStarts = DumpClass("PLAYER_START", AFortPlayerStart::StaticClass());
		auto Chests = DumpClass("CHEST", Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena.Tiered_Chest_Athena_C"));
		auto FactionChests = DumpClass("FACTION_CHEST", Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena_FactionChest.Tiered_Chest_Athena_FactionChest_C"));
		auto AmmoBoxes = DumpClass("AMMO_BOX", Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Ammo_Athena.Tiered_Ammo_Athena_C"));
		auto Pickups = DumpClass("PICKUP", AFortPickupAthena::StaticClass());
		auto Containers = DumpClass("CONTAINER", ABuildingContainer::StaticClass());
		auto Buildings = DumpClass("BUILDING_SM", ABuildingSMActor::StaticClass());
		auto NavBounds = DumpClass("NAV_BOUNDS", ANavMeshBoundsVolume::StaticClass());
		auto POIs = DumpClass("POI", AFortPoiVolume::StaticClass());

		{
			auto AddActorsToGraph = [&](const char* Label, UClass* Class)
			{
				if (!Class)
					return;

				auto Actors = Utils::GetAll<AActor>(Class);
				for (int32 i = 0; i < Actors.Num(); ++i)
					AddGraphNode(Label, Actors[i]);
				Actors.Free();
			};

			AddActorsToGraph("PLAYER_START", AFortPlayerStart::StaticClass());
			AddActorsToGraph("WARMUP_START", AFortPlayerStartWarmup::StaticClass());
			AddActorsToGraph("POI", AFortPoiVolume::StaticClass());
			AddActorsToGraph("CHEST", Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena.Tiered_Chest_Athena_C"));
			AddActorsToGraph("FACTION_CHEST", Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena_FactionChest.Tiered_Chest_Athena_FactionChest_C"));
			AddActorsToGraph("AMMO_BOX", Utils::FindObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Ammo_Athena.Tiered_Ammo_Athena_C"));
			AddActorsToGraph("PICKUP", AFortPickupAthena::StaticClass());
		}

		for (int i = 0; i < (int)GraphNodes.size(); ++i)
		{
			auto& Node = GraphNodes[i];
			GraphDump
				<< "NODE|Id=" << Node.Id
				<< "|Type=" << Node.Label
				<< "|Name=" << Node.Name
				<< "|X=" << Node.Location.X
				<< "|Y=" << Node.Location.Y
				<< "|Z=" << Node.Location.Z
				<< "\n";
		}

		for (int i = 0; i < (int)GraphNodes.size(); ++i)
		{
			auto& Node = GraphNodes[i];
			xvector<pair<float, int>> Candidates{};

			for (int j = 0; j < (int)GraphNodes.size(); ++j)
			{
				if (i == j)
					continue;

				auto& Other = GraphNodes[j];
				auto DeltaZ = abs(Node.Location.Z - Other.Location.Z);
				if (DeltaZ > 5000.f)
					continue;

				auto Distance = UKismetMathLibrary::Vector_Distance(Node.Location, Other.Location);
				auto MaxDistance = (Node.Label == "POI" || Other.Label == "POI") ? 35000.f : 18000.f;
				if (Distance > MaxDistance)
					continue;

				Candidates.push_back({ Distance, j });
			}

			std::sort(Candidates.begin(), Candidates.end(), [](const auto& A, const auto& B)
			{
				return A.first < B.first;
			});

			auto LinkCount = min((int)Candidates.size(), Node.Label == "POI" ? 8 : 5);
			for (int LinkIndex = 0; LinkIndex < LinkCount; ++LinkIndex)
			{
				auto& Candidate = Candidates[LinkIndex];
				GraphDump
					<< "EDGE|From=" << Node.Id
					<< "|To=" << Candidate.second
					<< "|Dist=" << Candidate.first
					<< "\n";
			}
		}

		FullDump.flush();
		NodeDump.flush();
		GraphDump.flush();

		Utils::Log("Bot map dump complete. actors=", TotalActors,
			" warmupStarts=", WarmupStarts,
			" playerStarts=", PlayerStarts,
			" chests=", Chests,
			" factionChests=", FactionChests,
			" ammoBoxes=", AmmoBoxes,
			" pickups=", Pickups,
			" containers=", Containers,
			" buildings=", Buildings,
			" navBounds=", NavBounds,
			" pois=", POIs,
			" graphNodes=", GraphNodes.size());
		Utils::Log("Bot map files: C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotMapDump.txt, C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotMapNodes.txt and C:\\Users\\Kai\\Desktop\\Fate-13.40\\BotGraph.txt");
	}

private:
	static inline void (*TickFlushOG)(UNetDriver *);
	static void TickFlushHook(UNetDriver *Driver)
	{
		if (Driver->ReplicationDriver)
			((void (*)(UReplicationDriver *))ServerReplicateActors)(Driver->ReplicationDriver);

		auto* GS = AFortGameStateAthena::Get();
		auto* GM = AFortGameModeAthena::Get();
		if (GS && GM && !dumpedBotMap)
			DumpBotMapData();

		auto RealPlayerCount = GetAliveRealPlayerCount(GM);
		auto AliveBotCount = GetAliveBotCount(GM);
		auto TotalPlayerCount = GetTotalAlivePlayerCount(GM);
		Backend::gAlivePlayerCount = TotalPlayerCount;
		auto BotAllowance = GetBotAllowance(GM);

		auto CurrentBotAllowance = GetBotAllowance(GM);

        CheckPoiDiscovery(GM);

        static bool bEndGameTriggered = false;
        if (!bEndGameTriggered  && Backend::bGameStarted && RealPlayerCount == 0)
        {
           bEndGameTriggered = true;
           Utils::Log("No real players remain shutting down server");
           
		   #include <Windows.h>
           
		   std::thread([]() {
           Sleep(500);
           Backend::GameStopped();
           Sleep(5000);
           ExitProcess(0);
		   }).detach();
		}
        if (Globals::bBotsEnabled && GS && GM && GS->GamePhase == EAthenaGamePhase::Warmup && RealPlayerCount > 0 && AliveBotCount < CurrentBotAllowance && TotalPlayerCount < GM->GameSession->MaxPlayers)
		{
			static TArray<AActor*> PlayerStarts;
			static bool bCachedStarts = false;
			if (!bCachedStarts)
			{
				PlayerStarts = Utils::GetAll<AFortPlayerStartWarmup>();
				bCachedStarts = true;
			}

			if (PlayerStarts.Num() > 0 && UKismetMathLibrary::RandomBoolWithWeight(0.05f))
				PlayerBots::SpawnPlayerBots(PlayerStarts[UKismetMathLibrary::RandomIntegerInRange(0, PlayerStarts.Num() - 1)]);
		}

		if (!startedBus)
		{
			auto Time = UGameplayStatics::GetTimeSeconds(UWorld::Get());
			auto bLobbyFilled = GS && GM && TotalPlayerCount >= GM->GameSession->MaxPlayers;
			if (bLobbyFilled)
			{
				GS->WarmupCountdownEndTime = Time;
			}

			if (GS->WarmupCountdownEndTime <= Time || GM->AlivePlayers.Num() >= 30)
			{
				startedBus = true;
				GS->WarmupCountdownEndTime = Time; 
				PlayerBots::SetBotsToBus();
				((void (*)(AGameModeBase*, int))(ImageBase + 0x1ed5710))(AFortGameModeAthena::Get(), 0);
			}
		}
		else
		{
			for (auto& Bot : BotArray)
			{
				Bot->Tick();
			}
		}

		//Wildlife::Tick();
		PlayerBots::Tick();

		return TickFlushOG(Driver);
	}

	static UClass** GetGameSessionClass(__int64 a1, UClass** OutClass)
	{
		*OutClass = AFortGameSessionDedicatedAthena::StaticClass();
		return OutClass;
	}

	static void SpawnAIDirectorHook(AFortGameModeAthena* GameMode)
	{
		if (GameMode)
		{
			GameMode->AIDirector = Utils::SpawnActor<AAthenaAIDirector>({ 0, 0, -99999 }, {});
		}
	}

public:
	static void HookFunctions()
	{
		Utils::PatchU32(ImageBase + 0x1EA2F9C, 0x216C930);
		Utils::Hook(ImageBase + 0x400F8D0, SpawnAIDirectorHook);
		Utils::Hook(ReadyToStartMatch, ReadyToStartMatchHook, ReadyToStartMatchOG);
		Utils::Hook(TickFlush, TickFlushHook, TickFlushOG);
		Utils::Hook<AFortGameModeAthena>(uint32(0xd3), GetGameSessionClass);
	}

	static void HookPost()
	{
		Utils::Hook(ImageBase + 0x1ebade0, OnAircraftExitedDropZone, OnAircraftExitedDropZoneOG);
		Utils::Hook<AFortGameModeAthena>(HandleStartingNewPlayerVft, HandleStartingNewPlayerHook, HandleStartingNewPlayerOG);
		Utils::Hook<AFortGameModeAthena>(SpawnDefaultPawnForVft, SpawnDefaultPawnForHook);
		//Wildlife::Init();
	}
};
