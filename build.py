import os
import shutil
import subprocess
import platform

CC = "g++"

OPTS = [
    "-std=c++11",
    "-O2",
    "-Wall",
    "-pthread",
]

HEADER_INCLUDES = [
    "-I", "Utilities",
]
TEST_HEADER_INCLUDES = [
    "-I", "lib/googletest/include",
]

TEST_STATIC_LIBRARIES = [
    "-L", "lib/googletest/lib",
    "-l", "gtest",
]

if platform.system() == "Windows" :
    SHARED_LIBRARIES = [
        "lib/googletest/lib/libgtest.dll",
    ]
elif platform.system() == "Linux":
    SHARED_LIBRARIES = [
        "lib/googletest/lib/libgtest.a",
    ]
else:
    SHARED_LIBRARIES = [] #some other platform

OUTPUT_DIR = "bin"
if not os.path.isdir(OUTPUT_DIR):
    os.mkdir(OUTPUT_DIR)

OUTPUT = os.path.join(OUTPUT_DIR, "Utilities.exe")
TEST_OUTPUT = os.path.join(OUTPUT_DIR, "UtilitiesTest.exe")

SRC = ["Utilities/Utilities.cpp"]
TEST_SRC = ["test/UtilitiesTest.cpp"]

args = [CC] + OPTS + HEADER_INCLUDES + SRC + ["-o", OUTPUT]
print("Calling", args)
subprocess.call(args)

args = [CC] + OPTS + TEST_HEADER_INCLUDES + HEADER_INCLUDES + TEST_SRC + ["-o", TEST_OUTPUT] + TEST_STATIC_LIBRARIES
print("Calling", args)
subprocess.call(args)

for s in SHARED_LIBRARIES:
    shutil.copyfile(s, os.path.join(OUTPUT_DIR, os.path.basename(s)))
