# ML Smart Selection Build Instructions

## New CMake Option

### ENABLE_ONNX_ML

Enable ML-based smart area selection using ONNX Runtime and YOLO models.

**Default:** OFF  
**Requirements:** ONNX Runtime library installed

### Usage

```bash
# Basic build with ML support
cmake .. -DENABLE_ONNX_ML=ON

# Specify custom ONNX Runtime path
cmake .. -DENABLE_ONNX_ML=ON -DONNXRUNTIME_ROOT=/path/to/onnxruntime

# Full build example
mkdir build && cd build
cmake .. \
  -DENABLE_ONNX_ML=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Features When Enabled

- Press `Ctrl+S` in capture mode to activate smart selection
- Automatically detects objects in screenshots
- Click detected regions to select them instantly
- Supports 80+ object classes (people, vehicles, electronics, etc.)
- Optional GPU acceleration with CUDA

### Prerequisites

1. **ONNX Runtime**: Download from https://github.com/microsoft/onnxruntime/releases
2. **ML Model**: Download YOLOv8n.onnx to `~/.flameshot/models/`

See [docs/ML-QuickStart.md](ML-QuickStart.md) for complete setup instructions.

### Runtime Dependencies

When `ENABLE_ONNX_ML=ON`:
- `libonnxruntime.so` (Linux) / `onnxruntime.dll` (Windows) must be in library path
- YOLO ONNX model file (~6MB) must be available

### Optional: GPU Support

For CUDA GPU acceleration:
```bash
cmake .. -DENABLE_ONNX_ML=ON
# Then set mlUseGPU=true in flameshot.ini
```

Requires NVIDIA GPU with CUDA support and ONNX Runtime built with CUDA provider.

---

## Updated Build Examples

### Linux with ML Support
```bash
# Install ONNX Runtime first (see docs/ML-QuickStart.md)
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_ONNX_ML=ON
make -j$(nproc)
```

### Windows with ML Support
```powershell
# Install ONNX Runtime first
mkdir build
cd build
cmake .. -DENABLE_ONNX_ML=ON -DONNXRUNTIME_ROOT=C:\onnxruntime
cmake --build . --config Release
```

### macOS with ML Support
```bash
# Install ONNX Runtime first
mkdir build && cd build
cmake .. -DENABLE_ONNX_ML=ON
cmake --build .
```

## Packaging Considerations

When distributing builds with `ENABLE_ONNX_ML=ON`:

1. Include ONNX Runtime libraries in package
2. Optionally bundle default YOLOv8n.onnx model
3. Document model download instructions for users
4. Note increased package size (~6-20MB for model)

## Troubleshooting Build Issues

### "ONNX Runtime not found"

```bash
# Set ONNXRUNTIME_ROOT explicitly
cmake .. -DENABLE_ONNX_ML=ON -DONNXRUNTIME_ROOT=/usr/local

# Or install to standard location
sudo cp -r /path/to/onnxruntime/include/* /usr/local/include/
sudo cp -r /path/to/onnxruntime/lib/* /usr/local/lib/
sudo ldconfig  # Linux only
```

### Linking Errors

Ensure `libonnxruntime.so` (or `.dll`/`.dylib`) is in library search path:

```bash
# Linux
export LD_LIBRARY_PATH=/path/to/onnxruntime/lib:$LD_LIBRARY_PATH

# macOS
export DYLD_LIBRARY_PATH=/path/to/onnxruntime/lib:$DYLD_LIBRARY_PATH

# Windows: Add to PATH environment variable
```
