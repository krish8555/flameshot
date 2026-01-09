# Quick Start: ML Smart Selection

## What This Feature Does

Automatically detects objects in your screenshots so you can select them with one click instead of manually drawing selection boxes.

## Setup (5 minutes)

### 1. Install ONNX Runtime

**Linux:**
```bash
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz
tar -xzf onnxruntime-linux-x64-1.16.3.tgz
sudo cp -r onnxruntime-linux-x64-1.16.3/include/* /usr/local/include/
sudo cp -r onnxruntime-linux-x64-1.16.3/lib/* /usr/local/lib/
sudo ldconfig
```

**Windows:**
Download from https://github.com/microsoft/onnxruntime/releases and extract to `C:\onnxruntime`, then add to PATH.

### 2. Download ML Model

```bash
mkdir -p ~/.flameshot/models/
cd ~/.flameshot/models/
wget https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.onnx
```

### 3. Build Flameshot

```bash
cd flameshot
mkdir build && cd build
cmake .. -DENABLE_ONNX_ML=ON
cmake --build .
```

### 4. Use It!

1. Press `Ctrl+Print` (or run `flameshot gui`)
2. Press `Ctrl+S` to activate smart selection
3. Click on any highlighted region to select it instantly!

## Keyboard Shortcuts

- `Ctrl+S`: Toggle smart selection
- `Tab`: Cycle through detected objects
- `L`: Toggle labels
- `ESC`: Cancel

## Configuration

Edit `~/.config/flameshot/flameshot.ini`:

```ini
[General]
mlModelPath=/home/user/.flameshot/models/yolov8n.onnx
mlConfidenceThreshold=0.3
mlUseGPU=false
```

## What It Detects

The model can detect 80 types of objects including:
- People, animals, vehicles
- Electronics (laptop, phone, keyboard, mouse)
- Furniture (chair, table, couch)
- Kitchen items (cup, bottle, utensils)
- And much more!

---

For detailed documentation, see [ML-SmartSelection.md](ML-SmartSelection.md)
