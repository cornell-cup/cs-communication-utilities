import os
import shutil
import subprocess

CC = "g++"

OPTS = [
  "-std=c++11",
  "-O2",
  "-Wall",
  "-pthread",
]

HEADER_INCLUDES = [
  "-I", "Utilities",
  "-I", "lib/googletest/include",
]

SHARED_LIBRARIES = [
  "lib/googletest/lib/libgtest.dll",
]

OUTPUT_DIR = "bin"
if not os.path.isdir(OUTPUT_DIR):
  os.mkdir(OUTPUT_DIR)

OUTPUT = os.path.join(OUTPUT_DIR, "Utilities.exe")

SRC = ["Utilities/Utilities.cpp"]

args = [CC] + OPTS + HEADER_INCLUDES + SRC + ["-o", OUTPUT]
print("Calling", args)
subprocess.call(args)

for s in SHARED_LIBRARIES:
  shutil.copyfile(s, os.path.join(OUTPUT_DIR, os.path.basename(s)))
