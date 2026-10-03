from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class LugaruConan(ConanFile):
    name = "lugaru"
    license = "GPL-2.0-only"

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        "openal-soft/1.23.1",
        "libjpeg/9f",
        "libpng/1.6.50",
        "sdl/2.32.10",
        "vorbis/1.3.7",
        "jsoncpp/1.9.5",
        "glext/cci.20210420",
        "glu/system",
    )

    test_requires = "catch2/3.16.0"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        CMakeDeps(self).generate()
        CMakeToolchain(self).generate()
