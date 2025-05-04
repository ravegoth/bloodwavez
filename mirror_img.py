import os
from PIL import Image

def mirror_image_tool() -> None:
    """
    Mirrors all PNG images in ./res/ that match a given prefix,
    flips them horizontally, and saves them with the '_mirror' suffix.
    """
    res_dir = "./res"
    try:
        files = os.listdir(res_dir)
    except FileNotFoundError:
        print(f"Error: Directory '{res_dir}' does not exist.")
        return
    except PermissionError as e:
        print(f"Access error for '{res_dir}': {e}")
        return

    # Filter PNG files
    png_files = sorted(
        f for f in files
        if f.lower().endswith(".png") and not f.lower().endswith("_mirror.png")
    )
    if not png_files:
        print(f"No PNG files found in '{res_dir}'.")
        return

    print("Image Mirroring Tool")
    print("Enter a prefix to select files (e.g., 'enemy_'):")
    prefix = input("Prefix: ").strip()

    # Filter files by prefix
    matching_files = [f for f in png_files if f.startswith(prefix)]
    if not matching_files:
        print(f"No files found with prefix '{prefix}'.")
        return

    print(f"Found {len(matching_files)} matching file(s):")
    for name in matching_files:
        print(f"- {name}")
    print()

    # Process and save mirrored images
    for selected in matching_files:
        source = os.path.join(res_dir, selected)
        name, ext = os.path.splitext(selected)
        dest = os.path.join(res_dir, f"{name}_mirror{ext}")
        try:
            with Image.open(source) as img:
                mirrored = img.transpose(Image.FLIP_LEFT_RIGHT)
                mirrored.save(dest)
                print(f"* Created {dest} *")
        except FileNotFoundError:
            print(f"Error: File '{source}' not found.")
        except Exception as e:
            print(f"Error processing image '{selected}': {e}")

    print("Done!")

if __name__ == "__main__":
    mirror_image_tool()
