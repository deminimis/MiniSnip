import os

# ---------------- CONFIG ----------------
PROJECT_ROOT_NAME = "MiniSnip"  # Target subfolder
OUTPUT_FILENAME = "1allfiles.txt"
SKELETON_FILENAME = "project_skeleton.txt"

# Allowed extensions for a C++ Win32 project
ALLOWED_EXTENSIONS = {".cpp", ".h", ".rc", ".manifest"}

# Completely ignored folders (Build artifacts and IDE files)
IGNORE_FOLDERS = {
    "Release",
    "x64",
    "Debug",
    ".git",
    ".vs",
}

# Files never to include
HIDDEN_FILES = {
    OUTPUT_FILENAME,
    SKELETON_FILENAME,
    os.path.basename(__file__),
    ".DS_Store",
    "thumbs.db",
    "MiniSnip.aps",
    "MiniSnip.vcxproj",
    "MiniSnip.vcxproj.filters",
    "MiniSnip.vcxproj.user",
}
# ----------------------------------------

def extract_signatures(filepath, ext):
    """Reads a C++ file and extracts class and function declarations/definitions."""
    signatures = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            
        for line in lines:
            stripped = line.strip()
            
            # Match class definitions, methods, or major functions
            if ext in ['.h', '.cpp']:
                if (stripped.startswith('class ') or 
                    stripped.startswith('struct ') or
                    ( '(' in stripped and stripped.endswith(')') and not stripped.startswith('//') )):
                    # Grab a clean line representation
                    signatures.append(stripped.rstrip('{').strip())
                    
    except Exception:
        pass # Ignore unreadable files
        
    return signatures

def should_include_file(file_path):
    filename = os.path.basename(file_path)
    _, ext = os.path.splitext(file_path)

    if filename in HIDDEN_FILES:
        return False

    return ext.lower() in ALLOWED_EXTENSIONS

def generate_tree(root_path):
    out = []
    out.append(f"PROJECT STRUCTURE ({PROJECT_ROOT_NAME})")
    out.append("=" * 60)

    for current_root, dirs, files in os.walk(root_path):
        dirs[:] = [d for d in dirs if d not in IGNORE_FOLDERS]

        rel = os.path.relpath(current_root, root_path)
        level = 0 if rel == "." else rel.count(os.sep) + 1
        indent = "    " * level

        out.append(f"{indent}{os.path.basename(current_root)}/")

        for f in files:
            full_path = os.path.join(current_root, f)
            if should_include_file(full_path):
                out.append(f"{indent}    {f}")

    out.append("\n" + "=" * 60 + "\n")
    return "\n".join(out)

def pack_and_skeletonize():
    base_dir = os.getcwd()
    project_root = os.path.join(base_dir, PROJECT_ROOT_NAME)
    output_path = os.path.join(base_dir, OUTPUT_FILENAME)
    skeleton_path = os.path.join(base_dir, SKELETON_FILENAME)

    if not os.path.isdir(project_root):
        print(f"ERROR: '{PROJECT_ROOT_NAME}' folder not found in {base_dir}")
        return

    tree_str = generate_tree(project_root)
    file_count = 0
    total_signatures = 0

    with open(output_path, "w", encoding="utf-8") as out, \
         open(skeleton_path, "w", encoding="utf-8") as skel_out:
        
        # Write headers and tree structure
        out.write(tree_str)
        
        skel_out.write("=== MINISNIP - ARCHITECTURE SKELETON ===\n")
        skel_out.write("This file contains file paths and key C++ structural definitions.\n\n")
        skel_out.write(tree_str)
        skel_out.write("\n=== CLASS & FUNCTION SIGNATURES ===\n")

        for current_root, dirs, files in os.walk(project_root):
            dirs[:] = [d for d in dirs if d not in IGNORE_FOLDERS]

            for f in files:
                full_path = os.path.join(current_root, f)

                if should_include_file(full_path):
                    rel_path = os.path.relpath(full_path, project_root)
                    ext = os.path.splitext(f)[1].lower()

                    # --- 1. FULL FILE PACKING ---
                    try:
                        with open(full_path, "r", encoding="utf-8", errors='ignore') as infile:
                            out.write(f"FILE: {rel_path}\n")
                            out.write("-" * 60 + "\n")
                            out.write(infile.read())
                            out.write("\n\n" + "=" * 60 + "\n\n")
                            file_count += 1
                    except Exception as e:
                        print(f"Failed reading {rel_path}: {e}")
                    
                    # --- 2. SKELETON EXTRACTION ---
                    sigs = extract_signatures(full_path, ext)
                    if sigs:
                        skel_out.write(f"\n📂 {rel_path}\n")
                        for sig in sigs[:15]:  # Caps it so skeleton file doesn't explode with raw lines
                            skel_out.write(f"    - {sig}\n")
                        total_signatures += len(sigs)

    print(f"Lines packed successfully!")
    print(f"✅ SUCCESS: Packed {file_count} source files into {OUTPUT_FILENAME}")
    print(f"✅ SUCCESS: Extracted structural hints into {SKELETON_FILENAME}")

if __name__ == "__main__":
    pack_and_skeletonize()