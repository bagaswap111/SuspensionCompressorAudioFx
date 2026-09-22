# Building Suspension Compressor for VST/JUCE

## Prerequisites

### All Platforms
- **CMake**: Version 3.15 or higher
  - Windows: https://cmake.org/download/
  - macOS: `brew install cmake`
  - Linux: `sudo apt-get install cmake` or `sudo yum install cmake`

- **C++ Compiler** with C++17 support
  - Windows: Visual Studio 2019/2022 with C++ workload
  - macOS: Xcode Command Line Tools (`xcode-select --install`)
  - Linux: GCC 7+ or Clang 5+

### Platform-Specific Requirements

#### Windows
- Visual Studio 2019 or 2022 (Community Edition is free)
  - Install "Desktop development with C++" workload
- Windows SDK 10.0.17763.0 or later

#### macOS
- Xcode 11.0 or later
- macOS 10.13 (High Sierra) or later for AU support
- For Audio Units: Must be signed with Apple Developer certificate

#### Linux
- GCC 7+ or Clang 5+
- libasound2-dev (ALSA)
- libjack-jackd2-dev (optional, for JACK support)
- libfreetype6-dev, libx11-dev, libxcomposite-dev, libxcursor-dev, libxinerama-dev, libxrandr-dev (for GUI)

```bash
# Ubuntu/Debian
sudo apt-get install \
    build-essential \
    git \
    cmake \
    libasound2-dev \
    libjack-jackd2-dev \
    libfreetype6-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxrandr-dev

# Fedora/RHEL
sudo dnf install \
    gcc-c++ \
    git \
    cmake \
    alsa-lib-devel \
    jack-audio-connection-kit-devel \
    freetype-devel \
    libX11-devel \
    libXcomposite-devel \
    libXcursor-devel \
    libXinerama-devel \
    libXrandr-devel
```

## Build Instructions

### Windows (Visual Studio)

```powershell
cd SuspensionCompressor
mkdir build
cd build

# Generate Visual Studio 2022 solution
cmake .. -G "Visual Studio 17 2022" -A x64

# Build Release configuration
cmake --build . --config Release

# Optional: Build Debug configuration
cmake --build . --config Debug
```

**Output locations:**
- VST3: `build/SuspensionCompressor_artefacts/Release/VST3/Suspension Compressor.vst3`
- Standalone: `build/SuspensionCompressor_artefacts/Release/Standalone/Suspension Compressor.exe`

### macOS (Xcode)

```bash
cd SuspensionCompressor
mkdir build
cd build

# Generate Xcode project
cmake .. -G Xcode

# Build Release
cmake --build . --config Release

# Or build using xcodebuild directly
xcodebuild -project SuspensionCompressor.xcodeproj -configuration Release -alltargets
```

**Output locations:**
- VST3: `build/SuspensionCompressor_artefacts/Release/VST3/Suspension Compressor.vst3`
- AU: `build/SuspensionCompressor_artefacts/Release/AU/Suspension Compressor.component`
- Standalone: `build/SuspensionCompressor_artefacts/Release/Standalone/Suspension Compressor.app`

**Installation:**
```bash
# Copy VST3 to system folder
cp -r build/SuspensionCompressor_artefacts/Release/VST3/"Suspension Compressor.vst3" ~/Library/Audio/Plug-Ins/VST3/

# Copy AU to system folder (requires codesigning for macOS 10.15+)
cp -r build/SuspensionCompressor_artefacts/Release/AU/"Suspension Compressor.component" ~/Library/Audio/Plug-Ins/Components/

# Refresh Audio Unit cache
killall AudioComponentRegistrar
```

### Linux

```bash
cd SuspensionCompressor
mkdir build
cd build

# Generate Makefiles
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Or using make directly
make -j$(nproc)
```

**Output locations:**
- VST3: `build/SuspensionCompressor_artefacts/Release/VST3/Suspension Compressor.vst3`
- LV2: `build/SuspensionCompressor_artefacts/Release/LV2/Suspension Compressor.lv2`
- Standalone: `build/SuspensionCompressor_artefacts/Release/Standalone/Suspension Compressor`

**Installation:**
```bash
# Create plugin directories
mkdir -p ~/.vst3
mkdir -p ~/.lv2

# Copy plugins
cp -r build/SuspensionCompressor_artefacts/Release/VST3/"Suspension Compressor.vst3" ~/.vst3/
cp -r build/SuspensionCompressor_artefacts/Release/LV2/"Suspension Compressor.lv2" ~/.lv2/
```

## CMake Options

You can customize the build with these CMake options:

```bash
# Disable specific plugin formats
cmake .. -DJUCE_BUILD_VST3=OFF
cmake .. -DJUCE_BUILD_AU=OFF
cmake .. -DJUCE_BUILD_LV2=OFF
cmake .. -DJUCE_BUILD_STANDALONE=OFF

# Enable VST2 support (requires VST2 SDK)
cmake .. -DJUCE_VST3_CAN_REPLACE_VST2=ON

# Set custom VST3 copy directory
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/plugins
```

## Troubleshooting

### Common Issues

#### "JUCE framework not found"
The JUCE framework is automatically downloaded during the first build via FetchContent. Ensure you have an internet connection during the initial CMake configuration.

#### "Compiler does not support C++17"
Update your compiler:
- Windows: Install latest Visual Studio
- macOS: Update Xcode (`xcode-select --install`)
- Linux: Install newer GCC/Clang from your package manager

#### "Audio Unit validation failed" (macOS)
macOS 10.15+ requires Audio Units to be code-signed:
```bash
codesign --force --sign "Developer ID Application: Your Name" \
  ~/Library/Audio/Plug-Ins/Components/"Suspension Compressor.component"
```

#### "Plugin not showing in DAW"
1. Verify the plugin was built successfully
2. Check that your DAW scans the correct plugin folder
3. Try rescaning plugins in your DAW
4. Check DAW compatibility (some DAWs only support specific formats)

#### Build errors related to JUCE
Delete the build directory and reconfigure:
```bash
rm -rf build
mkdir build
cd build
cmake ..
```

## Development Tips

### Debugging
Build with Debug configuration and use your IDE's debugger:
```bash
# Windows
cmake --build . --config Debug

# macOS/Linux
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Hot Reload (Development)
For faster iteration during development, you can use the standalone app:
```bash
# Run the standalone version
./build/SuspensionCompressor_artefacts/Debug/Standalone/Suspension\ Compressor
```

### Performance Testing
Build with Release configuration and enable LTO (Link Time Optimization):
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## Plugin Formats Explained

- **VST3**: Steinberg's format, works on Windows, macOS, and Linux. Supported by most modern DAWs.
- **AU (Audio Units)**: Apple's format, macOS only. Required for Logic Pro and GarageBand.
- **LV2**: Open standard, primarily for Linux. Supported by some DAWs like Ardour.
- **Standalone**: Independent application, useful for testing and quick audio processing.

## License Notes

This plugin uses the JUCE framework, which is licensed under GPL v3 for open-source projects or requires a commercial license for proprietary/closed-source distribution. See https://juce.com/licensing for details.
