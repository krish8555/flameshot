# ML-Based Smart Selection for Flameshot

## Overview

This feature adds intelligent area selection to Flameshot using machine learning object detection. When activated, it automatically detects objects, UI elements, or regions in your screenshot and allows you to select them with a single click.

## Features

- **Automatic Object Detection**: Uses pre-trained ML models (YOLOv8, YOLOv5) to detect objects in screenshots
- **Interactive Selection**: Click on any detected region to instantly select it
- **Visual Feedback**: Highlighted bounding boxes with confidence scores and labels
- **Keyboard Navigation**: Use Tab to cycle through detections, L to toggle labels, ESC to cancel
- **Optional GPU Acceleration**: Supports CUDA for faster inference on compatible hardware
- **Configurable**: Adjust confidence thresholds and model paths via configuration

## Prerequisites

### ONNX Runtime Installation

#### Linux (Ubuntu/Debian)
```bash
# Install ONNX Runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz
tar -xzf onnxruntime-linux-x64-1.16.3.tgz
sudo cp -r onnxruntime-linux-x64-1.16.3/include/* /usr/local/include/
sudo cp -r onnxruntime-linux-x64-1.16.3/lib/* /usr/local/lib/
sudo ldconfig
```

#### macOS (Homebrew)
```bash
# Install via Homebrew (if available) or download manually
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-osx-universal2-1.16.3.tgz
tar -xzf onnxruntime-osx-universal2-1.16.3.tgz
sudo cp -r onnxruntime-osx-universal2-1.16.3/include/* /usr/local/include/
sudo cp -r onnxruntime-osx-universal2-1.16.3/lib/* /usr/local/lib/
```

#### Windows
```powershell
# Download ONNX Runtime from GitHub releases
# Extract to C:\onnxruntime
# Add C:\onnxruntime\lib to PATH
```

### ML Model Download

Download a pre-trained YOLO model in ONNX format:

```bash
# Create models directory
mkdir -p ~/.flameshot/models/
cd ~/.flameshot/models/

# Download YOLOv8n (nano, fastest)
wget https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.onnx

# Or YOLOv8s (small, more accurate)
# wget https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8s.onnx

# Or YOLOv5s (alternative)
# wget https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5s.onnx
```

## Building Flameshot with ML Support

### CMake Configuration

```bash
cd flameshot
mkdir build && cd build

# Enable ML support
cmake .. -DENABLE_ONNX_ML=ON

# Optional: Specify ONNX Runtime location
cmake .. -DENABLE_ONNX_ML=ON -DONNXRUNTIME_ROOT=/path/to/onnxruntime

# Build
cmake --build .
```

### Build Options

- `ENABLE_ONNX_ML=ON`: Enable ML-based smart selection (default: OFF)
- `ONNXRUNTIME_ROOT`: Path to ONNX Runtime installation (auto-detected if installed system-wide)

## Usage

### Activating Smart Selection

1. Take a screenshot with Flameshot (`Ctrl+Print` or `flameshot gui`)
2. Press `Ctrl+S` to activate smart selection
3. Wait for object detection to complete (1-2 seconds)
4. Click on any highlighted region to select it
5. Use the selection as normal (copy, save, annotate, etc.)

### Keyboard Shortcuts

- `Ctrl+S`: Toggle smart selection on/off
- `Tab`: Cycle through detected objects
- `L`: Toggle labels visibility
- `ESC`: Cancel smart selection

### Configuration

Edit `~/.config/flameshot/flameshot.ini`:

```ini
[General]
# Path to ONNX model file
mlModelPath=/home/user/.flameshot/models/yolov8n.onnx

# Detection confidence threshold (0.0 to 1.0)
# Lower values detect more objects but with less confidence
mlConfidenceThreshold=0.3

# Enable GPU acceleration (requires CUDA)
mlUseGPU=false
```

## Supported Models

The implementation supports YOLO models in ONNX format:

- **YOLOv8n**: Fastest, suitable for real-time use (~6ms inference on GPU)
- **YOLOv8s**: Small model, good balance of speed and accuracy
- **YOLOv8m**: Medium model, better accuracy, slower
- **YOLOv5s/m/l**: Legacy YOLO v5 models (also supported)

### Model Comparison

| Model | Size | Speed (CPU) | Speed (GPU) | Best For |
|-------|------|-------------|-------------|----------|
| YOLOv8n | 6 MB | ~200ms | ~6ms | General use, fast response |
| YOLOv8s | 22 MB | ~400ms | ~10ms | Better accuracy |
| YOLOv8m | 50 MB | ~800ms | ~15ms | High accuracy required |

## Detected Object Classes

The default YOLO models can detect 80 COCO dataset classes:

- **People & Animals**: person, cat, dog, bird, horse, etc.
- **Vehicles**: car, bicycle, motorcycle, airplane, bus, etc.
- **Electronics**: laptop, mouse, keyboard, cell phone, tv, etc.
- **Furniture**: chair, couch, bed, table, etc.
- **Kitchen**: bottle, cup, fork, knife, bowl, etc.
- And 60+ more classes

## Performance Tips

1. **Use GPU Acceleration**: Set `mlUseGPU=true` if you have a CUDA-compatible GPU
2. **Choose the Right Model**: YOLOv8n is recommended for most users
3. **Adjust Confidence Threshold**: Lower values detect more objects, higher values are more selective
4. **Close Other Applications**: ML inference is CPU/GPU intensive

## Troubleshooting

### "ML model not found" Error

- Ensure the model file exists at the configured path
- Check that the file is a valid ONNX model
- Try downloading the model again

### "ONNX support not compiled" Error

- Rebuild Flameshot with `-DENABLE_ONNX_ML=ON`
- Ensure ONNX Runtime is properly installed
- Check CMake output for ONNX Runtime detection

### Slow Detection Performance

- Use YOLOv8n instead of larger models
- Enable GPU acceleration if available
- Close other resource-intensive applications
- Consider upgrading hardware (inference requires ~2GB RAM)

### No Objects Detected

- Lower the confidence threshold in settings
- Ensure the screenshot contains recognizable objects from COCO dataset
- Try a different model (some work better for specific types of objects)

## Architecture

### Components

1. **ObjectDetector** (`src/ml/objectdetector.h`): Abstract interface for ML backends
2. **OnnxDetector** (`src/ml/onnxdetector.h`): ONNX Runtime implementation
3. **SmartSelectionOverlay** (`src/widgets/capture/smartselectionoverlay.h`): UI overlay
4. **CaptureWidget Integration**: Keyboard shortcut and coordination logic

### Adding New Backends

To add support for TensorFlow Lite, CoreML, or other frameworks:

1. Create a new class inheriting from `ObjectDetector`
2. Implement required virtual methods
3. Add backend selection in `CaptureWidget::toggleSmartSelection()`

## Future Enhancements

Potential improvements for future versions:

- [ ] UI element detection (buttons, text fields, windows)
- [ ] Custom model training for screenshot-specific detection
- [ ] Region proposal refinement
- [ ] Multi-model ensemble detection
- [ ] Edge-based detection for non-rectangular selections
- [ ] Caching of detection results
- [ ] Background/foreground segmentation
- [ ] Text region detection with OCR integration

## Credits

- ONNX Runtime: https://onnxruntime.ai/
- YOLOv8: https://github.com/ultralytics/ultralytics
- COCO Dataset: https://cocodataset.org/

## License

This feature is part of Flameshot and is licensed under GPL-3.0-or-later.
