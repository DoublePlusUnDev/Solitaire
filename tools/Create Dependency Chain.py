#can be used to display dependencies: https://csacademy.com/app/graph_editor/

import os
from pathlib import Path


output_file = "_graph.txt"

root_path = (Path(__file__).parent / "..").resolve()
print(root_path)
folders_to_search = [".", "include", "src"]
folders = [str(root_path / folder) for folder in folders_to_search]
ignore = ["memtrace", "gtest_lite"]

c_length = 0
h_length = 0
inl_length = 0

nodes = []
connections = []

for folder in folders:
    folder_path = Path(folder)

    if not folder_path.exists():
        continue

    for file_path in folder_path.glob("*.h"):
        name = file_path.stem.lower()
        if name in ignore:
            continue

        nodes.append(name)
        print(name)

for folder in folders:
    folder_path = Path(folder)
    for file_path in folder_path.glob("*.*"):
        if not file_path.suffix.lower() in [".c", ".cpp", ".h", ".inl"]:
            continue
        
        name = file_path.stem.lower()
        if name in ignore:
            continue

        if file_path.suffix.lower() in [".c", ".cpp", "h"]:
            nodes.append(name)


        with open(file_path, "r", encoding="utf-8") as file:
            lines = file.readlines()

            if file_path.suffix.lower() in [".c", ".cpp"]:
                c_length+=len(lines)
            elif file_path.suffix.lower() == ".h":
                h_length+=len(lines)
            elif file_path.suffix.lower() == ".inl":
                inl_length+=len(lines)
            
            for line in lines:
                if ("include" in line and "\"" in line):
                    dependency = (line.split("\"")[1].replace(".h", "")).lower()
                    if dependency != name:
                        connections.append([dependency, name])

with open(output_file, "w", encoding="utf-8") as graphfile:
    for node in nodes:
        graphfile.write(node + "\n")

    for connection in connections:
        graphfile.write(connection[0] + " " + connection[1] + "\n")

print(f"Header length: {h_length}")
print(f"Source length: {c_length}")
print(f"Template length: {inl_length}") 
print(f"Combined: {h_length + c_length + inl_length}")
input()
