// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UEDebuggerBPLibrary.h"
#include "UEDebugger.h"
#include "UnrealEngine.h"
#include "EngineGlobals.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/Console.h"
#include "UEDebuggerConsoleCommandGroup.h"

DEFINE_LOG_CATEGORY(LogUEDebuggerPrintStringToConsole);

static TAutoConsoleVariable<FString> CVarEnableDebugPrintStringToConsole(
	TEXT("EnableDebug.PrintStringToConsole"),
	TEXT("0"),
	TEXT("Toggle PrintStringToConsole.\n")
	TEXT(" 0: Disable PrintStringToConsole.\n")
	TEXT(" 1: Enable PrintStringToConsole.")
	TEXT(" CategoryName: Enable PrintStringToConsole for specified CategoryName."),
	ECVF_Default);

static TAutoConsoleCommandGroup CCGStatFPSAndUNIT(
	TEXT("CCGStatFPSAndUNIT"),
	{TEXT("stat fps"), TEXT("stat unit") },
	{TEXT("stat fps"), TEXT("stat unit") });

FString FBlueprintExceptionDebugInfo::ToLogString()
{
	FString InputParametersString;
	for (int32 IndexTemp = 0; IndexTemp < InputParametersStrings.Num(); IndexTemp++)
	{
		if (IndexTemp == 0)
		{
			InputParametersString += TEXT("InputParameters: \n");
		}
		InputParametersString += InputParametersStrings[IndexTemp];
		if (IndexTemp < InputParametersStrings.Num() - 1)
		{
			InputParametersString += TEXT("\n");
		}
	}

	FString OutputParametersString;
	for (int32 IndexTemp = 0; IndexTemp < OutputParametersStrings.Num(); IndexTemp++)
	{
		if (IndexTemp == 0)
		{
			OutputParametersString += TEXT("OutputParameters: \n");
		}
		OutputParametersString += OutputParametersStrings[IndexTemp];
		if (IndexTemp < OutputParametersStrings.Num() - 1)
		{
			OutputParametersString += TEXT("\n");
		}
	}

	FString ParametersString;
	if (!InputParametersString.IsEmpty())
	{
		ParametersString = TEXT("\n") + InputParametersString;
	}

	if (!OutputParametersString.IsEmpty())
	{
		ParametersString += TEXT("\n") + OutputParametersString;
	}

	FString ReturnValue = FString::Printf(TEXT("[%s(%s)] [%s.%s.\"%s\"] %s \nStackTrace:\n%s"),
		*FrameCounterString,
		*IndexString,
		*PreFrameNameString,
		*NodeGraphNameString,
		*NodeCustomFullNameString,
		*ParametersString,
		*ScriptCallstackString
		);
	return ReturnValue;
}

FString FBlueprintExceptionDebugInfo::ToScreenString()
{
	FString InputParametersString;
	for (int32 IndexTemp = 0; IndexTemp < InputParametersStrings.Num(); IndexTemp++)
	{
		if (IndexTemp == 0)
		{
			InputParametersString += TEXT("Input:{ ");
		}
		InputParametersString += InputParametersStrings[IndexTemp];
		if (IndexTemp < InputParametersStrings.Num() - 1)
		{
			InputParametersString += TEXT("; ");
		}
		else
		{
			InputParametersString += TEXT("}");
		}
	}

	FString OutputParametersString;
	for (int32 IndexTemp = 0; IndexTemp < OutputParametersStrings.Num(); IndexTemp++)
	{
		if (IndexTemp == 0)
		{
			OutputParametersString += TEXT("Output:{ ");
		}
		OutputParametersString += OutputParametersStrings[IndexTemp];
		if (IndexTemp < OutputParametersStrings.Num() - 1)
		{
			OutputParametersString += TEXT("; ");
		}
		else
		{
			OutputParametersString += TEXT("}");
		}
	}

	FString PreFrameNameLeftString;
	FString PreFrameNameRightString;
	PreFrameNameString.Split(TEXT("."), &PreFrameNameLeftString, &PreFrameNameRightString);

	FString ReturnValue = FString::Printf(TEXT("[%s(%s)]> [%s.%s.%s.\"%s\"] %s %s"),
		*FrameCounterString,
		*IndexString,
		*ActiveObjectNameString,
		*PreFrameNameRightString,
		*NodeGraphNameString,
		*NodeCustomFullNameString,
		*InputParametersString,
		*OutputParametersString
	);
	return ReturnValue;
}

UUEDebuggerBPLibrary::UUEDebuggerBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UUEDebuggerBPLibrary::PrintStringToConsole(UObject* WorldContextObject, const FString& InString, bool bPrintToConsole, bool bPrintToScreen, bool bPrintToLog, FLinearColor TextColor, float Duration, const FName& CategoryName)
{
#if !NO_LOGGING
	static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("EnableDebug.PrintStringToConsole"));
	if (!CVar)
	{
		return;
	}

	FString ValueOfCVar = CVar->GetString().TrimStartAndEnd().ToUpper();

	if (!(ValueOfCVar.Equals(TEXT("1")) || ValueOfCVar.Equals(TEXT("TRUE")) || ValueOfCVar.Equals(CategoryName.ToString().TrimStartAndEnd().ToUpper())))
	{
		return;
	}

	FString StringWithPrefix = TEXT("[") + CategoryName.ToString() + TEXT("] ") + InString;
	
	UUEDebuggerBPLibrary::CustomPrintString(WorldContextObject, StringWithPrefix, bPrintToScreen, bPrintToLog, TextColor, Duration);

	if (bPrintToConsole)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

		if (!World)
		{
			return;
		}

		// APlayerController* MyPC = World->GetGameInstance()->GetFirstLocalPlayerController();

		FString NetMode = World->GetNetMode() == ENetMode::NM_Standalone ? TEXT("") : (World->GetNetMode() == ENetMode::NM_Client ? TEXT("Client: ") : TEXT("Server: "));

		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController != nullptr)
			{
				PlayerController->ClientMessage(NetMode + StringWithPrefix, CategoryName, Duration);
			}
		}
	}
#endif
}

void UUEDebuggerBPLibrary::CustomPrintString(UObject* WorldContextObject, const FString& InString, bool bPrintToScreen, bool bPrintToLog, FLinearColor TextColor, float Duration)
{
// #if !(UE_BUILD_SHIPPING || NO_LOGGING) // Do not Print in Shipping or NO_LOGGING
#if !(NO_LOGGING) // Do not Print in NO_LOGGING

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client%d: "), GPlayInEditorID);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server: "));
				break;
			case NM_Standalone:
				break;
			}
		}
	}

	const FString FinalDisplayString = Prefix + InString;
	FString FinalLogString = FinalDisplayString;

	static const FBoolConfigValueHelper DisplayPrintStringSource(TEXT("Kismet"), TEXT("bLogPrintStringSource"), GEngineIni);
	if (DisplayPrintStringSource)
	{
		const FString SourceObjectPrefix = FString::Printf(TEXT("[%s] "), *GetNameSafe(WorldContextObject));
		FinalLogString = SourceObjectPrefix + FinalLogString;
	}

	if (bPrintToLog)
	{
		UE_LOG(LogUEDebuggerPrintStringToConsole, Log, TEXT("%s"), *FinalLogString);

		APlayerController* PC = (WorldContextObject ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : NULL);
		ULocalPlayer* LocalPlayer = (PC ? Cast<ULocalPlayer>(PC->Player) : NULL);
		if (LocalPlayer && LocalPlayer->ViewportClient && LocalPlayer->ViewportClient->ViewportConsole)
		{
			LocalPlayer->ViewportClient->ViewportConsole->OutputText(FinalDisplayString);
		}
	}
	else
	{
		UE_LOG(LogUEDebuggerPrintStringToConsole, Verbose, TEXT("%s"), *FinalLogString);
	}

	// Also output to the screen, if possible
	if (bPrintToScreen)
	{
		if (GAreScreenMessagesEnabled)
		{
			if (GConfig && Duration < 0)
			{
				GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
			}
			GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, TextColor.ToFColor(true), FinalDisplayString);
		}
		else
		{
			UE_LOG(LogUEDebuggerPrintStringToConsole, VeryVerbose, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
		}
	}
#endif
}

void UUEDebuggerBPLibrary::RegisterConsoleCommandGroup(UObject* WorldContextObject, const FString& Name, const TArray<FString>& ConsoleCommandsToEnable, const TArray<FString>& ConsoleCommandsToDisable, bool& Succeed)
{
	Succeed = false;
	IConsoleCommandGroupObject* ConsoleCommandGroupObject = IConsoleCommandGroupManager::Get().FindConsoleCommandGroupObject(Name);

	if (ConsoleCommandGroupObject)
	{
		return;
	}

	ConsoleCommandGroupObject = IConsoleCommandGroupManager::Get().RegisterConsoleCommandGroupObject(Name, ConsoleCommandsToEnable, ConsoleCommandsToDisable);

	if (ConsoleCommandGroupObject)
	{
		Succeed = true;
	}
}

void UUEDebuggerBPLibrary::UnregisterConsoleCommandGroup(UObject* WorldContextObject, const FString& Name, bool& Succeed)
{
	Succeed = false;
	IConsoleCommandGroupObject* ConsoleCommandGroupObject = IConsoleCommandGroupManager::Get().FindConsoleCommandGroupObject(Name);

	if (!ConsoleCommandGroupObject)
	{
		return;
	}

	IConsoleCommandGroupManager::Get().UnregisterConsoleCommandGroupObject(Name);

	Succeed = true;
}

void UUEDebuggerBPLibrary::GetAllConsoleCommandGroupNames(UObject* WorldContextObject, APlayerController* Player, TArray<FString>& AllConsoleCommandGroupNames)
{
	AllConsoleCommandGroupNames = IConsoleCommandGroupManager::Get().GetAllConsoleCommandGroupObjectNames();
}

void UUEDebuggerBPLibrary::EnableConsoleCommandGroup(UObject* WorldContextObject, APlayerController* Player, const FString& ConsoleCommandGroupName)
{
	IConsoleCommandGroupObject* ConsoleCommandGroupObject = IConsoleCommandGroupManager::Get().FindConsoleCommandGroupObject(ConsoleCommandGroupName);
	if (ConsoleCommandGroupObject)
	{
		ConsoleCommandGroupObject->Enable(WorldContextObject, Player);
	}
}

void UUEDebuggerBPLibrary::DisableConsoleCommandGroup(UObject* WorldContextObject, APlayerController* Player, const FString& ConsoleCommandGroupName)
{
	IConsoleCommandGroupObject* ConsoleCommandGroupObject = IConsoleCommandGroupManager::Get().FindConsoleCommandGroupObject(ConsoleCommandGroupName);
	if (ConsoleCommandGroupObject)
	{
		ConsoleCommandGroupObject->Disable(WorldContextObject, Player);
	}
}

void UUEDebuggerBPLibrary::GetWorldFromObject(UObject* InObject, UObject*& OutWorldContextObject, UWorld*& OutWorld)
{
	if (InObject)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(InObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			OutWorldContextObject = InObject;
			OutWorld = World;
			return;
		}
	}

	OutWorldContextObject = nullptr;
	OutWorld = nullptr;
	return;
}

int64 UUEDebuggerBPLibrary::GetFrameNumber()
{
	return (int64)GFrameNumber;
}