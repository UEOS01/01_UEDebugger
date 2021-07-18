// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UEDebuggerConsoleCommandGroup.h"
#include "UEDebugger.h"
#include "UnrealEngine.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogConsoleCommandGroupManager, Log, All);

IConsoleCommandGroupManager* IConsoleCommandGroupManager::Singleton;

void IConsoleCommandGroupManager::SetupSingleton()
{
	check(!Singleton);
	if (!Singleton)
	{
		Singleton = new FConsoleCommandGroupManager; // we will leak this
	}
	check(Singleton);
}

FConsoleCommandGroupManager::~FConsoleCommandGroupManager()
{
	for (TMap<FString, IConsoleCommandGroupObject*>::TConstIterator PairIt(ConsoleCommandGroupObjects); PairIt; ++PairIt)
	{
		IConsoleCommandGroupObject* Var = PairIt.Value();

		delete Var;
	}
}

IConsoleCommandGroupObject* FConsoleCommandGroupManager::RegisterConsoleCommandGroupObject(const FString& Name, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable)
{
	return AddConsoleCommandGroupObject(Name, new FConsoleCommandGroupObject(Name, InConsoleCommandsToEnable, InConsoleCommandsToDisable));
}

void FConsoleCommandGroupManager::UnregisterConsoleCommandGroupObject(IConsoleCommandGroupObject* ConsoleCommandGroupObject)
{
	if (!ConsoleCommandGroupObject)
	{
		return;
	}
	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject);

	const FString ObjName = FindConsoleCommandGroupObjectName(ConsoleCommandGroupObject);
	if (!ObjName.IsEmpty())
	{
		UnregisterConsoleCommandGroupObject(ObjName);
	}
}

void FConsoleCommandGroupManager::UnregisterConsoleCommandGroupObject(const FString& Name)
{
	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject);

	IConsoleCommandGroupObject* Object = FindConsoleCommandGroupObject(Name);

	if (Object)
	{
		ConsoleCommandGroupObjects.Remove(Name);
		Object->Release();
	}
}

IConsoleCommandGroupObject* FConsoleCommandGroupManager::FindConsoleCommandGroupObject(const FString& Name) const
{
	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject);
	IConsoleCommandGroupObject* ConsoleCommandGroupObject = ConsoleCommandGroupObjects.FindRef(Name);
	return ConsoleCommandGroupObject;
}

bool FConsoleCommandGroupManager::IsNameRegistered(const FString& Name) const
{
	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject);
	return ConsoleCommandGroupObjects.Contains(Name);
}

TArray<FString> FConsoleCommandGroupManager::GetAllConsoleCommandGroupObjectNames() const
{
	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject);
	TArray<FString> NameArray;
	ConsoleCommandGroupObjects.GenerateKeyArray(NameArray);
	return NameArray;
}

IConsoleCommandGroupObject* FConsoleCommandGroupManager::AddConsoleCommandGroupObject(const FString& Name, IConsoleCommandGroupObject* Obj)
{
	check(!Name.IsEmpty());
	check(Obj);

	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject); // we will lock on the entire add process
	IConsoleCommandGroupObject* ExistingObj = ConsoleCommandGroupObjects.FindRef(Name);

	if (ExistingObj)
	{
		UE_LOG(LogConsoleCommandGroupManager, Warning, TEXT("ConsoleCommandGroup object named '%s' already exists but is being registered again, but we weren't expected it to be! (FConsoleCommandGroupManager::AddConsoleCommandGroupObject)"), *Name);
		return ExistingObj;
	}
	else
	{
		ConsoleCommandGroupObjects.Add(Name, Obj);
		return Obj;
	}
}

FString FConsoleCommandGroupManager::FindConsoleCommandGroupObjectName(const IConsoleCommandGroupObject* Obj) const
{
	check(Obj);

	FScopeLock ScopeLock(&ConsoleCommandGroupObjectsSynchronizationObject);
	for (TMap<FString, IConsoleCommandGroupObject*>::TConstIterator PairIt(ConsoleCommandGroupObjects); PairIt; ++PairIt)
	{
		IConsoleCommandGroupObject* Var = PairIt.Value();

		if (Var == Obj)
		{
			const FString& Name = PairIt.Key();

			return Name;
		}
	}

	return FString();
}

void FConsoleCommandGroupObject::Release()
{
	delete this;
}

const FString FConsoleCommandGroupObject::GetHelp() const
{
	return TEXT("");
}

void FConsoleCommandGroupObject::SetHelp(const FString Value)
{

}

void FConsoleCommandGroupObject::Enable(UObject* WorldContextObject, APlayerController* Player)
{
	// First, try routing through the primary player
	APlayerController* TargetPC = Player ? Player : UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (TargetPC)
	{
		for (auto& ConsoleCommand : ConsoleCommandsToEnable)
		{
			TargetPC->ConsoleCommand(ConsoleCommand, true);
		}
	}
}

void FConsoleCommandGroupObject::Disable(UObject* WorldContextObject, APlayerController* Player)
{
	// First, try routing through the primary player
	APlayerController* TargetPC = Player ? Player : UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (TargetPC)
	{
		for (auto& ConsoleCommand : ConsoleCommandsToDisable)
		{
			TargetPC->ConsoleCommand(ConsoleCommand, true);
		}
	}
}