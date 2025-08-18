#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SkyEngineUEStyle.h"

class FSkyEngineUECommands : public TCommands<FSkyEngineUECommands>
{
public:

	FSkyEngineUECommands()
		: TCommands<FSkyEngineUECommands>(TEXT("SkyEnginePlugin"), NSLOCTEXT("Contexts", "SkyEnginePlugin", "SkyEnginePlugin Plugin"), NAME_None, FSkyEngineUEStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};