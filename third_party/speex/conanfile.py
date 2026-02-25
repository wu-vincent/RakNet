from conan import ConanFile
from conan.tools.files import get, copy, rm, rmdir
from conan.tools.gnu import PkgConfigDeps
from conan.tools.layout import basic_layout
from conan.tools.meson import Meson, MesonToolchain
import os


class SpeexConan(ConanFile):
    name = "speex"
    version = "1.2.1"
    description = "Speex is an Open Source/Free Software patent-free audio compression format designed for speech."
    license = "BSD-3-Clause"
    url = "https://github.com/xiph/speex"
    homepage = "https://www.speex.org/"
    topics = ("audio", "codec", "speech", "compression")

    package_type = "library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        self.settings.rm_safe("compiler.libcxx")
        self.settings.rm_safe("compiler.cppstd")

    def layout(self):
        basic_layout(self, src_folder="src")

    def build_requirements(self):
        self.tool_requires("meson/[>=1.2.3 <2]")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        deps = PkgConfigDeps(self)
        deps.generate()
        tc = MesonToolchain(self)
        tc.project_options["tools"] = "disabled"
        tc.project_options["test-binaries"] = "disabled"
        tc.generate()

    def build(self):
        meson = Meson(self)
        meson.configure()
        meson.build()

    def package(self):
        copy(self, "COPYING", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        meson = Meson(self)
        meson.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "share"))
        rm(self, "*.pdb", os.path.join(self.package_folder, "lib"))
        rm(self, "*.pdb", os.path.join(self.package_folder, "bin"))

    def package_info(self):
        self.cpp_info.libs = ["speex"]
        self.cpp_info.set_property("pkg_config_name", "speex")
        self.cpp_info.set_property("cmake_file_name", "Speex")
        self.cpp_info.set_property("cmake_target_name", "Speex::speex")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.append("m")
