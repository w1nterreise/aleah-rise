// AleahRise v1.04 -- adaptive soundtrack system for UE4
// © Daniel Winterreise, 2019

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "AdaptiveScore.generated.h"

UCLASS(Blueprintable)
class UAdaptiveScore : public UObject
{
    GENERATED_BODY()
    
//==============================================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
    
    UFUNCTION(BlueprintCallable)   /*Step 2*/ void InitializeScoreFull(TArray<USoundCue*> pattern_cues,
                                              TArray<USoundCue*> bridge_cues, TArray<USoundCue*> stinger_cues,
                                              float fade_time, uint8 filterchain_index);
                                    
    UFUNCTION(BlueprintCallable)   /*Step 2*/ void InitializeScorePatternsOnly(TArray<USoundCue*> pattern_cues,
                                              float fade_time, uint8 filterchain_index);
                                    
    UFUNCTION(BlueprintCallable)   /*Step 2*/ void InitializeScorePatternsAndBridges(TArray<USoundCue*>  pattern_cues,
                                              TArray<USoundCue*> bridge_cues, float fade_time, uint8 filterchain_index);
                                    
    UFUNCTION(BlueprintCallable)   /*Step 2*/ void InitializeScorePatternsAndStingers(TArray<USoundCue*>  pattern_cues,
                                              TArray<USoundCue*> stinger_cues, float fade_time, uint8 filterchain_index);
                                    
    UFUNCTION(BlueprintCallable)        void Clear();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
//==============================================================================================================
                                    
    UFUNCTION() TArray<USoundCue*>  GetPatternCues();
    UFUNCTION() TArray<USoundCue*>  GetBridgeCues();
    UFUNCTION() TArray<USoundCue*>  GetStingerCues();
    
    UFUNCTION() float GetFadeTime();    
    UFUNCTION() uint8 GetFilterchainIndex();
        
    private:
        UPROPERTY()
        TArray<USoundCue*> __pattern_cues;
        TArray<USoundCue*> __bridge_cues;
        TArray<USoundCue*> __stinger_cues;
        
        UPROPERTY()
        float __fade_time;
        UPROPERTY()
        float __filterchain_index;
    
    public:
        UAdaptiveScore();   
        ~UAdaptiveScore();
        
};
