
#include "UEDebuggerEditor.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "KismetCompilerModule.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "UEDebuggerBPLibrary.h"
#include "WatchPointViewer.h"

#define LOCTEXT_NAMESPACE "FUEDebuggerEditorModule"

DEFINE_LOG_CATEGORY(LogUEDebuggerEditorModule);

static int64 GPreFrameCounter = 0;
static float BreakpointScreenStringDuration = 10.0f;
static FAutoConsoleCommandWithWorldAndArgs CVarBreakpointType(
	TEXT("UEDebugger.BreakpointType"),
	TEXT("Arguments: 0/1\n")
	TEXT("0: Blueprint Breakpoint is default type. 1: Blueprint Breakpoint is used to PrintString."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			BreakpointScreenStringDuration = 10.0f;

			if (Args.Num() != 0)
			{
				const int32 Value = FCString::Atoi(*Args[0]);
				if (Value == 1)
				{
					FBlueprintCoreDelegates::OnScriptException.Clear();
					/** Delegate that gets called when a script exception occurs */
					FBlueprintCoreDelegates::OnScriptException.AddStatic(&FUEDebuggerEditorModule::OnScriptExceptionCustom);
				}
				else
				{
					FBlueprintCoreDelegates::OnScriptException.Clear();
					/** Delegate that gets called when a script exception occurs */
					FBlueprintCoreDelegates::OnScriptException.AddStatic(&FKismetDebugUtilities::OnScriptException);
				}

				if (Args.IsValidIndex(1))
				{
					BreakpointScreenStringDuration = FCString::Atof(*Args[1]);
					BreakpointScreenStringDuration = BreakpointScreenStringDuration < 0.0f ? 0.0f : BreakpointScreenStringDuration;
				}

				UE_LOG(LogUEDebuggerEditorModule, Log, TEXT("UEDebugger.BreakpointType is %s"), *Args[0]);
			}
		}),
	ECVF_Cheat);


void FUEDebuggerEditorModule::StartupModule()
{


}

void FUEDebuggerEditorModule::ShutdownModule()
{

}


void FUEDebuggerEditorModule::OnScriptExceptionCustom(const UObject* ActiveObject, const FFrame& StackFrame, const FBlueprintExceptionInfo& Info)
{
	if (!ActiveObject)
	{
		return;
	}

	if (Info.GetType() != EBlueprintExceptionType::Breakpoint)
	{
		return;
	}

	FBlueprintExceptionDebugInfo BlueprintExceptionDebugInfo;
	bool ValidDebugInfo = FUEDebuggerEditorModule::GetBlueprintExceptionDebugInfo(ActiveObject, StackFrame, Info, BlueprintExceptionDebugInfo);

	if (!ValidDebugInfo)
	{
		return;
	}

	FString LogString = BlueprintExceptionDebugInfo.ToLogString();
	FString ScreenString = BlueprintExceptionDebugInfo.ToScreenString();

	UObject* ActiveObjectTemp = const_cast<UObject*>(ActiveObject);

	if (GPreFrameCounter != GFrameCounter)
	{
		FString FrameCounterInfoForLog = FString::Printf(TEXT("\n \n=================================================== FrameCounter: %d ==================================================="), GFrameCounter);
		UUEDebuggerBPLibrary::CustomPrintString(ActiveObjectTemp, FrameCounterInfoForLog, false, true, FLinearColor::Yellow, BreakpointScreenStringDuration);
		FString FrameCounterInfoForScreen = FString::Printf(TEXT("======== FrameCounter: %d ========"), GFrameCounter);
		UUEDebuggerBPLibrary::CustomPrintString(ActiveObjectTemp, FrameCounterInfoForScreen, true, false, FLinearColor::Yellow, BreakpointScreenStringDuration);
	}
	GPreFrameCounter = GFrameCounter;
	
	UUEDebuggerBPLibrary::CustomPrintString(ActiveObjectTemp, LogString, false, true, FLinearColor::Red, BreakpointScreenStringDuration);
	UUEDebuggerBPLibrary::CustomPrintString(ActiveObjectTemp, ScreenString, true, false, FLinearColor::Red, BreakpointScreenStringDuration);
}

bool FUEDebuggerEditorModule::GetBlueprintExceptionDebugInfo(const UObject* ActiveObject, const FFrame& StackFrame, const FBlueprintExceptionInfo& Info, FBlueprintExceptionDebugInfo& OutBlueprintExceptionDebugInfo)
{
	if (!ActiveObject || Info.GetType() != EBlueprintExceptionType::Breakpoint)
	{
		OutBlueprintExceptionDebugInfo = FBlueprintExceptionDebugInfo();
		return false;
	}

	// Infos
	int64 FrameCounter = (int64)GFrameCounter;
	FString FrameCounterString = FString::Printf(TEXT("%d"), FrameCounter);

	static int64 Index = 0;
	FString IndexString = FString::Printf(TEXT("%d"), Index);
	Index++;

	FString TimestampString = FPlatformTime::StrTimestamp();

	FString ActiveObjectNameString = ActiveObject->GetName();

	FString StackTraceString = StackFrame.GetStackTrace();
	FString ScriptCallstackString = StackFrame.GetScriptCallstack();
	FString StackDescriptionString = StackFrame.GetStackDescription();

	FString PreFrameNameString;

	FString NodeGraphNameString;

	FString NodeNameString;
	FString NodeTitleString;
	FString NodeUniqueIDString;
	FString NodeCustomFullNameString; // Use this for Print.

	TArray<FString> WatchedPinsStrings;

	TArray<FString> InputParametersStrings;
	TArray<FString> OutputParametersStrings;

	FString OwnerNameString;
	FString InstigatorNameString;
	FString InstigatorControllerNameString;

	//

	// FKismetDebugUtilitiesData& Data = FKismetDebugUtilitiesData::Get();
	const int32 BreakpointOffset = StackFrame.Code - StackFrame.Node->Script.GetData() - 1;

	const TArray<const FFrame*>& ScriptStack = FBlueprintContextTracker::Get().GetScriptStack();

	if (ScriptStack.IsValidIndex(0))
	{
		const FFrame* PreFrame = ScriptStack[0];
		if (PreFrame)
		{
			PreFrameNameString = PreFrame->GetStackDescription();
		}
	}

	UObject* BlueprintInstance = StackFrame.Object;
	UClass* Class = BlueprintInstance ? BlueprintInstance->GetClass() : nullptr;
	UBlueprint* BlueprintObj = (Class ? Cast<UBlueprint>(Class->ClassGeneratedBy) : nullptr);


	// Find the node that generated the code which we hit
	UEdGraphNode* NodeStoppedAt = FKismetDebugUtilities::FindSourceNodeForCodeLocation(ActiveObject, StackFrame.Node, BreakpointOffset, /*bAllowImpreciseHit=*/ true);

	if (NodeStoppedAt)
	{
		if (NodeStoppedAt->GetGraph())
		{
			NodeGraphNameString = NodeStoppedAt->GetGraph()->GetName();
		}
		
		NodeNameString = NodeStoppedAt->GetDescriptiveCompiledName();
		NodeTitleString = NodeStoppedAt->GetNodeTitle(ENodeTitleType::ListView).ToString();
		NodeUniqueIDString = FString::Printf(TEXT("%d"), NodeStoppedAt->GetUniqueID());

		NodeCustomFullNameString = NodeTitleString + TEXT("(") + NodeUniqueIDString + TEXT(")");

		if(BlueprintObj)
		{
			// WatchViewer::UpdateInstancedWatchDisplay();

			// We have a valid instance, iterate over all the watched pins and create rows for them
			for (const FEdGraphPinReference& PinRef : BlueprintObj->WatchedPins)
			{
				UEdGraphPin* Pin = PinRef.Get();

				// FText GraphName = FText::FromString(Pin->GetOwningNode()->GetGraph()->GetName());
				// FText NodeName = Pin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView);

				FDebugInfo DebugInfo;
				const FKismetDebugUtilities::EWatchTextResult WatchStatus = FKismetDebugUtilities::GetDebugInfo(DebugInfo, BlueprintObj, BlueprintInstance, Pin);

				if (WatchStatus == FKismetDebugUtilities::EWTR_Valid)
				{
					// DebugInfo.Type
					FString PinInfoString = TEXT("\"") + DebugInfo.DisplayName.ToString() + TEXT("\" = \"") + DebugInfo.Value.ToString() + TEXT("\"");
					WatchedPinsStrings.Add(PinInfoString);
				}
			}
		}

		const TArray<UEdGraphPin*>& Pins = NodeStoppedAt->GetAllPins();

		for (UEdGraphPin* Pin : Pins)
		{
			if (!Pin)
			{
				continue;
			}

			// FText GraphName = FText::FromString(Pin->GetOwningNode()->GetGraph()->GetName());
			// FText NodeName = Pin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView);

			FDebugInfo DebugInfo;
			const FKismetDebugUtilities::EWatchTextResult WatchStatus = FKismetDebugUtilities::GetDebugInfo(DebugInfo, BlueprintObj, BlueprintInstance, Pin);

			if (WatchStatus == FKismetDebugUtilities::EWTR_Valid)
			{
				// DebugInfo.Type
				FString PinInfoString = TEXT("\"") + DebugInfo.DisplayName.ToString() + TEXT("\" = \"") + DebugInfo.Value.ToString() + TEXT("\"");
				if (Pin->Direction == EEdGraphPinDirection::EGPD_Input)
				{
					InputParametersStrings.Add(PinInfoString);
				}
				else if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
				{
					OutputParametersStrings.Add(PinInfoString);
				}
			}
		}
	}
	
	{
		const AActor* ActiveActor = Cast<AActor>(ActiveObject);
		if (ActiveActor)
		{
			if (ActiveActor->GetOwner())
			{
				OwnerNameString = ActiveActor->GetOwner()->GetName();
			}
			if (ActiveActor->GetInstigator())
			{
				InstigatorNameString = ActiveActor->GetInstigator()->GetName();
			}
			if (ActiveActor->GetInstigatorController())
			{
				InstigatorControllerNameString = ActiveActor->GetInstigatorController()->GetName();
			}
		}
	}

	{
		OutBlueprintExceptionDebugInfo.FrameCounter = FrameCounter;
		OutBlueprintExceptionDebugInfo.FrameCounterString = FrameCounterString;

		OutBlueprintExceptionDebugInfo.Index = Index;
		OutBlueprintExceptionDebugInfo.IndexString = IndexString;


		OutBlueprintExceptionDebugInfo.TimestampString = TimestampString;

		OutBlueprintExceptionDebugInfo.ActiveObjectNameString = ActiveObjectNameString;

		OutBlueprintExceptionDebugInfo.StackTraceString = StackTraceString;
		OutBlueprintExceptionDebugInfo.ScriptCallstackString = ScriptCallstackString;
		OutBlueprintExceptionDebugInfo.StackDescriptionString = StackDescriptionString;

		OutBlueprintExceptionDebugInfo.PreFrameNameString = PreFrameNameString;

		OutBlueprintExceptionDebugInfo.NodeGraphNameString = NodeGraphNameString;

		OutBlueprintExceptionDebugInfo.NodeNameString = NodeNameString;
		OutBlueprintExceptionDebugInfo.NodeTitleString = NodeTitleString;
		OutBlueprintExceptionDebugInfo.NodeUniqueIDString = NodeUniqueIDString;
		OutBlueprintExceptionDebugInfo.NodeCustomFullNameString = NodeCustomFullNameString;

		OutBlueprintExceptionDebugInfo.WatchedPinsStrings = WatchedPinsStrings;

		OutBlueprintExceptionDebugInfo.InputParametersStrings = InputParametersStrings;

		OutBlueprintExceptionDebugInfo.OutputParametersStrings = OutputParametersStrings;

		OutBlueprintExceptionDebugInfo.OwnerNameString = OwnerNameString;
		OutBlueprintExceptionDebugInfo.InstigatorNameString = InstigatorNameString;
		OutBlueprintExceptionDebugInfo.InstigatorControllerNameString = InstigatorControllerNameString;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUEDebuggerEditorModule, UEDebuggerEditor)