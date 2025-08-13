// Copyright (c) 2023 Hitbox Initiative. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * A simplified list of input sources, which the Input Source Selector plugin can auto-detect
 * and inform other systems of.
 */
UENUM(BlueprintType)
enum class EInputSourceSelectorInputType : uint8
{
	Unknown,
	MouseAndKeyboard,
	Gamepad,
	Touch,
	Gesture
};