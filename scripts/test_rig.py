"""Run the hardware-independent animation tests with an installed C++ compiler."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = os.environ.get("CXX") or shutil.which("g++")
if not compiler:
    raise SystemExit("Install a C++ compiler or set CXX to its path")
with tempfile.TemporaryDirectory(prefix="rig-tests-") as temp:
    binary = Path(temp) / ("rig-test.exe" if os.name == "nt" else "rig-test")
    subprocess.run([
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-I" + str(root / "test/native/fakes"),
        "-I" + str(root / "lib/RigLighting/src"),
        str(root / "test/native/rig_test.cpp"),
        str(root / "lib/RigLighting/src/RigController.cpp"),
        str(root / "lib/RigLighting/src/RigLighting.cpp"),
        str(root / "lib/RigLighting/src/PartyLighting.cpp"),
        "-o", str(binary),
    ], check=True)
    # Windows compiler DLLs live beside g++; make them available to the test process.
    environment = dict(os.environ)
    environment["PATH"] = str(Path(compiler).resolve().parent) + os.pathsep + environment.get("PATH", "")
    subprocess.run([str(binary)], env=environment, check=True)
