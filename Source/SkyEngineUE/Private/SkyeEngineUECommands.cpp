#include "SkyEngineUECommands.h"

#define LOCTEXT_NAMESPACE "FSkyEngineUEModule"

void FSkyEngineUECommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SkyEngineUE", "SkyEngine Export Configurations", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE