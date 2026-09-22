#pragma once

#include <JuceHeader.h>

/**
 * Preset definitions for Suspension Compressor
 * Based on real car models and their suspension characteristics
 */
struct SuspensionPreset
{
    const char* name;
    const char* carModel;
    SuspensionModel::SuspensionType type;
    float threshold;
    float ratio;
    float attack;
    float release;
    float knee;
    float makeupGain;
    float damping;
    float mix;
    bool autoRelease;
    float lookAhead;
};

constexpr SuspensionPreset factoryPresets[] = {
    // Sport/Performance Presets
    {"Ferrari 488 GTB", "Ferrari 488 GTB (2015)", SuspensionModel::SuspensionType::DoubleWishbone, 
     -18.0f, 6.0f, 5.0f, 80.0f, 20.0f, 3.0f, 0.7f, 100.0f, false, 0.0f},
    
    {"Lamborghini Huracán", "Lamborghini Huracán (2014)", SuspensionModel::SuspensionType::MagneticSuspension,
     -15.0f, 8.0f, 1.0f, 100.0f, 15.0f, 4.0f, 0.8f, 100.0f, false, 2.0f},
    
    {"Porsche 911 GT3", "Porsche 911 GT3 (991)", SuspensionModel::SuspensionType::MultiLink,
     -20.0f, 4.0f, 15.0f, 200.0f, 35.0f, 2.0f, 0.85f, 100.0f, false, 0.0f},
    
    {"BMW M3 Competition", "BMW M3 Competition (G80)", SuspensionModel::SuspensionType::AirSuspension,
     -16.0f, 5.0f, 20.0f, 300.0f, 40.0f, 2.5f, 1.0f, 100.0f, true, 0.0f},
    
    // Luxury Presets
    {"Rolls-Royce Phantom", "Rolls-Royce Phantom VIII", SuspensionModel::SuspensionType::AirSuspension,
     -24.0f, 2.0f, 50.0f, 600.0f, 60.0f, 1.0f, 1.3f, 100.0f, true, 0.0f},
    
    {"Mercedes S-Class", "Mercedes-Benz S-Class (W223)", SuspensionModel::SuspensionType::AirSuspension,
     -22.0f, 2.5f, 40.0f, 500.0f, 50.0f, 1.5f, 1.2f, 100.0f, true, 0.0f},
    
    {"Aston Martin DB11", "Aston Martin DB11", SuspensionModel::SuspensionType::MultiLink,
     -18.0f, 3.5f, 30.0f, 350.0f, 45.0f, 2.0f, 1.1f, 100.0f, false, 0.0f},
    
    // Off-Road/Truck Presets
    {"Dakar Rally Truck", "Ford F-150 Dakar Rally", SuspensionModel::SuspensionType::LeafSpring,
     -12.0f, 10.0f, 40.0f, 800.0f, 10.0f, 6.0f, 0.5f, 100.0f, false, 0.0f},
    
    {"Toyota Land Cruiser", "Toyota Land Cruiser 300", SuspensionModel::SuspensionType::TorsionBeam,
     -20.0f, 4.0f, 25.0f, 300.0f, 35.0f, 3.0f, 0.9f, 100.0f, false, 0.0f},
    
    // Economy/Everyday Presets
    {"Honda Civic Type R", "Honda Civic Type R (FK8)", SuspensionModel::SuspensionType::MacPherson,
     -16.0f, 5.0f, 15.0f, 150.0f, 25.0f, 3.5f, 0.8f, 100.0f, false, 0.0f},
    
    {"VW Golf GTI", "Volkswagen Golf GTI (Mk8)", SuspensionModel::SuspensionType::TorsionBeam,
     -18.0f, 4.0f, 20.0f, 200.0f, 30.0f, 2.5f, 0.85f, 100.0f, false, 0.0f},
    
    // Classic/Performance Hybrid
    {"Alfa Romeo GTV", "Alfa Romeo GTV (Classic)", SuspensionModel::SuspensionType::DeDionAxle,
     -14.0f, 6.0f, 12.0f, 180.0f, 20.0f, 4.0f, 0.75f, 100.0f, false, 0.0f},
    
    {"Lancia Stratos", "Lancia Stratos HF (Rally)", SuspensionModel::SuspensionType::DeDionAxle,
     -10.0f, 8.0f, 8.0f, 150.0f, 15.0f, 5.0f, 0.65f, 100.0f, false, 0.0f},
    
    // Specialized Applications
    {"Corvette C8 Z06", "Chevrolet Corvette C8 Z06", SuspensionModel::SuspensionType::MagneticSuspension,
     -14.0f, 7.0f, 2.0f, 120.0f, 18.0f, 4.5f, 0.75f, 100.0f, false, 3.0f},
    
    {"Tesla Model S Plaid", "Tesla Model S Plaid", SuspensionModel::SuspensionType::AirSuspension,
     -20.0f, 3.0f, 25.0f, 400.0f, 40.0f, 2.0f, 1.15f, 100.0f, true, 1.0f}
};

constexpr int numFactoryPresets = sizeof(factoryPresets) / sizeof(factoryPresets[0]);

inline const SuspensionPreset& getPreset(int index)
{
    if (index >= 0 && index < numFactoryPresets)
        return factoryPresets[index];
    
    // Return default preset if index out of bounds
    return factoryPresets[0];
}

inline int findPresetByName(const juce::String& name)
{
    for (int i = 0; i < numFactoryPresets; ++i)
    {
        if (juce::String(factoryPresets[i].name) == name)
            return i;
    }
    return -1; // Not found
}
