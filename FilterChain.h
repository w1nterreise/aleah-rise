// AleahRise v1.04 -- adaptive soundtrack system for UE4
// © Daniel Winterreise, 2019

#pragma once

#include "CoreMinimal.h"
#include "FilterChain.generated.h"

UCLASS()
class UStaticFilterChain : public UObject
{
    GENERATED_BODY()
   
public:

    static void BoolArrayFromByte(uint8, bool[]);
    
    UFUNCTION()
    static uint8 ApplyFilterChain(uint8 source_texture, uint8 filterchain_index);

};


USTRUCT()
struct FFilter
{
    GENERATED_BODY()

    public: 
    
    void SetTrack(uint8 track) { __track = track; }
    void SetOperation(FString operation) { __operation = operation.ToLower(); }
    void SetMask(uint8 mask) { __mask = mask; }
    void SetTerminate(bool terminate) { __terminate = terminate; }
    
    uint8 GetTrack() { return __track; }
    FString GetOperation() { return __operation; }
    uint8 GetMask() { return __mask; }
    bool IsTerminate() { return __terminate; }
    
    uint8 Apply(uint8 source_texture, bool & changed);
    
    protected:

    UPROPERTY()     uint8 __track;
    UPROPERTY()     FString __operation;
    UPROPERTY()     uint8 __mask;
    UPROPERTY()     bool __terminate;

    public:
    
    FFilter()
    {
        __track = 0;
        __operation = TEXT("");
        __mask = 0; 
        __terminate = false;
    }
};


UCLASS(Blueprintable)
class UDynamicFilterChain : public UObject
{
    GENERATED_BODY()
    
    public:
    UFUNCTION(BlueprintCallable)    void AddFilter(uint8 track, FString operation, uint8 mask, bool terminate_subchain);
                                    // "Operation" must be "or", "and" or "xor".
                                    
    UFUNCTION(BlueprintCallable)    void Clear();
    
    UFUNCTION()
    uint8 ApplyDynamicFilterChain(uint8 source);
    
    UFUNCTION()
    bool IsAvailable();
        
    private:
    UPROPERTY()
    TArray<FFilter> __chain;
    
    public:
    UDynamicFilterChain();
    ~UDynamicFilterChain();
    
};