// AleahRise v1.04 -- adaptive soundtrack system for UE4
// © Daniel Winterreise, 2019

#include "FilterChain.h"

constexpr int BYTE_SIZE = 8;

void UStaticFilterChain::BoolArrayFromByte(uint8 byte_var, bool bool_array[BYTE_SIZE]) {
    for (int i = 0; i < BYTE_SIZE; ++i) {
        bool_array[i] = (byte_var & (1 << i)) != 0;
    }
}

uint8 UStaticFilterChain::ApplyFilterChain(uint8 source_texture, uint8 filterchain_index) {
    uint8 result = source_texture;
    bool tracks[BYTE_SIZE];
    BoolArrayFromByte(source_texture, tracks);

    //=================================================================================================
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //TYPE YOUR CODE HERE:

    //Example:
    if (filterchain_index == 42) {
        if (tracks[2] == true) {
            uint8 mask = 6; // 0000 0110 bin;
            result = mask & source_texture;
        }
        if (tracks[3] == true) {
            uint8 mask2 = 13; // 0000 1101 bin;
            result = mask2 & source_texture;
        }
    }

    //CYSMA filter chain:
    if (filterchain_index == 66) {
        uint8 mask = 31;
        if (tracks[2]) { mask = 21; }
        if (tracks[3]) { mask = 24; }
        result = mask & source_texture;
    }
    
    if (filterchain_index == 67) {
        uint8 mask = 31;
        if (tracks[2]) { mask = 21; }
        if (tracks[3]) { mask = 24; }
        result = mask & source_texture;
        if (tracks[4]) { 
            mask = 29;
            result &= mask;
        }
    }
    
    if (filterchain_index == 68) {
        uint8 mask = 31;
        if (tracks[0]) { mask = 17; }
        if (tracks[1]) { mask = 18; }
        if (tracks[2]) { mask = 20; }
        if (tracks[3]) { mask = 24; }
        result = mask & source_texture;
    }
    
    if (filterchain_index == 69) {
        uint8 mask = 31;
        if (tracks[0]) { mask = 1; }
        if (tracks[1]) { mask = 2; }
        if (tracks[2]) { mask = 4; }
        if (tracks[3]) { mask = 8; }
        if (tracks[4]) { mask = 16; }
        result = mask & source_texture;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //=================================================================================================

    return result;
}

uint8 FFilter::Apply(uint8 source_texture, bool & changed) {
    
    uint8 result = source_texture;
    bool tracks[BYTE_SIZE];
    bool c = false;
    UStaticFilterChain::BoolArrayFromByte(source_texture, tracks);
    if (tracks[__track]) {
        if (__operation == TEXT("or")) {
            result = source_texture | __mask;
            c = true;
        }
        else if (__operation == TEXT("and")) {
            result = source_texture & __mask;
            c = true;
        }
        else if (__operation == TEXT("xor")) {
            result = source_texture ^ __mask;
            c = true;
        }
    }
    changed = c;
    return result;
}

void UDynamicFilterChain::AddFilter(uint8 track, FString operation, uint8 mask, bool terminate_subchain) {
    
    FFilter f;
    f.SetTrack(track);
    f.SetOperation(operation);
    f.SetMask(mask);
    f.SetTerminate(terminate_subchain);
    __chain.Add(f); 
}



void UDynamicFilterChain::Clear() {
    __chain.Empty();
}


uint8 UDynamicFilterChain::ApplyDynamicFilterChain(uint8 source_texture) {
    
    uint8 result = source_texture;
    uint8 src = source_texture;
    for (int i = 0; i < __chain.Num(); ++i) {
        bool changed;
        uint8 r = __chain[i].Apply(src, changed);
        if (changed) {
            result = r;
        }
        if (__chain[i].IsTerminate()) {
            src = result;
        }
    }
    return result;
}

bool UDynamicFilterChain::IsAvailable() {
    if (__chain.Num() > 0)
        return true;
    else
        return false;
}

UDynamicFilterChain::UDynamicFilterChain() {
    UE_LOG(LogTemp, Display, TEXT("Dynamic filter chain created."));
}

UDynamicFilterChain::~UDynamicFilterChain() {
    UE_LOG(LogTemp, Display, TEXT("Dynamic filter chain destroyed."));
}