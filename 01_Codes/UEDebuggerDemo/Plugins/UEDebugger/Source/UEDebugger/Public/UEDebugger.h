// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "UEDebuggerBPLibrary.h"

// #if NO_LOGGING
// 	#define (WorldContextObject, InString, bPrintToConsole, bPrintToScreen, bPrintToLog, TextColor, Duration, CategoryName)  
// #else

#if defined(WITH_ASAN) && WITH_ASAN == 1 && PLATFORM_IOS
#define UE_PSTC(WorldContextObject, InString, bPrintToConsole, bPrintToScreen, bPrintToLog, TextColor, Duration, CategoryName)  
#else
    /**
     * A  macro that outputs a formatted message to Console of Client. and optionally, to the screen or to the log.
     *
     * By default "UE_PSTC" is disabled.
     * Use Console variable "EnableDebug.PrintStringToConsole 1" to enable all "UE_PSTC";
     * Use Console variable "EnableDebug.PrintStringToConsole 0" to disable all "UE_PSTC";
     * Use Console variable "EnableDebug.PrintStringToConsole CategoryName" to enable "UE_PSTC" for specified "CategoryName".
     *
     * Example01:
     * {    
     *     FName CategoryName = FName("PSTC_ShowID");
     *     int32 Id = 9;
     *     FString CallbackScript = FString::Printf(TEXT("PSTC_Name = %s | ID = %d"), *CategoryName.ToString(), Id);
     *     UE_PSTC(this, CallbackScript, true, true, true, FLinearColor(1.0, 0.0, 0.0), 1.0f, CategoryName);
     * }
     *
     * If Server call this function, it will print the string to the Console of Client if bPrintToConsole is true. Use '~' key to open Console in Editor.
     * Prints a string to the log, and optionally, to the screen
     * If Print To Log is true, it will be visible in the Output Log window.  Otherwise it will be logged only as 'Verbose', so it generally won't show up.
     *
     * @param	WorldContextObject  Often use "this" to it if "this" is a "UObject".
     * @param	InString		The string to log out
     * @param	bPrintToConsole	Whether or not to print the output to the Console of Client.
     * @param	bPrintToScreen	Whether or not to print the output to the screen
     * @param	bPrintToLog		Whether or not to print the output to the log
     * @param	TextColor       Color of Screen String (if Print to Screen is True).
     * @param	Duration		The display duration (if Print to Screen is True). Using negative number will result in loading the duration time from the config.
     * @param	CategoryName	Use Console variable "EnableDebug.PrintStringToConsole CustomCategoryName" to enable "PrintStringToConsole" for specified "CustomCategoryName".
     */
#define UE_PSTC(WorldContextObject, InString, bPrintToConsole, bPrintToScreen, bPrintToLog, TextColor, Duration, CategoryName) \
	{ \
		UUEDebuggerBPLibrary::PrintStringToConsole(WorldContextObject, InString, bPrintToConsole, bPrintToScreen, bPrintToLog, TextColor, Duration, CategoryName);\
	}
#endif
// #endif

class UEDEBUGGER_API FUEDebuggerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};