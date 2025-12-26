#!/bin/bash
set -e

# 取得 repo 根目錄
REPO_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$REPO_DIR"

# Arduino IDE / CLI 編譯輸出目錄
BUILD_DIR="build/esp32.esp32.esp32"

# 找 .ino.bin（這個才是 APP 映像檔）
BIN=$(ls -t "$BUILD_DIR"/*.ino.bin 2>/dev/null | head -n 1)

if [ -z "$BIN" ]; then
  echo "❌ 找不到 .ino.bin"
  echo "請先在 Arduino IDE/CLI 編譯專案（Export compiled binary 或等效動作）"
  exit 1
fi

echo "👉 找到最新韌體: $BIN"

echo "🔄 覆蓋 firmware.bin"
cp "$BIN" firmware.bin

# --- 自動版本號 +1（不想自動加就把這段砍掉） ---
if [ -f version.txt ]; then
  VER=$(cat version.txt)
  if [[ "$VER" =~ ^[0-9]+$ ]]; then
    NEW_VER=$((VER+1))
  else
    echo "⚠️ version.txt 格式怪怪的，重設為 1"
    NEW_VER=1
  fi
  echo "$NEW_VER" > version.txt
  echo "📈 自動版本提升: $VER → $NEW_VER"
else
  echo "1" > version.txt
  echo "🆕 建立 version.txt = 1"
fi
# -------------------------------------------------

git add firmware.bin version.txt
git commit -m "Update firmware: $(date '+%Y-%m-%d %H:%M:%S')"
git push

echo "🚀 Firmware pushed to GitHub 完成！"
