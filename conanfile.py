from conan import ConanFile
import os
from conan.tools.files import copy
from pathlib import Path
import shutil

class Raven(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.test_requires(
            "cpr/1.14.1"
        )
