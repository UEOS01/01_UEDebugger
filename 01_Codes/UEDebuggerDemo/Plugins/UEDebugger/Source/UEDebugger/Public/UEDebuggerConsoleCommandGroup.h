// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Misc/AssertionMacros.h"
#include "Containers/UnrealString.h"
#include "Logging/LogMacros.h"

class IConsoleCommandGroupObject;
class APlayerController;

/**
 * handles console command group, registered console command group are released on destruction
 */
struct UEDEBUGGER_API IConsoleCommandGroupManager
{
	virtual IConsoleCommandGroupObject* RegisterConsoleCommandGroupObject(const FString& Name, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable) = 0;

	virtual void UnregisterConsoleCommandGroupObject(IConsoleCommandGroupObject* ConsoleCommandGroupObject) = 0;

	virtual void UnregisterConsoleCommandGroupObject(const FString& Name) = 0;

	virtual IConsoleCommandGroupObject* FindConsoleCommandGroupObject(const FString& Name) const = 0;

	/**
	 * Check if a name (command or variable) has been registered with the console manager
	 * @param Name - Name to check. Must not be 0
	 */
	virtual bool IsNameRegistered(const FString& Name) const = 0;

	virtual TArray<FString> GetAllConsoleCommandGroupObjectNames() const = 0;

	/** Returns the singleton for the ConsoleCommandGroupManager **/
	FORCEINLINE static IConsoleCommandGroupManager& Get()
	{
		if (!Singleton)
		{
			SetupSingleton();
			check(Singleton != nullptr);
		}
		return *Singleton;
	}

protected:

	virtual ~IConsoleCommandGroupManager() { }

private:

	/** Singleton for the ConsoleCommandGroupManager **/
	static IConsoleCommandGroupManager* Singleton;

	/** Function to create the singleton **/
	static void SetupSingleton();
};

class UEDEBUGGER_API FConsoleCommandGroupManager :public IConsoleCommandGroupManager
{
public:
	/** constructor */
	FConsoleCommandGroupManager()
	{
	}

	/** destructor */
	~FConsoleCommandGroupManager();

	// interface IConsoleManager -----------------------------------

	virtual IConsoleCommandGroupObject* RegisterConsoleCommandGroupObject(const FString& Name, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable) override;

	virtual void UnregisterConsoleCommandGroupObject(IConsoleCommandGroupObject* ConsoleCommandGroupObject) override;

	virtual void UnregisterConsoleCommandGroupObject(const FString& Name) override;

	virtual IConsoleCommandGroupObject* FindConsoleCommandGroupObject(const FString& Name) const override;

	/**
	 * Check if a name (command or variable) has been registered with the console manager
	 * @param Name - Name to check. Must not be 0
	 */
	virtual bool IsNameRegistered(const FString& Name) const override;

	virtual TArray<FString> GetAllConsoleCommandGroupObjectNames() const override;

	// ----------------------------------------------------

	/**
	 * @param Name must not be 0, must not be empty
	 * @param Obj must not be 0
	 * @return 0 if the name was already in use
	 */
	IConsoleCommandGroupObject* AddConsoleCommandGroupObject(const FString& Name, IConsoleCommandGroupObject* Obj);

	FString FindConsoleCommandGroupObjectName(const IConsoleCommandGroupObject* Obj) const;

private: 

	/** Map of ConsoleCommandGroupObjects, indexed by the name of that ConsoleCommandGroupObject */
	// [name] = pointer (pointer must not be 0)
	TMap<FString, IConsoleCommandGroupObject*> ConsoleCommandGroupObjects;

	/**
	 * Used to prevent concurrent access to ConsoleCommandGroupObjects.
     **/
	mutable FCriticalSection ConsoleCommandGroupObjectsSynchronizationObject;
};

/**
 * Interface for console command group objects
 */
class UEDEBUGGER_API IConsoleCommandGroupObject
{

public:

	IConsoleCommandGroupObject(const FString& InName, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable)
		: Name(InName), ConsoleCommandsToEnable(InConsoleCommandsToEnable), ConsoleCommandsToDisable(InConsoleCommandsToDisable)
	{}

	virtual ~IConsoleCommandGroupObject() {}

	/**
	 *  should only be called by the manager, needs to be implemented for each instance
	 */
	virtual void Release() = 0;

	/**
	 *  @return never 0, can be multi line ('\n')
	 */
	virtual const FString GetHelp() const = 0;
	/**
	 *  @return never 0, can be multi line ('\n')
	 */
	virtual void SetHelp(const FString Value) = 0;

	/**
	 *  should only be called by the manager, needs to be implemented for each instance
	 */
	virtual void Enable(UObject* WorldContextObject, APlayerController* Player) = 0;

	/**
     *  should only be called by the manager, needs to be implemented for each instance
     */
	virtual void Disable(UObject* WorldContextObject, APlayerController* Player) = 0;

	FString Name;

	TArray<FString> ConsoleCommandsToEnable;
	TArray<FString> ConsoleCommandsToDisable;

	friend class FConsoleCommandGroupManager;
};

class UEDEBUGGER_API FConsoleCommandGroupObject : public IConsoleCommandGroupObject
{
public:
	FConsoleCommandGroupObject(const FString& InName, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable)
		: IConsoleCommandGroupObject(InName, InConsoleCommandsToEnable, InConsoleCommandsToDisable)
	{
	}

	/**
	 *  should only be called by the manager, needs to be implemented for each instance
	 */
	virtual void Release() override;

	/**
	 *  @return never 0, can be multi line ('\n')
	 */
	virtual const FString GetHelp() const override;
	/**
	 *  @return never 0, can be multi line ('\n')
	 */
	virtual void SetHelp(const FString Value) override;

	/**
	 *  should only be called by the manager, needs to be implemented for each instance
	 */
	virtual void Enable(UObject* WorldContextObject, APlayerController* Player) override;

	/**
	 *  should only be called by the manager, needs to be implemented for each instance
	 */
	virtual void Disable(UObject* WorldContextObject, APlayerController* Player) override;
};

/**
 * Base class for autoregistering Console Command Group.
 */
class UEDEBUGGER_API FAutoConsoleCommandGroupObject
{
protected:
	FAutoConsoleCommandGroupObject(IConsoleCommandGroupObject* InTarget)
		: Target(InTarget)
	{
		check(Target);
	}
	/** Destructor, removes the ConsoleCommandGroup object **/
	virtual ~FAutoConsoleCommandGroupObject()
	{
		IConsoleCommandGroupManager::Get().UnregisterConsoleCommandGroupObject(Target);
	}

private:
	/** Contained console object, cannot be 0 **/
	IConsoleCommandGroupObject* Target;
};

/**
 * AutoConsoleCommandGroup
 */
class UEDEBUGGER_API TAutoConsoleCommandGroup : public FAutoConsoleCommandGroupObject
{
public:
	/**
	 * Create a float, int or string console variable
	 * @param Name must not be 0
	 * @param Help must not be 0
	 * @param Flags bitmask combined from EConsoleVariableFlags
	 */
	TAutoConsoleCommandGroup(const FString& Name, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable);
};

inline TAutoConsoleCommandGroup::TAutoConsoleCommandGroup(const FString& Name, const TArray<FString>& InConsoleCommandsToEnable, const TArray<FString>& InConsoleCommandsToDisable)
	: FAutoConsoleCommandGroupObject(IConsoleCommandGroupManager::Get().RegisterConsoleCommandGroupObject(Name, InConsoleCommandsToEnable, InConsoleCommandsToDisable))
{

}
