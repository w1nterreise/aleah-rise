// AleahRise v1.04 -- adaptive soundtrack system for UE4
// © Daniel Winterreise, 2019

#include "AdaptiveMixer.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h" 
#include "AudioDevice.h"
#include "ActiveSound.h"


DEFINE_LOG_CATEGORY(AdaptiveMixerLog);
#define print_debug_message(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red,text)
    

AAdaptiveMixer::AAdaptiveMixer() {
    
    UE_LOG(AdaptiveMixerLog, Display, TEXT("Adaptive mixer created."));
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        __pattern_audio_components[i] = CreateDefaultSubobject<UAudioComponent>
            (*FString("pattern_audiocomp" + FString::FromInt(i)));
        __all_audio_components.Add(__pattern_audio_components[i]);
    }
    
    __bridge_audio_component = CreateDefaultSubobject<UAudioComponent>
        (*FString("bridge_audio_comp"));
    __all_audio_components.Add(__bridge_audio_component);
    
    __stinger_audio_component = CreateDefaultSubobject<UAudioComponent>
            (*FString("stinger_audio_comp"));
    __all_audio_components.Add(__stinger_audio_component);

    
    int count = 0;
    for (UAudioComponent* audioComp : __all_audio_components) {
        if (audioComp) {
            if (audioComp->IsValidLowLevelFast()) {
                audioComp->bAutoActivate = false;
                audioComp->bAutoDestroy = false;
                audioComp->bIsMusic = true;
                audioComp->bStopWhenOwnerDestroyed = true;
                ++count;
            }
        }
    }
    
    if (__pattern_audio_components[0]->IsValidLowLevelFast()) {
    SetRootComponent(__pattern_audio_components[0]); }
        
    __default_adaptive_score = CreateDefaultSubobject<UAdaptiveScore>(TEXT("default_adaptive_score"));
    __default_dynamic_filter_chain = CreateDefaultSubobject<UDynamicFilterChain>(TEXT("default_dfc"));  
    
    if ((count == __all_audio_components.Num()) &&
    (__default_adaptive_score != nullptr) &&
    (__default_dynamic_filter_chain != nullptr)) {
        UE_LOG(AdaptiveMixerLog, Display, TEXT("Adaptive mixer: subobject creation successful."));
    }
    else {
        UE_LOG(AdaptiveMixerLog, Error, TEXT("Adaptive mixer: subobject creation failed."));
    }
            
    __is_running = false;
    __is_initialized = false;
    __texture = 0;
}


UAdaptiveScore* AAdaptiveMixer::GetDefaultAdaptiveScore() {
    if (__default_adaptive_score == nullptr)
        UE_LOG(AdaptiveMixerLog, Error, TEXT("Something went wrong: __default_adaptive_score == nullptr."));
    return __default_adaptive_score;
}

UDynamicFilterChain* AAdaptiveMixer::GetDynamicFilterChain() {
    if (__default_dynamic_filter_chain == nullptr)
        UE_LOG(AdaptiveMixerLog, Error, TEXT("Something went wrong: __default_dynamic_filter_chain == nullptr."));
    return __default_dynamic_filter_chain;
}

bool AAdaptiveMixer::InitializeMixer(UAdaptiveScore* adaptive_composition, float master_volume) {
    
    if (__is_running) {
        UE_LOG(AdaptiveMixerLog, Warning, TEXT("Adatpive mixer is already running."));
        UE_LOG(AdaptiveMixerLog, Display, TEXT("You must stop it before re-initialization."));
        return false;
    }
    
    if (adaptive_composition == nullptr) {
        UE_LOG(AdaptiveMixerLog, Warning, TEXT("Adaptive mixer initialization failed."));
        print_debug_message(TEXT("adaptive_composition is nullptr."));
        return false;
    }
    
    __is_initialized = false;
    
    __loaded_score = adaptive_composition;
    TArray<USoundCue*> patterns = __loaded_score->GetPatternCues();
    
    if (patterns.Num() < 1) {
        UE_LOG(AdaptiveMixerLog, Warning, TEXT("Patterns array is empty. Initialization canceled."));
        return false;
    }
    
    //try to initialize patterns:
    int initialized_patterns;
    initialized_patterns = 0;
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        USoundCue* loadCue = i < patterns.Num() ? patterns[i] : nullptr;
        bool validCue = __isSoundBaseValid(loadCue);
        if (validCue) {
            __pattern_audio_components[i]->SetSound(loadCue);
            ++initialized_patterns;
            UE_LOG(AdaptiveMixerLog, Display, TEXT("Pattern %d cue initialized."), i);
        }
        __patterns_validation[i] = validCue ? TRUE : FALSE;
    }
    
    if (initialized_patterns < 1) {
        UE_LOG(AdaptiveMixerLog, Warning, TEXT("Adaptive mixer initialization failed."));
        UE_LOG(AdaptiveMixerLog, Display, TEXT("Hm..? May be something is wrong with patterns array?"));
        return false;
    }
    
    //it's ok:
    __is_initialized = true;
    __bridge_sound_cues = __loaded_score->GetBridgeCues();
    __stinger_sound_cues = __loaded_score->GetStingerCues();
    __initializeDefaultVolume();
    __master_volume = master_volume;
    __score_fade_time = __loaded_score->GetFadeTime();
    __score_filterchain_index = __loaded_score->GetFilterchainIndex();
    __default_dynamic_filter_chain->Clear();
    UE_LOG(AdaptiveMixerLog, Display, TEXT("Adaptive mixer initialized."));
    return true;
}

void AAdaptiveMixer::Run(uint8 initial_texture) {
    
    if (!__is_initialized) {
        UE_LOG(AdaptiveMixerLog, Warning, TEXT("Adaptive mixer is not initialized."));
        return;
    }
        
    if (__is_running) {
        Stop();
    }
    
    __is_running = true;
    __beginToPlaySilently();
    __texture = initial_texture;
    __decodeFromByte(__getFilteredTexture(), __score_fade_time);
    UE_LOG(AdaptiveMixerLog, Display, TEXT("Adaptive mixer is running."));
}

void AAdaptiveMixer::Stop() {
    
    if (!__is_running)
        return;
    
    __is_running = false;
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        if (__patterns_validation[i] == TRUE) {
            __pattern_audio_components[i]->Stop();
        }
    }
    
    UE_LOG(AdaptiveMixerLog, Display, TEXT("Adaptive mixer stoped."));  
}

void AAdaptiveMixer::IncreaseTexture() {
    
    if (!__is_running)
        return;
    
    uint8 increased_texture;
    increased_texture = __texture * 2 + 1; // validate
    PlayNewTexture(increased_texture);
}

void AAdaptiveMixer::DecreaseTexture() {
    
    if (!__is_running)
        return;
    
    uint8 decreased_texture;
    decreased_texture = (__texture - 1) / 2; // validate
    PlayNewTexture(decreased_texture);
}

void AAdaptiveMixer::PlayNewTexture(uint8 new_texture) {
    
    if (!__is_running)
        return;
    
    if (__texture != new_texture) {
        __texture = new_texture;
        uint8 processed_txt = __getFilteredTexture();
        __decodeFromByte(processed_txt, __score_fade_time);
        UE_LOG(AdaptiveMixerLog, Display, TEXT("__texture changed to %d"), __texture);
        UE_LOG(AdaptiveMixerLog, Display, TEXT("(processed is %d)"), processed_txt);
    }
}

void AAdaptiveMixer::PlayNewTextureAfterBridge(uint8 new_texture, int bridge_index,
    float fade_out_ratio, float fade_in_ratio, float bridge_volume) {
        
    if (!__is_running)
        return;
    
    if (bridge_index < 0)
        return;
    
    if (bridge_index >= __bridge_sound_cues.Num())
        return;
    
    USoundCue* cue = __bridge_sound_cues[bridge_index];
    
    if (!__isSoundBaseValid(cue))
        return;
    
    //it's ok:
    __bridge_audio_component->SetSound(cue);
    float bridge_duration;
    bridge_duration = __bridge_audio_component->Sound->GetDuration();
    FTimerDelegate crossfade_timer_Del;
    crossfade_timer_Del.BindUFunction(this, FName("__onBridgeCrossfadeTimer"),
    fade_in_ratio * bridge_duration);
    GetWorld()->GetTimerManager().SetTimer(__bridge_timer_handle, crossfade_timer_Del,
    bridge_duration - bridge_duration * fade_in_ratio, false);
    __bridge_audio_component->FadeIn(fade_out_ratio*bridge_duration, __verifiedVolume(bridge_volume * __master_volume), 0.0f);
    __texture = new_texture;
    UE_LOG(AdaptiveMixerLog, Display, TEXT("__texture changed to %d."), __texture);
    for (int i = 0; i < PTRN_COUNT; ++i) {
        __muteTrack(i, bridge_duration * fade_out_ratio);
    }
}


void AAdaptiveMixer::PlayStinger(int index, float stinger_volume) {
    
    if (!__is_running)
        return;
    
    if (index < 0)
        return;

    if (index >= __stinger_sound_cues.Num())
        return;
    
    USoundCue* cue = __stinger_sound_cues[index];
    if (!__isSoundBaseValid(cue))
        return;
    
    //it's ok:
    __stinger_audio_component->SetSound(cue);
    __stinger_audio_component->FadeIn(0.0f, __verifiedVolume(stinger_volume * __master_volume), 0.0f);
}

void AAdaptiveMixer::InsertPattern(uint8 index) {
    
    if (index >= PTRN_COUNT)
        return;
    
    uint8 mask = 1 << index;
    PlayNewTexture(__texture | mask);
}

void AAdaptiveMixer::EjectPattern(uint8 index) {
    
    if (index >= PTRN_COUNT)
        return;
    
    uint8 mask = 1 << index;
    mask = uint8(~mask);
    PlayNewTexture(__texture & mask);
}

void AAdaptiveMixer::BitwiseANDing(uint8 mask) {
    PlayNewTexture(__texture & mask);
}

void AAdaptiveMixer::BitwiseORing(uint8 mask) {
    PlayNewTexture(__texture | mask);
}


void AAdaptiveMixer::InitializeAllPatternsVolume(float ptrn0_vol, float ptrn1_vol,
    float ptrn2_vol, float ptrn3_vol, float ptrn4_vol, float ptrn5_vol,
    float ptrn6_vol, float ptrn7_vol, bool adjust_playback) {

    __patterns_volume[0] = __verifiedVolume(ptrn0_vol);
    __patterns_volume[1] = __verifiedVolume(ptrn1_vol);
    __patterns_volume[2] = __verifiedVolume(ptrn2_vol);
    __patterns_volume[3] = __verifiedVolume(ptrn3_vol);
    __patterns_volume[4] = __verifiedVolume(ptrn4_vol);
    __patterns_volume[5] = __verifiedVolume(ptrn5_vol);
    __patterns_volume[6] = __verifiedVolume(ptrn6_vol);
    __patterns_volume[7] = __verifiedVolume(ptrn7_vol);
    
    if (adjust_playback)
        __decodeFromByte(__getFilteredTexture(), __score_fade_time);
}

void AAdaptiveMixer::SetPatternVolume(uint8 index, float volume) {
    
    if (index >= PTRN_COUNT)
        return;
    
    __patterns_volume[index] = __verifiedVolume(volume);
    __decodeFromByte(__getFilteredTexture(), __score_fade_time);
}


void AAdaptiveMixer::SetAllPatternsVolume(float volume) {
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        __patterns_volume[i] = __verifiedVolume(volume);
    }
    __decodeFromByte(__getFilteredTexture(), __score_fade_time);
}

void AAdaptiveMixer::SetMasterVolume(float volume) {
    __master_volume = __verifiedVolume(volume);
    __decodeFromByte(__getFilteredTexture(), __score_fade_time);
}

uint8 AAdaptiveMixer::BinaryToDecimal(int binary_number) {
    
    int num = binary_number;
    uint8 dec_value = 0;
    const int MAX_BIN_VALUE = 11111111;
    if ((binary_number >= 0) && (binary_number <= MAX_BIN_VALUE)) {
        int temp = num;
        int base = 1;
        while (temp) {
            int last_digit = temp % 10;
            temp = temp / 10;
            dec_value += last_digit * base;
            base = base * 2;
        }
    }
    return dec_value;
}

uint8 AAdaptiveMixer::BoolArrayToDecimal(bool track0, bool track1, bool track2, bool track3,
                                        bool track4, bool track5, bool track6, bool track7) {
                                            
    uint8 n = 0;

/*                                          v                
                                          u@@,              
                            .           G@@@@B@:            
                */if (track0) n += 1;/* S@B@B@B:      .@J   
                             .@B          kB@:        iB@B: 
                */if (track1) n += 2;/*    .          B@B@@7
                                JB.                  2@B@B@B
   .;2E@@@B@B@BO*/if (track2) n += 4;/*             Z@B@B@B@
 5@@@@@@@B@B@B@@@B@S               U@Bv          .kB@B@B@B@Y
  Y@B@B@B@@@B@B@*/if (track3) n += 8;/*@B@BO0EO@B@B@B@B@B@B 
    @B@@@0ui:     .:YB@U               :@B@B@B@B@B@B@@@B@P  
      .         */if (track4) n += 16;/*  :0B@B@B@B@@@@P    
                     Z@B@B                    .:7vY7i       
                */if (track5) n += 32;/*                      
                  v@@B@B@B@                                 
               :*/if (track6) n += 64;/*                      
         :,,7E@@@B@B@B@B@7           .v58M@BBZF7.           
         XB@B@B@*/if (track7) n += 128;/*@@B@@@B@B@O:        
           @@B@@@B@B@BGB@X     iB@@B@B@B@B@B@B@B@@@B@7      
            ,EB@B07      LBS.5B@B@@@B@B@B@B@Fvi:::rjO@B.    
                          .B@B@B@B@@@B@Bv             :@0   
                           @B@B@B@B@Bu                 @B@: 
  7                        B@B@@@@@:                .r@B@B@.
  .Sq:          */return n;/*B@B@M:             JB@B@@@B@@@B@
    iB@P,                .@B@@@B              BB@B@B@B@B@@@B
      ,@B@BSi.         .5@B@B@B@.            .@@B@B@B@B@BB5 
         L@@@@@B@B@M@B@B@B@B@B@B              @B@B@B@r      
            7@B@B@B@@@B@B@B@@@B:               uB@BB        
                .r50MM@@@B@MNr                   r@         */
                                
}

uint8 AAdaptiveMixer::FillNbits(uint8 bits, bool play) {
    
    if (bits > 8)
        return __texture;
    
    uint8 result = pow(2, bits) - 1;
    if (play)
        PlayNewTexture(result);
    return result;
}

uint8 AAdaptiveMixer::FillNbitsPlusX(uint8 bits, uint8 x, bool condition, bool play) {
    
    if (bits > 8)
        return __texture;
    
    uint8 result = pow(2, bits) - 1;
    if (condition)
        result += x;
    if (play)
        PlayNewTexture(result);
    return result;
}

uint8 AAdaptiveMixer::FillNbitsAddB(uint8 bits, uint8 b, bool condition, bool play) {
    if ((bits > 8) || (b >= 8))
        return __texture;
    
    uint8 result = pow(2, bits) - 1;
    if (condition)
        result |= (1 << b);
    if (play)
        PlayNewTexture(result);
    return result;
}

void AAdaptiveMixer::LogPlaybackTime(uint8 pattern_index) {
    
    if (pattern_index >= PTRN_COUNT)
        return;
    
    UAudioComponent* AudioComponent = __pattern_audio_components[pattern_index];
    if (AudioComponent) {
        if (FAudioDevice* device = AudioComponent->GetAudioDevice()) {
            const uint64 component_id = AudioComponent->GetAudioComponentID();
            FAudioThread::RunCommandOnAudioThread([device, component_id, pattern_index]() {
                if (FActiveSound* active = device->FindActiveSound(component_id)) {
                    UE_LOG(AdaptiveMixerLog, Warning, TEXT("Pattern %d playback time = %f."),
                        pattern_index, active->PlaybackTime);
                }
                else {
                    UE_LOG(AdaptiveMixerLog, Warning, TEXT("Can't find active sound."));
                }
            });
        }
        else {
            UE_LOG(AdaptiveMixerLog, Warning, TEXT("Can't find audio device."));
        }
    }
    else {
        UE_LOG(AdaptiveMixerLog, Warning, TEXT("Invalid pointer."));
    }
}

void AAdaptiveMixer::LogAllPlaybackTime() {
    
    if (!__is_running)
        return;
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        LogPlaybackTime(i);
    }
}


bool AAdaptiveMixer::IsRunning() {
    return __is_running;
}

bool AAdaptiveMixer::IsInitialized() {
    return __is_initialized;
}

uint8 AAdaptiveMixer::GetTexture() {
    return __texture;
}

bool AAdaptiveMixer::__isSoundBaseValid(USoundBase* base_ptr) {
    
    bool result = false;
    
    if (IsValid(base_ptr)) {
        if (base_ptr->IsValidLowLevelFast()) {
            result = true;
        }
    }
    return result;
}

uint8 AAdaptiveMixer::__getFilteredTexture() {
    
    uint8 result;
    if (__default_dynamic_filter_chain->IsAvailable()) {
        result = __default_dynamic_filter_chain->ApplyDynamicFilterChain(__texture);
        UE_LOG(AdaptiveMixerLog, Display, TEXT("Processing texture with Dynamic FCH..."));
    }
    else {
        result = UStaticFilterChain::ApplyFilterChain(__texture, __score_filterchain_index);
        UE_LOG(AdaptiveMixerLog, Display, TEXT("Processing texture with Static FCH..."));
    }
    return result;
}

void AAdaptiveMixer::__initializeDefaultVolume() {
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        __patterns_volume[i] = 1.0f;
    }
}

void AAdaptiveMixer::__onBridgeCrossfadeTimer(float fade) {
    
    if (!__is_running)
        return; 
    
    __beginToPlaySilently();
    __decodeFromByte(__getFilteredTexture(), fade);
    __bridge_audio_component->FadeOut(fade, 0.0f);
}

void AAdaptiveMixer::__muteTrack(uint8 index, float fade) {
    
    if (__patterns_validation[index] == TRUE) {
        __pattern_audio_components[index]->FadeOut(fade, 0.0f);
    }
}

float AAdaptiveMixer::__verifiedVolume(float volume) {
    
    if (volume < 0.0f)
        return 0.0f;
    else if (volume > 1.0f)
        return 1.0f;
    else
        return volume;
}

void AAdaptiveMixer::__adjustPatternVolume(uint8 index, float volume, float fade) {
    
    if (__patterns_validation[index] == TRUE) {
            
        __pattern_audio_components[index]->AdjustVolume(fade, volume * __master_volume);
    }
}

void AAdaptiveMixer::__decodeFromByte(uint8 processed_texture, float fade) {
    
    if (!__is_running)
        return;
    
    bool b[PTRN_COUNT];
    UStaticFilterChain::BoolArrayFromByte(processed_texture, b);
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        float volume = (b[i]) ? (__patterns_volume[i]) : 0.0f;
        __adjustPatternVolume(i, volume, fade);
    }
}

void AAdaptiveMixer::__beginToPlaySilently() {
    
    for (int i = 0; i < PTRN_COUNT; ++i) {
        if (__patterns_validation[i] == TRUE) {
            __pattern_audio_components[i]->Play();
        }
    }
    for (int i = 0; i < PTRN_COUNT; ++i) {
        __adjustPatternVolume(i, 0.0f, 0.0f);
    }
}

AAdaptiveMixer::~AAdaptiveMixer() {
        
    UE_LOG(AdaptiveMixerLog, Display, TEXT("Adaptive mixer destroyed."));
}
