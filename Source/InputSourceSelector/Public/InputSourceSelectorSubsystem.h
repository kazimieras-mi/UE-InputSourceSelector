// Copyright (c) 2023 Hitbox Initiative. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InputSourceSelectorTypes.h"
#include "InputCoreTypes.h"
#include "InputSourceSelectorSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputSourceSelectorInputTypeChange, EInputSourceSelectorInputType, OldInputType, EInputSourceSelectorInputType, NewInputType);

/**
 * Keeps track of the current input source and provides a delegate for anyone else to listen for input source changes.
 * Very useful for various widgets that must update when a player picks up the controller or goes back to mouse and keyboard/touch.
 */
UCLASS(BlueprintType)
class INPUTSOURCESELECTOR_API UInputSourceSelectorSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets the last input device type.
	 * @param WorldContext					World for which to get the last input used.
	 * @param bUpgradeUnknownToDefaults		Whether to upgrade to platform defaults if unknown/no inputs have been given yet.
	 * @return								The last input type, for example: keyboard, gamepad, touch.
	 */
	UFUNCTION(BlueprintPure, meta=(WorldContext=WorldContext))
	static EInputSourceSelectorInputType GetLastInputType(const UObject* WorldContext, bool bUpgradeUnknownToDefaults = true);

	/**
	 * @brief Determines the input type for a key.
	 * @param Key					The input key.
	 * @return						The input type for this key.
	 */
	static EInputSourceSelectorInputType GetInputTypeFromKey(const FKey& Key);

	/**
	 * @brief Gets a list of keys to be displayed for the current input mode by action or axis mapping.
	 * @param WorldContext			The world context.
	 * @param Name					The action name.
	 * @return						The key or chord list on the appropriate input device to trigger the action.
	 */
	UFUNCTION(BlueprintPure, meta=(WorldContext=WorldContext))
	static TArray<FKey> GetKeysByInputBindingName(const UObject* WorldContext, FName Name);

	/** Called when the input type changes. */
	UPROPERTY(BlueprintAssignable)
	FOnInputSourceSelectorInputTypeChange OnInputTypeChange;
	
	// Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End USubsystem Interface

	// Begin FTickableGameObject Interface
	virtual TStatId GetStatId() const override { return GetStatID(); }
	virtual bool IsTickable() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	// End FTickableGameObject Interface

private:
	/** Handle receiving input. */
	UFUNCTION()
	void HandleInput(FKey Key);
	
	/** Ensures that input bindings are set up. */
	static void SetUpInputBindings();

	/** Checks that the current input component is selected correctly. */
	void BindToInputComponent();

	/** The last known input type. */
	EInputSourceSelectorInputType LastInputType = EInputSourceSelectorInputType::Unknown;

	/** Name of the action we will bind to listen for inputs. */
	static FName ActionName;

	/** The current input component. */
	TWeakObjectPtr<UInputComponent> FoundInputComponent = nullptr;
};
