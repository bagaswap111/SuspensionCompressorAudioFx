# Suspension Compressor VST Plugin

A unique audio compressor plugin that uses mass-spring-damper physics simulation based on real vehicle suspension systems.

## Features

### 8 Suspension Types
- **MacPherson Strut** - Fast response, moderate compression (Honda Civic, Toyota Corolla)
- **Double Wishbone** - Ultra-fast, aggressive compression (Ferrari 488, Lamborghini Huracán)
- **Multi-Link** - Smooth, luxury compression (Porsche 911, BMW 5 Series)
- **Air Suspension** - Adaptive, comfort-oriented (Rolls-Royce Phantom, Mercedes S-Class)
- **Magnetic Suspension** - Instant response, real-time adaptive (Corvette C8, Audi R8)
- **Leaf Spring** - Heavy-duty, bouncy character (Ford F-150, Dakar Rally Truck)
- **Torsion Beam** - Balanced, everyday use (VW Golf, Toyota Yaris)
- **De Dion Axle** - Precise with slight overshoot (Alfa Romeo GTV, Lancia Stratos)

### Parameters
- **Threshold**: -60 to 0 dB
- **Ratio**: 1:1 to 20:1
- **Attack**: 0.1 to 100 ms
- **Release**: 10 to 1000 ms
- **Knee**: 0 to 100 dB (soft to hard)
- **Makeup Gain**: -12 to +24 dB
- **Mix**: 0 to 100% (parallel compression)
- **Damping (ζ)**: 0.1 to 2.0 (unique suspension parameter)
- **Look-ahead**: 0 to 10 ms

### Factory Presets (16 Car Models)
- Ferrari 488 GTB
- Lamborghini Huracán
- Porsche 911 GT3
- BMW M3 Competition
- Rolls-Royce Phantom
- Mercedes S-Class
- Aston Martin DB11
- Dakar Rally Truck
- Toyota Land Cruiser
- Honda Civic Type R
- VW Golf GTI
- Alfa Romeo GTV
- Lancia Stratos
- Corvette C8 Z06
- Tesla Model S Plaid

## Building

### Requirements
- CMake 3.15 or higher
- C++17 compatible compiler
- JUCE framework (automatically downloaded during build)

### Windows (Visual Studio)
```bash
cd SuspensionCompressor
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### macOS (Xcode)
```bash
cd SuspensionCompressor
mkdir build && cd build
cmake .. -G Xcode
cmake --build . --config Release
```

### Linux
```bash
cd SuspensionCompressor
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Output Formats
- VST3
- AU (macOS only)
- LV2 (Linux)
- Standalone Application

## UI Design
The interface is inspired by luxury sports car dashboards:
- Carbon fiber black background
- Brushed aluminum knobs
- Ferrari red accents
- Real-time gain reduction metering
- Suspension physics visualization

## Physics Model

The compressor uses the mass-spring-damper equation:

```
m*a + c*v + k*x = F
```

Where:
- m = mass (affects response speed)
- c = damping coefficient (ζ - zeta)
- k = spring constant (affects ratio feel)
- x = position (gain reduction)
- v = velocity
- a = acceleration
- F = force (from audio signal exceeding threshold)

## License

This project is provided as-is for educational and creative purposes.

## Credits

Concept and Implementation: SuspensionAudio
Physics Model: Based on classical mass-spring-damper systems
UI Design: Inspired by luxury sports car dashboards
