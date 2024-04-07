// AleahRise v1.04 -- adaptive soundtrack system for UE4
// © Daniel Winterreise, 2019

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AdaptiveScore.h"
#include "FilterChain.h"
#include "AdaptiveMixer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AdaptiveMixerLog, Log, All);

constexpr uint8 FALSE = 0;
constexpr uint8 TRUE = 1;

constexpr uint8 PTRN_COUNT = 8;

UCLASS(Blueprintable)
class AAdaptiveMixer : public AActor
{
    GENERATED_BODY()

    public:
    
//======================================================================================================
                                //These functions are for you friend//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
    
    // I N I T I A L I Z A T I O N  &  B A S I C  C O N T R O L :
    
    UFUNCTION(BlueprintCallable)   /*Step 1*/   UAdaptiveScore* GetDefaultAdaptiveScore();
                                   /*Step 2*\   UAdaptiveScore::InitializeScore(..)*/
                                   
    UFUNCTION(BlueprintCallable)   /*Step 3*/   bool InitializeMixer(UAdaptiveScore* adaptive_composition,
                                                float master_volume = 1.0f);
    
    UFUNCTION(BlueprintCallable)   /*Step 3.5*/ UDynamicFilterChain* GetDynamicFilterChain();
                                             // Allows to create filter chain within Blueprint
                                             // (Use the UDynamicFilterChain::AddFilter function)
                                             // another way -- type your own filter chain function
                                             // in FilterChain.cpp and recompile.
                                        
    UFUNCTION(BlueprintCallable)   /*Step 4*/   void Run(uint8 initial_texture);
    
    UFUNCTION(BlueprintCallable)        void Stop();
    
    UFUNCTION(BlueprintCallable)        bool IsRunning();
    UFUNCTION(BlueprintCallable)        bool IsInitialized();
    UFUNCTION(BlueprintCallable)        uint8 GetTexture();
    
    // P L A Y B A C K  M A N A G E M E N T :
    
    UFUNCTION(BlueprintCallable)        void PlayNewTexture(uint8 new_texture); // BASIC.
    UFUNCTION(BlueprintCallable)        void PlayNewTextureAfterBridge(uint8 new_texture,
                                        int bridge_index, float fade_out_ratio, float fade_in_ratio,
                                        float bridge_volume = 1.0f);
                                        
    UFUNCTION(BlueprintCallable)        void IncreaseTexture(); // = x * 2 + 1 
                                        //   0000 0001
                                        //   0000 0011
                                        //   0000 0111
                                        //   0000 1111                                      
    UFUNCTION(BlueprintCallable)        void DecreaseTexture(); // = (x - 1) / 2
                                        //   0000 0111
                                        //   0000 0011
                                        //   0000 0001

    UFUNCTION(BlueprintCallable)        void PlayStinger(int index, float stinger_volume = 1.0f);

    UFUNCTION(BlueprintCallable)        void InsertPattern(uint8 index);
    UFUNCTION(BlueprintCallable)        void EjectPattern(uint8 index);
    UFUNCTION(BlueprintCallable)        void BitwiseORing(uint8 mask);
                                        // Useful to insert more than 1 patterns.
    UFUNCTION(BlueprintCallable)        void BitwiseANDing(uint8 mask);
                                        // Useful to eject more than 1 patterns.

    // V O L U M E  M A N A G E M E N T :
    
    UFUNCTION(BlueprintCallable)        void SetPatternVolume(uint8 index, float volume = 1.0f);
    UFUNCTION(BlueprintCallable)        void SetAllPatternsVolume(float volume = 1.0f);
    UFUNCTION(BlueprintCallable)        void InitializeAllPatternsVolume(float ptrn0_vol = 1.0f,
                                        float ptrn1_vol = 1.0f, float ptrn2_vol = 1.0f,
                                        float ptrn3_vol = 1.0f, float ptrn4_vol = 1.0f,
                                        float ptrn5_vol = 1.0f, float ptrn6_vol = 1.0f,
                                        float ptrn7_vol = 1.0f, bool adjust_playback = false);
    UFUNCTION(BlueprintCallable)        void SetMasterVolume(float volume = 1.0f);
    
    // A D V A N C E D  M A T H S :
    
    UFUNCTION(BlueprintCallable)        uint8 BinaryToDecimal(int binary_number);// Just type in binary.
    UFUNCTION(BlueprintCallable)        uint8 BoolArrayToDecimal(bool track0, bool track1, bool track2,
                                        bool track3, bool track4, bool track5, bool track6, bool track7);
                                        // Set the texture as an array of boolean.
                                        
    UFUNCTION(BlueprintCallable)        uint8 FillNbits(uint8 bits, bool play); // Set texture = 2^n - 1, 
                                        // calls PlayNewTexture if play = true.
    UFUNCTION(BlueprintCallable)        uint8 FillNbitsPlusX(uint8 bits, uint8 x, bool condition, bool play);
                                        // Set texture = 2^n - 1, then texture += x if condition equals true.
    UFUNCTION(BlueprintCallable)        uint8 FillNbitsAddB(uint8 bits, uint8 b, bool condition, bool play);
                                        // Set texture = 2^n - 1, then adds pattern b if condition equals true.
    
    // M I S C :
    
    UFUNCTION(BlueprintCallable)        void LogPlaybackTime(uint8 pattern_index);
    UFUNCTION(BlueprintCallable)        void LogAllPlaybackTime(); //Was needed for tests.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//======================================================================================================

/*@B@B@B@B@@@B@B@B@B@B@@@@@B@@@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@@@B@B@MGZ@BME51F5U50B@M0O@B@@@B@B@B@B@@@@@B
B@B@B@B@B@B@@@B@B@B@B@B@B@@@@@B@B@@@B@B@@@@@B@B@B@B@B@B@B@B@B@B@B@XqPXEP2uLvvLvJJuuPNqZ@B@BBB@B@B@B@B@B@
@B@@@B@@@B@B@B@B@B@B@B@B@@@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@BNUU5k21JYL7;r7777rLLu11O@B@BBB@B@M@B@B@B
M@B@B@@@B@B@B@@@B@B@B@B@@@@@B@@@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@PJvu2U15JYL7i7iiiiir;77ju5@@B@M@@@B@@@M@B@
NZ@B@@@B@B@B@B@B@@@B@B@B@B@B@B@B@B@B@B@@@B@B@B@B@B@B@B@B@@@BBjvvvLuvL7vrrrrri:::::i:ii77GB@B@B@@@B@BBB@B
BMZOB@B@B@@@@@B@B@B@B@B@B@@@B@B@B@B@B@B@B@B@B@@@B@B@B@B@B@BS7JvYrrii:i:i:::::i:i:i:i:::;v@B@B@B@B@B@BBB@
MB@EOB@B@B@@@B@B@B@B@@@B@B@@@@@B@B@B@@@@@B@@@B@B@B@@@B@Bk7i:iiiiii::::i:i:::i:iii:iiii:::u@B@B@B@B@B@B@B
OBB@BOO@B@B@B@@@@@B@B@B@B@B@B@B@B@B@B@@@B@@@B@B@B@B@B@7,,::iiii;iiii::ii:iiiiiiiiiiiiiii::B@@@B@M@@@B@B@
@MBOBMO8@B@B@B@B@B@B@@@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B7.i:iiii;riii:i:i:iii:i:iiiiiiii;i;ii.5@@B@B@B@B@@@B
B@MMM@BBO@B@B@@@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@B@BM7,.:rkLi:iiiii:::i:iiiiiiii;iiiiirr7iii:i@B@B@B@B@B@@@
@B@B@@BBBZ@@@B@B@B@@@B@B@B@@@B@B@@@B@B@B@B@B@B@0r,i::::ijOqri:::i:iiiiiiiiiiiiii;i;i;;rii:,E@BB@@B@M@B@B
B@B@BMM@BOZ@B@B@@@B@B@B@@@B@@@B@B@@@B@@@B@B@BMX7r7LUuu7i.:FSLv;::i:i:::ii;iiiiiiiiiri;i;i;:LB@B@B@BBB@M@
BB@B@BBB@@0O@B@@@@@B@B@B@B@B@B@B@B@B@B@B@BPi   :rvLY5EO5FL::::FG7i::.:r:iiiiiiii;iii;iiiii::@B@B@@@B@B@B
B@B@B@B@@@BPO@B@@@@@B@B@B@B@B@B@B@@@B@@Mi              ,rv5Fr,,iiur.v0uYiii;iiiii;iiiriiii::S@B@B@B@B@B@
@B@B@B@B@B@B8O@B@B@B@B@@@@@B@@@@@B@B@j         .::i:,   ...:1Zv.:8@B@B87iiiiiiriiiri;i;i;ii:iB@B@B@B@B@@
B@B@B@BBB@B@B@BGB@B@B@B@@@B@@@B@Bk7.     . . .:7i:  ,FEMM@@@@@BBi:N@@MU:iiiiiri;i;i;iiiii;i:,S@@B@B@B@B@
@B@B@B@GMOOOMM@u.7UMMGBB@@M1;,          ... .::..:7G@B@B@B@M:. LBi.i:j;ir7riiir;iiriii;i;i;:::@@@@@B@B@B
B@BBB@MMM@MOZOM@. ,L; .i:                .. .,,,:S@@Z,M@B@M,   ,,Uji  2rrL7;r;ir;;iririiiiii:iu@B@B@B@@@
@B@M@B@B@B@BO0G@k iu7,:uJ    .,.:i,         ..:::.     :v;,,:,:..:FLL :1r77irirrr;ri;i;iiiiirr7B@B@B@B@B
BMBBB@B@B@B@BBM@B :ur,,BBX::::...:rkM@B0:  ...,::i:,::.   ....,: .:kvi:Miv77rrirrr;riiirii:ivY77B@B@BBB@
BM@B@B@B@BB8BB@B@,:vY.7B@BO::r7E@B@B@@BXME, ..,::::::,:,,,...., . .rk77Bvr777r7rrrrrririr:irjJv:FB@@@B@B
B@B@B@BBOBMBM@M@Bi.vv,Y@Z8@@B@B@S@@@Bu   7M, ,,,,,...:,,.... . .   :jJ:S1rr7rr;r;r;riiiirrivJ5uL:BB@B@B@
@B@B@@@MMMMMBMMB@7.rj.UBB8MM@BU:  ::. ,::.;2 .....,....             iur:j77r7irrii;iririr1jLL5q5JU@B@B@B
B@B@B@B@B@BMMBOB@u.ir,Y@BMM8M@, .,.,.:::::.v: .. . ... . ...   .:.  rSrii7rrrriiriiiii;rrUEFLLZNFL0@B@B@
@M@BBB@B@B@MBB@M@Y.:i:7BM8@BMO@: .:::::,:::ii .............  .uSjEL u1vrrirrri;i7riiiirrv;kkPJkEN2LG@B@B
B@B@B@B@B@BMOBMOBJ.7:::OBBM80MB@: .,.::::::ii  . . . ... . . u7  uv Lj7iir;rrririuiiii:7L7vMMOuBBO1rq@B@
@MBBBBBB@B@BBB@Z@F.S::.7@B@MOOZZ@: .,,:,::::i  .... ... ...  7L    ,uq7:;rrir;;ii7j:iii7FjvF@O80@BM02N@B
BBB@M@@@B@B@M@M8B8.LP.:.SMZ@BEqN8@; .,.,,:.,:  ..  ... .....  1OY77EFviriririiiiir77i;rrLk2uO@BGZ@B@BUY@
@B@BBB@B@B@BB@B8ZB:,@F::,PO0OZEkqG@r  ..:,,.r.      ..  . ...  Y@EG0uLrU;:iiriirr7;77i7riuOq5B@BO8@B@G1Y
@@M@B@B@B@B@B@MOG@r,Y@Si,,uOZZZMZGZ@u  .,,,,:FLr,  . ..... ...  uuv7i:OP:;:;i;i7rJr7JLi;i:j@GZ@@@G8@@@8P
@B@B@B@B@B@@@BBGZBF.:B@M7,:r5ZZ8MOO8@B. ..,,,.;uq    .. . ...    uJvuiFiLr:ii:ir72L7JuLir;ij@BGB@@q0@B@Z
B@@@B@BBB@B@B@B@O@B..2B@@Gr:i75MMM88G@B7   ,,,.. . ,.,.... .    B@B@B@v:Jii;:i;rrqJLvvkFLJ7vY@B8B@B10@BB
@B@@@B@B@B@B@B@B@B@v :M@@B@1rri7qB@@@B@B@:   ,::ii:i.  ....   :B@B@B@S:ru,i:i:iiJJkuL7vk0jLuuvM@ZB@@L0@B
B@B@B@B@B@BBM@BBB@B@ .:BB@B@BMur:ruSqBB@B@Br   ,,:...  . .   LB@B@B@B::Ur:i::i:707u0FuLLF88FZXvGBBB@BYN@
@B@B@BBB@B@B@@MM@B@@B  :BB@B@@@@M2r::LuLYE@B@L   .:.. . .   MB@B@B@@L:iZii:i;:rZkvY8E5uYYuOZqZX75B@B@BrX
BBB@B@B@B@B@MMMBMBB@B@r..0@@B@B@B@BGi:2@Eu7uB@BJ          7@B@B@B@B@,:J0:r:iiiSMkP7FOO8Oq1;1B@MO7uB@B@Ou
@B@B@BBB@MBMBMBOMOBBBB@Ei.UB@B@B@B@B@P.:@B@SMB@B@Mr     :B@B@@@B@B@v.,Oi;iiiii@BBZkU@B@B@BOLuB@B8:uBGP@B
B@B@@@B@MMMBMBMMOMMM8@B@Mu:ik@B@B@B@B@BJrM@@B@B@B@B@OS0@B@B@B@B@B@M,.J8:ii7::LB@B@B@B@@@B@B@qLB@BB;ZBkU@
@@BB@BBMMOBMOMMOMO8GZ0X2k8EU7:1@@B@B@B@Bk7@B@@@B@B@B@B@B@B@B@@@B@Br,.B7iiLv::O@B@@@B@B@B@B@B@MuO@@Mv@BP0
B@B@BBB@BBMBOMZOOZN0EZqXFkqOOqr72OB@B@BMM08@@@B@B@B@@@@@B@B@B@B@BO,.q@:rrj:i;@@@B@B@B@B@B@B@B@MUM@BMZ@B8
@@BM@M@@@BMM@B@MMMOEMMOEG08ZOO@MPYuG@B@BO0MB@@@B@B@B@B@B@B@B@B@@@i.:@riivii,OB@B@B@B@@@B@B@@@B@@8B@@OO@O
MMOMBMM@B@B@B@BMMBO8OBGGZ88OM@@@B@OU2@MM@OB@B@B@B@B@B@@@B@B@B@B@O  BB,i77i:vB@B@B@B@B@B@B@B@B@@@M0Fqq10P
GO@MMBMOMMMM@MOGGZMO8GGEZGMMBB@B@@@BO2MOO8@B@B@@@B@B@@@B@B@B@@@Br 7B::;7rriB@B@B@B@B@B@BBEPN0qPv7u0Xq5*/
    

    private:
    
        UPROPERTY()         uint8 __texture;

        UPROPERTY()         UAdaptiveScore* __default_adaptive_score;   
        UPROPERTY()         UDynamicFilterChain* __default_dynamic_filter_chain;    
    
        UPROPERTY()         UAudioComponent* __pattern_audio_components[PTRN_COUNT];    
        UPROPERTY()         UAudioComponent* __bridge_audio_component; 
        UPROPERTY()         UAudioComponent* __stinger_audio_component;  
        UPROPERTY()         TArray<UAudioComponent*> __all_audio_components;
                
        UPROPERTY()         TArray<USoundCue*> __bridge_sound_cues;        
        UPROPERTY()         TArray<USoundCue*> __stinger_sound_cues;
        
        UPROPERTY()         float __patterns_volume[PTRN_COUNT];
        UPROPERTY()         float __master_volume;
        UPROPERTY()         uint8 __patterns_validation[PTRN_COUNT];
        
        UPROPERTY()         FTimerHandle __bridge_timer_handle;
    
        UPROPERTY()         UAdaptiveScore* __loaded_score; 
        UPROPERTY()         float __score_fade_time;
        UPROPERTY()         uint8 __score_filterchain_index;
       
        UPROPERTY()         bool __is_initialized;
        UPROPERTY()         bool __is_running;
    
    private:
        
        UFUNCTION()         void __beginToPlaySilently();       
        UFUNCTION()         void __muteTrack(uint8 index, float fade);  
        UFUNCTION()         void __decodeFromByte(uint8 processed_texture, float fade);     
        UFUNCTION()         void __adjustPatternVolume(uint8 index, float volume, float fade);      
        UFUNCTION()         uint8 __getFilteredTexture();
        
        UFUNCTION()         void __onBridgeCrossfadeTimer(float fade); 
        
        UFUNCTION()         float __verifiedVolume(float volume);
        UFUNCTION()         void __initializeDefaultVolume();
        
        UFUNCTION()         bool __isSoundBaseValid(USoundBase* base_ptr);
                
    public:
    
        AAdaptiveMixer();
        ~AAdaptiveMixer();
    
};