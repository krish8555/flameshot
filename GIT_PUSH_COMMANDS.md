# Git Commands to Push Your Changes

## Step 1: Check Current Status

```bash
git status
```

## Step 2: Add All Your Changes

```bash
# Add all new and modified files
git add .

# Or add specific directories
git add src/ml/
git add src/widgets/capture/smartselectionoverlay.*
git add docs/ML-*.md
git add CMakeLists.txt
git add src/CMakeLists.txt
git add src/widgets/capture/CMakeLists.txt
git add src/utils/confighandler.h
git add src/widgets/capture/capturewidget.h
git add src/widgets/capture/capturewidget.cpp
git add README.md
```

## Step 3: Commit Your Changes

```bash
git commit -m "feat: Add ML-based smart selection using YOLO object detection

- Implemented ObjectDetector abstract interface and OnnxDetector
- Added SmartSelectionOverlay for interactive region selection
- Integrated with CaptureWidget (Ctrl+S shortcut)
- Added ENABLE_ONNX_ML CMake option
- Supports 80+ COCO object classes with YOLO models
- Optional GPU acceleration via CUDA
- Complete documentation in docs/ML-*.md files"
```

## Step 4: Push to Your Private Repo

### If this is your first push to this branch:

```bash
# Push to your private repo (assuming 'origin' is your repo)
git push origin master

# Or if you're on a different branch:
git push origin <your-branch-name>
```

### If you need to set up the remote:

```bash
# Check current remote
git remote -v

# If you need to add your private repo as remote:
git remote add origin https://github.com/your-username/flameshot.git

# Or if using SSH:
git remote add origin git@github.com:your-username/flameshot.git

# Then push
git push -u origin master
```

## Step 5: Verify the Push

```bash
# Check that everything was pushed
git log --oneline -5

# Visit your GitHub repo to confirm the changes are there
```

## Alternative: Push to a New Branch

```bash
# Create and switch to a new branch
git checkout -b ml-smart-selection

# Add and commit as above, then push
git push -u origin ml-smart-selection
```

## Quick One-Liner (if you're already set up)

```bash
git add . && git commit -m "feat: Add ML-based smart selection" && git push origin master
```

---

## Troubleshooting

### If you get "failed to push some refs":

```bash
# Pull first, then push
git pull origin master --rebase
git push origin master
```

### If you need to force push (use carefully):

```bash
git push origin master --force
```

### To see what will be pushed:

```bash
git diff origin/master..HEAD
```
