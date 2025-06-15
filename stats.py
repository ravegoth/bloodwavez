import os
from pathlib import Path

def count_lines(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            return sum(1 for _ in f)
    except Exception:
        return 0

def get_size(file_path):
    return os.path.getsize(file_path)

source_exts = ['.cpp', '.h']
image_exts = ['.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tiff', '.webp']
audio_exts = ['.wav', '.mp3', '.ogg', '.flac', '.aac']
binary_exts = ['.bin', '.dll', '.exe']

src_dir = Path('./src')
res_dir = Path('./res')
bin_dir = Path('./bin')

cpp_h_count = 0
cpp_h_lines = 0
cpp_h_size_bytes = 0
image_count = 0
audio_count = 0
binary_count = 0

for path in src_dir.rglob('*'):
    if path.suffix in source_exts:
        cpp_h_count += 1
        cpp_h_lines += count_lines(path)
        cpp_h_size_bytes += get_size(path)

for path in res_dir.rglob('*'):
    if path.suffix.lower() in image_exts:
        image_count += 1
    elif path.suffix.lower() in audio_exts:
        audio_count += 1

for path in bin_dir.rglob('*'):
    if path.suffix.lower() in binary_exts:
        binary_count += 1

cpp_h_size_kb = cpp_h_size_bytes / 1024
cpp_h_size_mb = cpp_h_size_kb / 1024

print(f"number of .cpp/.h files in ./src: {cpp_h_count}")
print(f"total lines of code in ./src: {cpp_h_lines}")
print(f"total size: {cpp_h_size_kb:.2f} KB / {cpp_h_size_mb:.2f} MB")
print(f"number of image files in ./res: {image_count}")
print(f"number of audio files in ./res: {audio_count}")
print(f"number of binary files in ./bin: {binary_count}")
