// Copyright (c) 2023 Hitbox Initiative. All rights reserved.

#include "InputSourceSelectorSubsystem.h"

#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

FName UInputSourceSelectorSubsystem::ActionName = TEXT("UInputSourceSelectorSubsystem.InputSourceListenerAction");

EInputSourceSelectorInputType UInputSourceSelectorSubsystem::GetLastInputType(const UObject* WorldContext, const bool bUpgradeUnknownToDefaults /* = true */)
{
	EInputSourceSelectorInputType RetVal = EInputSourceSelectorInputType::Unknown;
	
	if(const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull))
	{
		if(const UInputSourceSelectorSubsystem* Subsystem = World->GetSubsystem<UInputSourceSelectorSubsystem>())
		{
			RetVal = Subsystem->LastInputType;
		}
	}
	
	if(bUpgradeUnknownToDefaults && RetVal == EInputSourceSelectorInputType::Unknown)
	{
		static TMap<FString, EInputSourceSelectorInputType> PlatformDefaults =
			{
				{"Windows", EInputSourceSelectorInputType::MouseAndKeyboard},
				{"Mac", EInputSourceSelectorInputType::MouseAndKeyboard},
				{"Linux", EInputSourceSelectorInputType::MouseAndKeyboard},
				{"IOS", EInputSourceSelectorInputType::Touch},
				{"iOS", EInputSourceSelectorInputType::Touch},
				{"Android", EInputSourceSelectorInputType::Touch},
			};

		if(const FString PlatformName = UGameplayStatics::GetPlatformName(); PlatformDefaults.Contains(PlatformName))
		{
			RetVal = PlatformDefaults[PlatformName];	
		}
		else
		{
			RetVal = EInputSourceSelectorInputType::Gamepad;
		}
	}

	return RetVal;
}

EInputSourceSelectorInputType UInputSourceSelectorSubsystem::GetInputTypeFromKey(const FKey& Key)
{
	if(Key.IsGamepadKey())
	{
		return EInputSourceSelectorInputType::Gamepad;
	}

	if(Key.IsGesture())
	{
		return EInputSourceSelectorInputType::Gesture;
	}

	if(Key.IsTouch())
	{
		return EInputSourceSelectorInputType::Touch;
	}
	
	return EInputSourceSelectorInputType::MouseAndKeyboard;
}

TArray<FKey> UInputSourceSelectorSubsystem::GetKeysByInputBindingName(const UObject* WorldContext, const FName Name)
{
	if(const UInputSettings* InputSettings = UInputSettings::GetInputSettings())
	{
		const EInputSourceSelectorInputType DesiredInputType = GetLastInputType(WorldContext);
		TArray<FKey> Keys;
		{
			TArray<FInputActionKeyMapping> ActionMappings;
			InputSettings->GetActionMappingByName(Name,ActionMappings);
			for(auto& Mapping : ActionMappings)
			{
				if(GetInputTypeFromKey(Mapping.Key) == DesiredInputType)
				{
					Keys.Add(Mapping.Key);
				}
			}
		}
		{	
			TArray<FInputAxisKeyMapping> AxisMappings;
			InputSettings->GetAxisMappingByName(Name,AxisMappings);
			for(auto& Mapping : AxisMappings)
			{
				if(GetInputTypeFromKey(Mapping.Key) == DesiredInputType)
				{
					Keys.Add(Mapping.Key);
				}
			}
		}
		return Keys;
	}

	return {};
}

void UInputSourceSelectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SetUpInputBindings();

	BindToInputComponent();
}

void UInputSourceSelectorSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	BindToInputComponent();
}

void UInputSourceSelectorSubsystem::SetUpInputBindings()
{
	if(UInputSettings* Settings = UInputSettings::GetInputSettings())
	{
		for(auto& Mapping : Settings->GetActionMappings())
		{
			if(Mapping.ActionName == ActionName)
			{
				return;
			}
		}
		
		Settings->AddActionMapping(FInputActionKeyMapping(ActionName, EKeys::AnyKey), true);
	}
}

void UInputSourceSelectorSubsystem::BindToInputComponent()
{
	if(FoundInputComponent.IsValid())
	{
		if(const APlayerController* PlayerController = Cast<APlayerController>(FoundInputComponent->GetOwner()))
		{
			if(PlayerController->IsPrimaryPlayer())
			{
				// We have the right one!
				return;
			}

			// It seems like we have a valid player controller, but it's not the primary player. We should unbind in case we were bound.
			FoundInputComponent->RemoveActionBinding(ActionName, IE_Pressed);
		}
	}

	// Looks like we need to find the input controller, let's try and find the first player's one.
	if(const AController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if(UInputComponent* PlayerInputComponent = PlayerController->GetComponentByClass<UInputComponent>())
		{
			FoundInputComponent = PlayerInputComponent;
			FInputActionBinding NonConsumingBinding(ActionName, IE_Pressed);
			NonConsumingBinding.bConsumeInput = false;
			NonConsumingBinding.ActionDelegate.BindDelegate(this, &UInputSourceSelectorSubsystem::HandleInput);
			PlayerInputComponent->AddActionBinding(NonConsumingBinding);
		}
	}
}

void UInputSourceSelectorSubsystem::HandleInput(FKey Key)
{
	if(const EInputSourceSelectorInputType NewInputType = GetInputTypeFromKey(Key); NewInputType != LastInputType)
	{
		const EInputSourceSelectorInputType CachedInputType = LastInputType;
		LastInputType = NewInputType;
		OnInputTypeChange.Broadcast(CachedInputType, NewInputType);
		
	}
}
