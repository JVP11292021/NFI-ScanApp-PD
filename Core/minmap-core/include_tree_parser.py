import os
import re

project_root = r"C:/repo/NFI-ScanApp-PD/Core/minmap-core"
output_file = r"include_tree.txt"
max_depth = 3  # Maximum recursion depth
include_pattern = re.compile(r'^\s*#include\s*["<](.+)[">]')

# Whitelist of main files to scan
whitelist = [
    "feature/pairing.h",
    "controllers/feature_matching.h",
    "controllers/image_reader.h",
    "controllers/feature_extraction.h",
    "controllers/option_manager.h",
    "util/file.h",
    "util/misc.h",
    "estimators/alignment.h",
    "scene/reconstruction.h",
    "estimators/similarity_transform.h",
    "sfm/observation_manager.h"
]

# Normalize whitelist for easier comparison
whitelist = [os.path.normpath(os.path.join(project_root, f)) for f in whitelist]

def resolve_include(file_path, include_name):
    """Resolve include to a file path in the project if possible"""
    # Relative to current file
    possible_path = os.path.join(os.path.dirname(file_path), include_name)
    if os.path.exists(possible_path):
        return os.path.normpath(possible_path)
    # Relative to project root
    possible_path = os.path.join(project_root, include_name)
    if os.path.exists(possible_path):
        return os.path.normpath(possible_path)
    return include_name  # likely system header

def simplify_path(file_path):
    """Return previous folder + file name for readability"""
    file_path = os.path.normpath(file_path)
    relative = file_path.replace(project_root, "").strip(os.sep)
    parts = relative.split(os.sep)
    if len(parts) >= 2:
        return f"{parts[-2]}/{parts[-1]}"
    return parts[-1]

def write_include_tree(file_path, f, visited=None, depth=0):
    """Recursively write include tree"""
    if visited is None:
        visited = set()

    indent = "  " * depth
    simplified_file = simplify_path(file_path)

    if file_path in visited:
        f.write(f"{indent}[CIRCULAR] {simplified_file}\n")
        return
    visited.add(file_path)

    if not os.path.exists(file_path):
        f.write(f"{indent}[MISSING] {simplified_file}\n")
        return

    if depth >= max_depth:
        return

    with open(file_path, "r", encoding="utf-8", errors="ignore") as file_content:
        for line in file_content:
            match = include_pattern.match(line)
            if match:
                inc = match.group(1)
                resolved = resolve_include(file_path, inc)
                simplified_resolved = simplify_path(resolved)

                f.write(f"{indent}{simplified_file} -> {simplified_resolved}\n")

                # Recurse for any file that exists in the project
                if os.path.isfile(resolved) and resolved.startswith(os.path.normpath(project_root)):
                    write_include_tree(resolved, f, visited.copy(), depth + 1)

# Main script: only process top-level files in the whitelist
with open(output_file, "w", encoding="utf-8") as f:
    for file in whitelist:
        if os.path.isfile(file):
            f.write(f"\n=== Include tree for: {simplify_path(file)} ===\n")
            write_include_tree(file, f)

print(f"Include tree written to: {output_file}")
