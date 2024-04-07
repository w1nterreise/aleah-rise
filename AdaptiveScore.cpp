// AleahRise v1.04 -- adaptive soundtrack system for UE4
// © Daniel Winterreise, 2019

#include "AdaptiveScore.h"


void UAdaptiveScore::InitializeScoreFull(TArray<USoundCue*> pattern_cues, TArray<USoundCue*> bridge_cues,
    TArray<USoundCue*> stinger_cues, float fade_time, uint8 filterchain_index) {
        
    Clear();    
    
    __pattern_cues = pattern_cues;
    __bridge_cues = bridge_cues;
    __stinger_cues = stinger_cues;

    __fade_time = fade_time;
    __filterchain_index = filterchain_index;
}

void UAdaptiveScore::InitializeScorePatternsOnly(TArray<USoundCue*> pattern_cues,
    float fade_time, uint8 filterchain_index) {
        
    Clear();
        
    __pattern_cues = pattern_cues;
    
    __fade_time = fade_time;
    __filterchain_index = filterchain_index;
}

void UAdaptiveScore::InitializeScorePatternsAndBridges(TArray<USoundCue*> pattern_cues, TArray<USoundCue*> bridge_cues,
    float fade_time, uint8 filterchain_index) {
        
    Clear();
    
    __pattern_cues = pattern_cues;
    __bridge_cues = bridge_cues;
    
    __fade_time = fade_time;
    __filterchain_index = filterchain_index;
}

void UAdaptiveScore::InitializeScorePatternsAndStingers(TArray<USoundCue*> pattern_cues, TArray<USoundCue*> stinger_cues,
    float fade_time, uint8 filterchain_index) {
        
    Clear();
        
    __pattern_cues = pattern_cues;
    __stinger_cues = stinger_cues;
    
    __fade_time = fade_time;
    __filterchain_index = filterchain_index;
}

TArray<USoundCue*> UAdaptiveScore::GetPatternCues() {
    return __pattern_cues;
}

TArray<USoundCue*> UAdaptiveScore::GetBridgeCues() {
    return __bridge_cues;
}

TArray<USoundCue*> UAdaptiveScore::GetStingerCues() {
    return __stinger_cues;
}


float UAdaptiveScore::GetFadeTime() {

    return __fade_time;
}

uint8 UAdaptiveScore::GetFilterchainIndex() {

    return __filterchain_index;
}

void UAdaptiveScore::Clear() {
    
    __pattern_cues.Empty();
    __bridge_cues.Empty();
    __stinger_cues.Empty();

}

UAdaptiveScore::UAdaptiveScore() {
    UE_LOG(LogTemp, Display, TEXT("Adaptive score created."));
}

UAdaptiveScore::~UAdaptiveScore() {
    UE_LOG(LogTemp, Display, TEXT("Adaptive score destroyed."));
}