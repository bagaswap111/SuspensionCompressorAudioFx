#pragma once

#include <JuceHeader.h>
#include <cmath>

/**
 * SuspensionModel - Implements mass-spring-damper physics for audio compression
 * 
 * Based on the equation: m*a + c*v + k*x = F
 * where:
 *   m = mass (affects response speed)
 *   c = damping coefficient (ζ - zeta)
 *   k = spring constant (affects compression ratio feel)
 *   x = position (gain reduction)
 *   v = velocity
 *   a = acceleration
 *   F = force (from audio signal exceeding threshold)
 */
class SuspensionModel
{
public:
    enum class SuspensionType
    {
        MacPherson = 0,
        DoubleWishbone,
        MultiLink,
        AirSuspension,
        MagneticSuspension,
        LeafSpring,
        TorsionBeam,
        DeDionAxle,
        NumTypes
    };

    struct SuspensionParams
    {
        float mass;           // kg - affects response speed
        float springConstant; // N/m - affects ratio feel
        float dampingCoeff;   // Ns/m - ζ (zeta) damping
        float attackMs;       // ms
        float releaseMs;      // ms
        float minRatio;
        float maxRatio;
        float defaultKnee;
        bool hasAutoRelease;
        bool hasAdaptiveDamping;
    };

private:
    SuspensionType currentType = SuspensionType::DoubleWishbone;
    SuspensionParams params;
    
    // State variables (per channel)
    float position = 0.0f;   // Current gain reduction
    float velocity = 0.0f;   // Rate of change
    float prevForce = 0.0f;
    
    // Sample rate dependent
    float sampleRate = 44100.0f;
    float dt = 1.0f / 44100.0f;
    
    // Auto-release state
    float adaptiveReleaseTime = 0.0f;
    float lastGainReduction = 0.0f;
    
    // Look-ahead
    int lookAheadSamples = 0;
    
public:
    SuspensionModel()
    {
        setSuspensionType(SuspensionType::DoubleWishbone);
    }
    
    void prepare(double newSampleRate)
    {
        sampleRate = static_cast<float>(newSampleRate);
        dt = 1.0f / sampleRate;
        reset();
    }
    
    void reset()
    {
        position = 0.0f;
        velocity = 0.0f;
        prevForce = 0.0f;
        adaptiveReleaseTime = params.releaseMs;
        lastGainReduction = 0.0f;
    }
    
    void setSuspensionType(SuspensionType type)
    {
        currentType = type;
        
        switch(type)
        {
            case SuspensionType::MacPherson:
                params = {1.0f, 100.0f, 10.0f, 15.0f, 120.0f, 2.0f, 6.0f, 30.0f, false, false};
                break;
            case SuspensionType::DoubleWishbone:
                params = {0.5f, 200.0f, 8.0f, 5.0f, 80.0f, 4.0f, 10.0f, 20.0f, false, false};
                break;
            case SuspensionType::MultiLink:
                params = {1.5f, 60.0f, 15.0f, 40.0f, 400.0f, 1.5f, 4.0f, 50.0f, false, false};
                break;
            case SuspensionType::AirSuspension:
                params = {2.0f, 50.0f, 20.0f, 30.0f, 400.0f, 2.0f, 8.0f, 40.0f, true, true};
                break;
            case SuspensionType::MagneticSuspension:
                params = {0.3f, 300.0f, 15.0f, 1.0f, 100.0f, 3.0f, 12.0f, 10.0f, false, true};
                break;
            case SuspensionType::LeafSpring:
                params = {3.0f, 40.0f, 6.0f, 40.0f, 600.0f, 6.0f, 20.0f, 60.0f, false, false};
                break;
            case SuspensionType::TorsionBeam:
                params = {1.2f, 80.0f, 12.0f, 25.0f, 250.0f, 2.0f, 6.0f, 35.0f, false, false};
                break;
            case SuspensionType::DeDionAxle:
                params = {0.8f, 150.0f, 10.0f, 12.0f, 150.0f, 3.0f, 8.0f, 25.0f, false, false};
                break;
            default:
                params = {1.0f, 100.0f, 10.0f, 15.0f, 120.0f, 2.0f, 6.0f, 30.0f, false, false};
                break;
        }
        
        reset();
    }
    
    SuspensionType getSuspensionType() const { return currentType; }
    
    const SuspensionParams& getParams() const { return params; }
    
    void setDamping(float zeta)
    {
        // Convert zeta (damping ratio) to damping coefficient
        // c = 2 * zeta * sqrt(k * m)
        params.dampingCoeff = 2.0f * zeta * std::sqrt(params.springConstant * params.mass);
    }
    
    float getDamping() const
    {
        // Convert back to zeta
        return params.dampingCoeff / (2.0f * std::sqrt(params.springConstant * params.mass));
    }
    
    void setLookAhead(int samples)
    {
        lookAheadSamples = samples;
    }
    
    /**
     * Process one sample using mass-spring-damper physics
     * 
     * @param inputSignal Current input level (in dB, relative to threshold)
     * @param threshold Compression threshold (dB)
     * @param ratio Compression ratio
     * @return Gain reduction amount (positive = attenuation)
     */
    float processSample(float inputSignal, float threshold, float ratio)
    {
        // Calculate force from audio signal exceeding threshold
        float signalAboveThreshold = juce::jmax(0.0f, inputSignal - threshold);
        
        // Apply ratio to determine target gain reduction
        float targetReduction = signalAboveThreshold * (1.0f - 1.0f / ratio);
        
        // Calculate force based on difference between current and target
        float force = (targetReduction - position) * params.springConstant;
        
        float acceleration = 0.0f;

        // Adaptive damping for magnetic/air suspension
        if (params.hasAdaptiveDamping)
        {
            // Increase damping for large transients
            float transientAmount = std::abs(force - prevForce);
            float adaptiveDamping = params.dampingCoeff * (1.0f + transientAmount * 0.1f);
            
            // Mass-spring-damper: m*a = F - c*v - k*x
            acceleration = (force - adaptiveDamping * velocity - params.springConstant * position) / params.mass;
            prevForce = force;
        }
        else
        {
            // Standard mass-spring-damper equation
            acceleration = (force - params.dampingCoeff * velocity - params.springConstant * position) / params.mass;
        }
        
        // Integrate using semi-implicit Euler method
        velocity += acceleration * dt;
        position += velocity * dt;
        
        // Ensure position doesn't go negative (no gain boost from compressor)
        position = juce::jmax(0.0f, position);
        
        // Auto-release behavior
        if (params.hasAutoRelease && position < lastGainReduction * 0.5f)
        {
            // Slow down release when gain reduction is decreasing rapidly
            adaptiveReleaseTime = juce::jmin(params.releaseMs * 2.0f, 
                                            params.releaseMs * (1.0f + lastGainReduction * 0.1f));
        }
        else
        {
            adaptiveReleaseTime = params.releaseMs;
        }
        
        lastGainReduction = position;
        
        return position;
    }
    
    float getCurrentGainReduction() const { return position; }
    
    float getAttackMs() const { return params.attackMs; }
    float getReleaseMs() const { return params.hasAutoRelease ? adaptiveReleaseTime : params.releaseMs; }
    
    static const char* getSuspensionName(SuspensionType type)
    {
        switch(type)
        {
            case SuspensionType::MacPherson: return "MacPherson Strut";
            case SuspensionType::DoubleWishbone: return "Double Wishbone";
            case SuspensionType::MultiLink: return "Multi-Link";
            case SuspensionType::AirSuspension: return "Air Suspension";
            case SuspensionType::MagneticSuspension: return "Magnetic Suspension";
            case SuspensionType::LeafSpring: return "Leaf Spring";
            case SuspensionType::TorsionBeam: return "Torsion Beam";
            case SuspensionType::DeDionAxle: return "De Dion Axle";
            default: return "Unknown";
        }
    }
    
    static const char* getCarExample(SuspensionType type)
    {
        switch(type)
        {
            case SuspensionType::MacPherson: return "Honda Civic, Toyota Corolla";
            case SuspensionType::DoubleWishbone: return "Ferrari 488, Lamborghini Huracán";
            case SuspensionType::MultiLink: return "Porsche 911, BMW 5 Series";
            case SuspensionType::AirSuspension: return "Rolls-Royce Phantom, Mercedes S-Class";
            case SuspensionType::MagneticSuspension: return "Corvette C8, Audi R8";
            case SuspensionType::LeafSpring: return "Ford F-150, Dakar Rally Truck";
            case SuspensionType::TorsionBeam: return "VW Golf, Toyota Yaris";
            case SuspensionType::DeDionAxle: return "Alfa Romeo GTV, Lancia Stratos";
            default: return "Unknown";
        }
    }
};
