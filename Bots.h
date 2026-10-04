#pragma once
#include "pch.h"
#include "Utils.h"
#include "XP.h"

namespace PlayerBots
{
	void RecordDeathContext(AFortAthenaAIBotController* DeadBotController, AController* InstigatedBy, AActor* DamageCauser);
}

std::vector<class Bot*> BotArray;
class Bot
{
public:
	AFortAthenaAIBotController* PC;
	ABP_MangPlayerPawn_Parent_C* Pawn;
	uint32 LKPTickCounter = 0;
	float LKPMoveDistance = 0;
	FVector LKPOldLoc = {};
	uint32 AlertedTickCounter = 0;
	uint32 ThreatenedTickCounter = 0;
	uint32 Shots = 0;
	bool IncrementShots = false;
	bool WasLKP = false;
	uint32 CurrentPatrolPoint = 0;
	bool LineOfSight = false;
	bool bHadATarget = false;
	bool jumping = false;
	int shotCounter = 0;
	FVector lastLocation{};
	bool bPatrollingForward = true;
	bool bIsWaiting = false;
	float WaitUntil = 0.f;

	Bot(AFortAthenaAIBotController* PC) : PC(PC)
	{
		Pawn = (ABP_MangPlayerPawn_Parent_C*)PC->PlayerBotPawn;
	}

	AActor* GetTarget(int32 Index)
	{
		if (PC->TargetHandler.Targets.Num() <= Index)
			return nullptr;
		return PC->TargetHandler.Targets[Index].SourceActor;
	}

	void CreateInventory()
	{
		auto Inv = Utils::SpawnActor<AFortInventory>(FVector{ 0, 0, -99999 }, {});
		Inv->Owner = PC;
		Inv->OnRep_Owner();
		PC->Inventory = Inv;
	}

	FFortItemEntry& GiveItem(UFortItemDefinition* Def, int Count)
	{
		UFortWorldItem* Item = (UFortWorldItem*)Def->CreateTemporaryItemInstanceBP(Count, -1);
		Item->OwnerInventory = PC->Inventory;
		if (auto Weapon = Def->Cast<UFortWeaponItemDefinition>())
		{
			Item->ItemEntry.LoadedAmmo = Inventory::GetStats(Weapon)->ClipSize;
		}
		auto& IE = PC->Inventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
		auto II = PC->Inventory->Inventory.ItemInstances.Add(Item);
		PC->Inventory->Inventory.MarkItemDirty(Item->ItemEntry);
		PC->Inventory->HandleInventoryLocalUpdate();
		return IE;
	}

	float ModLocation(float Base, float Sep, float Max)
	{
		auto Num = UKismetMathLibrary::RandomFloatInRange(0.f, 3.f);
		float Mod = 0.f;
		if (Num < 1.f)
			Mod = UKismetMathLibrary::RandomFloatInRange(-Base, Base);
		else if (Num > 1.f && Num < 2.f)
			Mod = UKismetMathLibrary::RandomFloatInRange(Base + Sep, Max);
		else
			Mod = UKismetMathLibrary::RandomFloatInRange(-Max, -Base - Sep);
		return Mod;
	}

	void Tick()
	{
		if (!Pawn || !Pawn->IsA<ABP_MangPlayerPawn_Parent_C>() || Pawn->bIsDBNO || !PC->Inventory)
			return;

		auto Target = GetTarget(0);

		if (!Target)
		{
			if (bHadATarget)
			{
				ThreatenedTickCounter = 0;
				Shots = 0;
				LKPTickCounter = 0;
				shotCounter = 0;
				jumping = false;
				Pawn->PawnStopFire(0);
				bHadATarget = false;
			}

			// Patrol logic - walk speed, ping pong between points, wait at each
			if (PC->CachedPatrollingComponent && PC->CachedPatrollingComponent->PatrolPath)
			{
				auto PatrolPath = PC->CachedPatrollingComponent->PatrolPath;
				int32 NumPoints = PatrolPath->PatrolPoints.Num();
				if (NumPoints > 0)
				{
					// Set walk speed so they don't run during patrol
					if (Pawn->CharacterMovement)
					{
						Pawn->CharacterMovement->MaxWalkSpeed = 300.f;
					}

					if (bIsWaiting)
					{
						if (WaitUntil > 0.f)
							WaitUntil -= 1.f;
						else
							bIsWaiting = false;
						return;
					}

					if (CurrentPatrolPoint >= (uint32)NumPoints)
						CurrentPatrolPoint = 0;

					auto Point = PatrolPath->PatrolPoints[CurrentPatrolPoint];
					if (Point)
					{
						auto BotPos = Pawn->K2_GetActorLocation();
						auto PointPos = Point->K2_GetActorLocation();
						auto Rot = UKismetMathLibrary::FindLookAtRotation(BotPos, PointPos);
						PC->SetControlRotation(Rot);
						Pawn->AddMovementInput(Pawn->GetActorForwardVector(), 1.f, true);

						if (Pawn->GetDistanceTo(Point) < 200.f)
						{
							// Wait at point then advance
							bIsWaiting = true;
							WaitUntil = (float)((int)(UKismetMathLibrary::RandomFloatInRange(90.f, 210.f)));

							int32 MaxPoints = NumPoints - 1;
							if (bPatrollingForward)
							{
								if ((int32)CurrentPatrolPoint >= MaxPoints)
									bPatrollingForward = false;
								else
									CurrentPatrolPoint++;
							}
							else
							{
								if (CurrentPatrolPoint <= 0)
									bPatrollingForward = true;
								else
									CurrentPatrolPoint--;
							}
						}
					}
				}
			}
			return;
		}

		if (auto TargetPawn = Target->Cast<AFortPlayerPawnAthena>())
			if (TargetPawn->bIsDBNO)
			{
				Pawn->AlertLevel = EAlertLevel::Unaware;
				Pawn->OnRep_AlertLevel();
				PC->TargetHandler.Targets.Remove(0);
				return;
			}

		if (!bHadATarget)
		{
			bHadATarget = true;
			// Restore run speed when entering combat
			if (Pawn->CharacterMovement)
				Pawn->CharacterMovement->MaxWalkSpeed = 600.f;
		}

		auto BotPos = Pawn->K2_GetActorLocation();
		auto TargetPos = Target->K2_GetActorLocation();
		float Distance = Pawn->GetDistanceTo(Target);
		LineOfSight = PC->LineOfSightTo(Target, PC->GetViewTarget() ? PC->GetViewTarget()->K2_GetActorLocation() : BotPos, true);

		switch (Pawn->AlertLevel)
		{
		case EAlertLevel::Threatened:
		{
			LKPTickCounter = 0;

			if (Pawn->CurrentWeapon && Pawn->CurrentWeapon->WeaponData && Pawn->CurrentWeapon->WeaponData->IsA<UFortWeaponMeleeItemDefinition>() && WasLKP)
			{
				WasLKP = false;
				auto Weapon = PC->Inventory->Inventory.ReplicatedEntries.Search([](FFortItemEntry& Entry)
					{ return Entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>(); });
				if (Weapon)
				{
					Pawn->EquipWeaponDefinition(Weapon->ItemDefinition->Cast<UFortWeaponItemDefinition>(), Weapon->ItemGuid);
					Pawn->PawnStopFire(0);
				}
			}

			if (Distance < 300 && LineOfSight)
				PC->StopMovement();
			else
			{
				auto Rot = UKismetMathLibrary::FindLookAtRotation(BotPos, TargetPos);
				PC->SetControlRotation(Rot);
				Pawn->AddMovementInput(Pawn->GetActorForwardVector(), 1.f, true);
			}

			if (ThreatenedTickCounter > 60)
			{
				if (!Pawn->bIsCrouched && UKismetMathLibrary::RandomBoolWithWeight(0.002f))
					Pawn->Crouch(false);
				else if (UKismetMathLibrary::RandomBoolWithWeight(0.002f))
				{
					Pawn->Jump();
					jumping = true;
				}
				else if (Pawn->bIsCrouched && ThreatenedTickCounter % 100 == 0)
					Pawn->UnCrouch(false);
				else if (jumping && ThreatenedTickCounter % 10 == 0)
				{
					Pawn->StopJumping();
					jumping = false;
				}
			}

			Pawn->bIsTargeting = true;
			Pawn->OnRep_IsTargeting();

			if (ThreatenedTickCounter % 17 == 0)
			{
				float XMod = ModLocation(30.f, 18.f, 30.f);
				float YMod = ModLocation(34.f, 19.f, 34.f);
				float ZMod = ModLocation(16.f, 17.f, 34.f);
				auto NTLoc = FVector(TargetPos.X + XMod, TargetPos.Y + YMod, TargetPos.Z + ZMod);
				auto Rot = UKismetMathLibrary::FindLookAtRotation(BotPos, NTLoc);
				PC->SetControlRotation(Rot);
			}

			if (!LineOfSight)
			{
				auto Pickaxe = PC->Inventory->Inventory.ReplicatedEntries.Search([](FFortItemEntry& Entry)
					{ return Entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>(); });
				if (Pickaxe && Pawn->CurrentWeapon && Pawn->CurrentWeapon->WeaponData && !Pawn->CurrentWeapon->WeaponData->IsA<UFortWeaponMeleeItemDefinition>())
				{
					Pawn->EquipWeaponDefinition(Pickaxe->ItemDefinition->Cast<UFortWeaponItemDefinition>(), Pickaxe->ItemGuid);
					Pawn->PawnStartFire(0);
					IncrementShots = false;
					Shots = 0;
					shotCounter = 0;
				}
			}
			else
			{
				if (Pawn->CurrentWeapon && Pawn->CurrentWeapon->WeaponData && Pawn->CurrentWeapon->WeaponData->IsA<UFortWeaponMeleeItemDefinition>())
				{
					auto Weapon = PC->Inventory->Inventory.ReplicatedEntries.Search([](FFortItemEntry& Entry)
						{ return Entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>(); });
					if (Weapon)
					{
						Pawn->EquipWeaponDefinition(Weapon->ItemDefinition->Cast<UFortWeaponItemDefinition>(), Weapon->ItemGuid);
						Pawn->PawnStopFire(0);
					}
				}
			}

			if (LineOfSight && ThreatenedTickCounter != 0 && ThreatenedTickCounter % 60 == 0)
			{
				Pawn->PawnStartFire(0);
				IncrementShots = true;
			}

			if (IncrementShots)
			{
				Shots++;
				shotCounter++;
			}

			if (Pawn->CurrentWeapon && Pawn->CurrentWeapon->WeaponData)
			{
				auto WeaponName = Pawn->CurrentWeapon->Name.ToString();
				if (WeaponName.starts_with("B_Assault_MidasDrum_Athena_") && shotCounter >= 19)
				{
					Pawn->PawnStopFire(0); shotCounter = -27; Shots = 0; IncrementShots = false;
				}
				else if (WeaponName.starts_with("B_Minigun_Athena_") && shotCounter >= 103)
				{
					Pawn->PawnStopFire(0); shotCounter = -27; Shots = 0; IncrementShots = false;
				}
				else if (WeaponName.starts_with("B_Pistol_RapidFireSMG_Athena_") && shotCounter >= 15)
				{
					Pawn->PawnStopFire(0); shotCounter = -27; Shots = 0; IncrementShots = false;
				}
				else if (Shots >= 20)
				{
					Pawn->PawnStopFire(0); Shots = 0; shotCounter = 0; IncrementShots = false;
				}
			}

			ThreatenedTickCounter++;
			break;
		}
		case EAlertLevel::LKP:
		{
			ThreatenedTickCounter = 0;
			Shots = 0;
			shotCounter = 0;
			if (!WasLKP) WasLKP = true;

			if (LKPTickCounter % 30 == 0 && LKPTickCounter != 0)
			{
				if (LKPMoveDistance < 50)
				{
					auto Pickaxe = PC->Inventory->Inventory.ReplicatedEntries.Search([](FFortItemEntry& Entry)
						{ return Entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>(); });
					if (Pickaxe)
					{
						Pawn->EquipWeaponDefinition(Pickaxe->ItemDefinition->Cast<UFortWeaponItemDefinition>(), Pickaxe->ItemGuid);
						Pawn->PawnStartFire(0);
					}
				}
				else
				{
					auto Weapon = PC->Inventory->Inventory.ReplicatedEntries.Search([](FFortItemEntry& Entry)
						{ return Entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>(); });
					if (Weapon)
					{
						Pawn->EquipWeaponDefinition(Weapon->ItemDefinition->Cast<UFortWeaponItemDefinition>(), Weapon->ItemGuid);
						Pawn->PawnStopFire(0);
					}
				}
				LKPMoveDistance = 0;
			}
			else
			{
				auto Diff = Pawn->K2_GetActorLocation() - LKPOldLoc;
				LKPMoveDistance += Diff.X + Diff.Y + Diff.Z;
			}
			LKPOldLoc = Pawn->K2_GetActorLocation();
			Pawn->UnCrouch(false);
			Pawn->StopJumping();
			jumping = false;
			PC->SetControlRotation(UKismetMathLibrary::FindLookAtRotation(BotPos, TargetPos));
			Pawn->AddMovementInput(Pawn->GetActorForwardVector(), 1.f, true);
			LKPTickCounter++;
			break;
		}
		case EAlertLevel::Alerted:
		{
			ThreatenedTickCounter = 0;
			Shots = 0;
			shotCounter = 0;
			LKPTickCounter = 0;
			Pawn->PawnStopFire(0);
			PC->SetControlRotation(UKismetMathLibrary::FindLookAtRotation(BotPos, TargetPos));
			AlertedTickCounter++;
			auto AISettings = AFortGameStateAthena::Get()->CurrentPlaylistInfo.BasePlaylist->AISettings;
			if (AISettings
				&& Pawn->FortAthenaAlertState
				&& Pawn->FortAthenaAlertState->StealthMeterTarget == 1
				&& Pawn->FortAthenaAlertState->StealthMeterTargetTime > 0
				&& LineOfSight
				&& Distance > AISettings->MinFootstepHearingRange
				&& Distance < AISettings->MaxFootstepHearingRange
				&& AlertedTickCounter % 150 == 0)
			{
				Pawn->AlertLevel = EAlertLevel::Threatened;
				Pawn->OnRep_AlertLevel();
			}
			break;
		}
		case EAlertLevel::Unaware:
		{
			ThreatenedTickCounter = 0;
			Shots = 0;
			shotCounter = 0;
			LKPTickCounter = 0;
			break;
		}
		}
	}
};

class Bots
{
	static void SpawnReinforcementSquad(AFortAthenaMutator_Gibson* Mutator, AActor* ReinforcementSpawner)
	{
		Utils::Log("meow at it");
	}

	static inline AFortPlayerPawnAthena* (*SpawnBotOG)(UFortServerBotManagerAthena* BotManager, FVector SpawnLoc, FRotator SpawnRot, UFortAthenaAIBotCustomizationData* BotData, FFortAthenaAIBotRunTimeCustomizationData& RuntimeBotData);
	static AFortPlayerPawnAthena* SpawnBot(UFortServerBotManagerAthena* BotManager, FVector SpawnLoc, FRotator SpawnRot, UFortAthenaAIBotCustomizationData* BotData, FFortAthenaAIBotRunTimeCustomizationData& RuntimeBotData)
	{
		if (__int64(_ReturnAddress()) - ImageBase == 0x20263ff)
			return SpawnBotOG(BotManager, SpawnLoc, SpawnRot, BotData, RuntimeBotData);

		auto& CL = BotData->CharacterCustomization->CustomizationLoadout;
		if (BotData->OverrideCosmeticMode == EBotDataOverrideCosmeticMode::CosmeticLibrary)
		{
			xmap<FName, FFortBotCosmeticItemSetDataTableRow*> SkinMap;
			FName Tag = RuntimeBotData.PredefinedCosmeticSetTag.TagName;
			for (auto& [Key, Val] : (TMap<FName, FFortBotCosmeticItemSetDataTableRow*>&)BotData->CosmeticCustomizationLibrary->PredefineSetsDataTable->RowMap)
			{
				if (Val->SetTag.TagName == Tag)
					SkinMap[Key] = Val;
			}
			auto Skin = Looting::PickWeighted(SkinMap, [](float Total)
				{ return ((float)rand() / 32767) * Total; });
			CL.Character = (UAthenaCharacterItemDefinition*)UKismetSystemLibrary::GetObjectFromPrimaryAssetId(Skin.second->CharacterAssetId);
		}

		AActor* SpawnLocator = Utils::SpawnActor<ADefaultPawn>(SpawnLoc, SpawnRot);
		auto Pawn = BotManager->CachedBotMutator->SpawnBot(BotData->PawnClass, SpawnLocator, SpawnLoc, SpawnRot, true);
		SpawnLocator->K2_DestroyActor();
		AFortAthenaAIBotController* PC = (AFortAthenaAIBotController*)Pawn->Controller;
		PC->CosmeticLoadoutBC = CL;
		auto HID = CL.Character->HeroDefinition;
		xmap<EFortCustomPartType, UCustomCharacterPart*> VO;
		for (auto& CVC : CL.CharacterVariantChannels)
		{
			auto Cos = (UAthenaCosmeticItemDefinition*)CVC.ItemVariantIsUsedFor;
			FPartVariantDef* Def = nullptr;
			auto var = Cos->ItemVariants.Search([CVC, Def](UFortCosmeticVariant* var) mutable
				{
					if (auto CCPV = var->Cast<UFortCosmeticCharacterPartVariant>()) {
						Def = CCPV->PartOptions.Search([CVC](FPartVariantDef& def) {
							return CVC.ActiveVariantTag.TagName == def.CustomizationVariantTag.TagName;
							});
						if (Def) return true;
					}
					return false;
				});
			if (var && Def)
			{
				for (auto& VP : Def->VariantParts)
				{
					UCustomCharacterPart* Part = VP;
					VO[Part->CharacterPartType] = Part;
				}
			}
		}
		if (HID)
		{
			for (auto& Spec : HID->Specializations)
			{
				UFortHeroSpecialization* spec = Spec;
				if (spec)
				{
					for (auto& part : spec->CharacterParts)
					{
						UCustomCharacterPart* Part = part;
						auto PartDef = VO.contains(Part->CharacterPartType) ? VO[Part->CharacterPartType] : Part;
						Pawn->ServerChoosePart(PartDef->CharacterPartType, PartDef);
					}
				}
			}
		}
		if (CL.Backpack)
		{
			for (auto& Part : CL.Backpack->CharacterParts)
				Pawn->ServerChoosePart(Part->CharacterPartType, Part);
		}

		Pawn->CosmeticLoadout = BotData->CharacterCustomization->CustomizationLoadout;
		Pawn->OnRep_CosmeticLoadout();

		BYTE False = 0;
		using FT = void (*)(UFortServerBotManagerAthena*, AFortPlayerPawnAthena*, UBehaviorTree*, void*, FConstructionBuildingInfo(*)[6], float*, void*, UFortAthenaAIBotInventoryItems*, UFortBotNameSettings*, FString*, BYTE*, uint8, uint8*, UFortAthenaAILODSettingsContainer*, FFortAthenaAIBotRunTimeCustomizationData, UFortAthenaAIBotCustomizationData*);
		using GetOrCreateLODDataT = UFortAthenaAILODSettingsContainer * (*)(UFortAthenaAIBotCustomizationData*);
		((FT)(ImageBase + 0x1fb6070))(BotManager, Pawn, BotData->BehaviorTree, 0, BotData->bOverrideConstructionBuildingInfo ? &BotData->OverrideConstructionBuildingInfo : &BotManager->ConstructionBuildingInfo, !BotData->bOverrideSkillLevel && !(BotData->bUseMatchMMRToOverrideSkillLevel && -99999.f != BotData->MatchMMRMaybe) ? 0 : &BotData->SkillLevel, BotData->bOverrideSkillSets ? &BotData->SkillSetOverrideClasses : 0, BotData->bOverrideStartupInventory ? BotData->StartupInventory : 0, BotData->bOverrideBotNameSettings ? BotData->BotNameSettings : 0, BotData->bOverrideBotIDAnalyticsSuffix ? &BotData->BotAnalyticsSuffix : 0, &False, BotData->bRequiresUniqueNetId, BotData->bHasCustomSquadId ? &BotData->CustomSquadId : 0, ((GetOrCreateLODDataT)(ImageBase + 0x1DBEC00))(BotData), RuntimeBotData, BotData);

		bool bEquipped = false;
		auto bot = new Bot(PC);
		bot->CreateInventory();

		// Set walk speed on spawn so they patrol at walk speed
		if (Pawn->CharacterMovement)
			Pawn->CharacterMovement->MaxWalkSpeed = 150.f;

		auto PatrolPaths = Utils::GetAll<AFortAthenaPatrolPath>();
		for (auto& Path : PatrolPaths)
		{
			if (Path->PatrolPoints.Num() > 0 && Path->PatrolPoints[0]->K2_GetActorLocation() == SpawnLoc)
			{
				PC->CachedPatrollingComponent->PatrolPath = Path;
				break;
			}
		}
		PatrolPaths.Free();

		if (auto* NavSystem = (UAthenaNavSystem*)UWorld::Get()->NavigationSystem)
		{
			if (PC->PathFollowingComponent && NavSystem->MainNavData)
			{
				PC->PathFollowingComponent->MyNavData = NavSystem->MainNavData;
				PC->PathFollowingComponent->OnNavDataRegistered(NavSystem->MainNavData);
				PC->PathFollowingComponent->Activate(true);
				PC->PathFollowingComponent->SetActive(true, true);
				PC->PathFollowingComponent->OnRep_IsActive();
			}
		}

		if (!PC->BrainComponent)
			PC->BrainComponent = (UBrainComponent*)UGameplayStatics::SpawnObject(UBrainComponent::StaticClass(), PC);

		if (PC->BrainComponent)
		{
			PC->BrainComponent->Activate(false);
			PC->BrainComponent->SetActive(true, false);
			PC->BrainComponent->OnRep_IsActive();
			PC->BrainComponent->RestartLogic();
		}

		for (auto& [Count, _Padding, Item] : BotData->StartupInventory->Items)
		{
			if (auto Weapon = Item->Cast<UFortWeaponRangedItemDefinition>())
			{
				if (auto Ammo = Weapon->GetAmmoWorldItemDefinition_BP())
					if (Ammo != Weapon)
						bot->GiveItem(Ammo, 69696);
			}
			auto& IE = bot->GiveItem(Item, Count);
			if (auto Weapon = Item->Cast<UFortWeaponRangedItemDefinition>())
			{
				if (!bEquipped)
				{
					bEquipped = true;
					Pawn->EquipWeaponDefinition(Weapon, IE.ItemGuid);
				}
			}
		}

		BotArray.push_back(bot);
		return Pawn;
	}

	static inline void (*OnPossessedPawnDiedOG)(AFortAthenaAIBotController* PC, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* HitComp, FName BoneName, FVector Momentum);
	static void OnPossessedPawnDied(AFortAthenaAIBotController* PC, AActor* DamagedActor, float Damage, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UPrimitiveComponent* HitComp, FName BoneName, FVector Momentum)
	{
		PlayerBots::RecordDeathContext(PC, InstigatedBy, DamageCauser);

		// Drop loot for ALL bots, only skip DangerGrape
		if (!PC->Pawn->IsA<ABP_Pawn_DangerGrape_C>())
		{
			for (auto& Entry : PC->Inventory->Inventory.ReplicatedEntries)
			{
				if (!Entry.ItemDefinition) continue;
				if (Entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>()) continue;
				if (Entry.ItemDefinition->IsA<UFortAmmoItemDefinition>()) continue; // skip ammo entries, there's already 69696 of them

				Inventory::SpawnPickup(PC->Pawn->K2_GetActorLocation(), Entry, EFortPickupSourceTypeFlag::AI, EFortPickupSpawnSource::PlayerElimination);
			}
		}


		for (auto& Bot : BotArray)
		{
			if (Bot && Bot->PC == PC)
			{
				BotArray.erase(std::remove(BotArray.begin(), BotArray.end(), Bot), BotArray.end());
				break;
			}
		}

		if (auto KillerPC = InstigatedBy->Cast<AFortPlayerControllerAthena>())
		{
			FGameplayTagContainer SourceTags;
			FGameplayTagContainer ContextTags;
			FGameplayTagContainer TargetTags = ((AFortPawn*)PC->Pawn)->GameplayTags;
			auto QuestManager = KillerPC->GetQuestManager(ESubGame::Athena);
			if (QuestManager)
			{
				QuestManager->GetSourceAndContextTags(&SourceTags, &ContextTags);
				XP::CountMap[PC->Class]++;
				XP::SendStatEvent(QuestManager, PC->Pawn, SourceTags, TargetTags, nullptr, nullptr, XP::CountMap[PC->Class], EFortQuestObjectiveStatEvent::Kill);
			}
		}

		return OnPossessedPawnDiedOG(PC, DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, HitComp, BoneName, Momentum);
	}

public:
	static void TickBots()
	{
		for (auto& bot : BotArray)
		{
			if (bot && bot->Pawn)
				bot->Tick();
		}
	}

	static void HookFunctions()
	{
		Utils::Hook(ImageBase + 0x1fc79b0, SpawnBot, SpawnBotOG);
		Utils::Hook(ImageBase + 0x1bc9e60, OnPossessedPawnDied, OnPossessedPawnDiedOG);
		Utils::Hook(ImageBase + 0xf84c60, SpawnReinforcementSquad);
	}
};