// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UEDebuggerBPLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUEDebuggerPrintStringToConsole, Log, All);

/**
 * Blueprint Exception Debug Info
 */
USTRUCT(BlueprintType)
struct UEDEBUGGER_API FBlueprintExceptionDebugInfo
{
	GENERATED_USTRUCT_BODY()

public:

	FBlueprintExceptionDebugInfo(){}

public:

	FString ToLogString();

	FString ToScreenString();

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int64 FrameCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString FrameCounterString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int64 Index;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString IndexString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TimestampString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString ActiveObjectNameString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString StackTraceString;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString ScriptCallstackString;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString StackDescriptionString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString PreFrameNameString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString NodeGraphNameString;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString NodeNameString;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString NodeTitleString;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString NodeUniqueIDString;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString NodeCustomFullNameString; // Use this for Print.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> InputParametersStrings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> OutputParametersStrings;
};

/*
 *
 */
UCLASS(meta = (ScriptName = "UEDebuggerBPLibrary"))
class UEDEBUGGER_API UUEDebuggerBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Print String to Console of Client. and optionally, to the screen or to the log.
	 *
	 * By default "PrintStringToConsole" is disabled.
	 * Use Console variable "EnableDebug.PrintStringToConsole 1" to enable all "PrintStringToConsole";
	 * Use Console variable "EnableDebug.PrintStringToConsole 0" to disable all "PrintStringToConsole";
	 * Use Console variable "EnableDebug.PrintStringToConsole CategoryName" to enable "PrintStringToConsole" for specified "CategoryName".
	 *
     * If Server call this function, it will print the string to the Console of Client if bPrintToConsole is true. Use '`' key to open Console in Editor.
	 * Prints a string to the log, and optionally, to the screen
     * If Print To Log is true, it will be visible in the Output Log window.  Otherwise it will be logged only as 'Verbose', so it generally won't show up.
     *
     * @param	WorldContextObject  Often use "this"("Self" in Blueprint) to it if "this" is a "UObject".
     * @param	InString		The string to log out
     * @param	bPrintToConsole	Whether or not to print the output to the Console of Client.
     * @param	bPrintToScreen	Whether or not to print the output to the screen
     * @param	bPrintToLog		Whether or not to print the output to the log
     * @param	TextColor       Color of Screen String (if Print to Screen is True).
     * @param	Duration		The display duration (if Print to Screen is True). Using negative number will result in loading the duration time from the config.
     * @param	CategoryName	Use Console variable "EnableDebug.PrintStringToConsole CustomCategoryName" to enable "PrintStringToConsole" for specified "CustomCategoryName".
     */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, DisplayName = "PrintStringToConsole", Keywords = "PrintString To Console", AdvancedDisplay = "2", DevelopmentOnly), Category = "UEDebugger | BlueprintLibraries")
	static void PrintStringToConsole(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool bPrintToConsole = true, bool bPrintToScreen = true, bool bPrintToLog = true, FLinearColor TextColor = FLinearColor(0.0, 1.0, 1.0), float Duration = 1.f, const FName& CategoryName = FName(TEXT("Temp")));

    /** Register ConsoleCommandGroup. Could use the TAutoConsoleCommandGroup to register a new ConsoleCommandGroup in C++ to.
     */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, DisplayName = "RegisterConsoleCommandGroup", Keywords = "RegisterConsoleCommandGroup"), Category = "UEDebugger | BlueprintLibraries | ConsoleCommandGroup")
    static void RegisterConsoleCommandGroup(UObject* WorldContextObject, const FString& Name, const TArray<FString>& ConsoleCommandsToEnable, const TArray<FString>& ConsoleCommandsToDisable, bool& Succeed);

    /** Unregister ConsoleCommandGroup by name. Could use the TAutoConsoleCommandGroup to register a new ConsoleCommandGroup in C++ to.
     */
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, DisplayName = "UnregisterConsoleCommandGroup", Keywords = "UnregisterConsoleCommandGroup"), Category = "UEDebugger | BlueprintLibraries | ConsoleCommandGroup")
    static void UnregisterConsoleCommandGroup(UObject* WorldContextObject, const FString& Name, bool& Succeed);

    /** Get all names ConsoleCommandGroups. Could use the TAutoConsoleCommandGroup to register a new ConsoleCommandGroup in C++ or use RegisterConsoleCommandGroup in blueprint.
     *  One of WorldContextObject and Player should be valid.
     */
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, DisplayName = "GetAllConsoleCommandGroupNames", Keywords = "GetAllConsoleCommandGroupNames"), Category = "UEDebugger | BlueprintLibraries | ConsoleCommandGroup")
	static void GetAllConsoleCommandGroupNames(UObject* WorldContextObject, APlayerController* Player, TArray<FString>& AllConsoleCommandGroupNames);

    /** Enable ConsoleCommandGroup by name. Could use the TAutoConsoleCommandGroup to register a new ConsoleCommandGroup in C++ or use RegisterConsoleCommandGroup in blueprint.
     *  One of WorldContextObject and Player should be valid.
     */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, DisplayName = "EnableConsoleCommandGroup", Keywords = "EnableConsoleCommandGroup"), Category = "UEDebugger | BlueprintLibraries | ConsoleCommandGroup")
	static void EnableConsoleCommandGroup(UObject* WorldContextObject, APlayerController* Player, const FString& ConsoleCommandGroupName);

    /** Disable ConsoleCommandGroup by name. Could use the TAutoConsoleCommandGroup to register a new ConsoleCommandGroup in C++ or use RegisterConsoleCommandGroup in blueprint.
     *  One of WorldContextObject and Player should be valid.
     */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, DisplayName = "DisableConsoleCommandGroup", Keywords = "DisableConsoleCommandGroup"), Category = "UEDebugger | BlueprintLibraries | ConsoleCommandGroup")
	static void DisableConsoleCommandGroup(UObject* WorldContextObject, APlayerController* Player, const FString& ConsoleCommandGroupName);

public:

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "DisableConsoleCommandGroup"), Category = "UEDebugger | BlueprintLibraries | UObject")
    static void GetWorldFromObject(UObject* InObject, UObject*& OutWorldContextObject, UWorld*& OutWorld);

	/** Returns the value of GFrameNumber which Incremented once per frame before the scene is being rendered. In split screen mode this is incremented once for all views (not for each view). */
	UFUNCTION(BlueprintPure, Category = "UEDebugger | BlueprintLibraries | Utilities")
    static int64 GetFrameNumber();

  public:

	/**
     * Prints a string to the log, and optionally, to the screen
     * If Print To Log is true, it will be visible in the Output Log window.  Otherwise it will be logged only as 'Verbose', so it generally won't show up.
     *
     * @param	InString		The string to log out
     * @param	bPrintToScreen	Whether or not to print the output to the screen
     * @param	bPrintToLog		Whether or not to print the output to the log
     * @param	bPrintToConsole	Whether or not to print the output to the console
     * @param	TextColor		Whether or not to print the output to the console
     * @param	Duration		The display duration (if Print to Screen is True). Using negative number will result in loading the duration time from the config.
     */
	// UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "Custom Print String", AdvancedDisplay = "2"), Category = "UEDebugger | BlueprintLibraries")
	static void CustomPrintString(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool bPrintToScreen = true, bool bPrintToLog = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

};

