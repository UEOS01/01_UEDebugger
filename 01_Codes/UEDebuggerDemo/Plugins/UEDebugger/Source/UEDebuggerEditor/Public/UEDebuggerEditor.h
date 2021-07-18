
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "UEDebuggerBPLibrary.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUEDebuggerEditorModule, Log, All);

class UEDEBUGGEREDITOR_API FUEDebuggerEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:

	static void OnScriptExceptionCustom(const UObject* ActiveObject, const FFrame& StackFrame, const FBlueprintExceptionInfo& Info);

	static bool GetBlueprintExceptionDebugInfo(const UObject* ActiveObject, const FFrame& StackFrame, const FBlueprintExceptionInfo& Info, FBlueprintExceptionDebugInfo& OutBlueprintExceptionDebugInfo);
};
