# Requires: conan 1.21.0 or higher
#   Note: Using an older version of conan yields a problem with
#         qt's bzip2 requirement
#   Reference: https://github.com/conan-io/conan-center-index/issues/533
#
# Building: 
#   $ conan install .. --build missing -s build_type=(Debug|Release)

from conans import ConanFile

class OwlConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = (
        ("openssl/1.1.1i", "override"),
        "qt/6.0.2",
        "boost/1.74.0",
        "libcurl/7.66.0",
        "spdlog/1.8.1",
        "libhunspell/1.7.0@owl/stable",
        "tidy-html5/5.7.28@owl/stable",
        "luadist/5.2.3@owl/stable",
        "rang/3.1@owl/stable"
    )

    generators = "cmake"

    default_options = {
        "qt:shared": True,
        "qt:with_sqlite3":False,
        "qt:with_mysql":False,
        "qt:with_pq":False,
        "qt:with_odbc":False,
        "qt:qt5compat":True,
        "qt:qtquickcontrols2":True,
        "qt:qtsvg":True,
        "boost:shared":False,
        "boost:without_test":False,
        "boost:without_filesystem":False,
        "boost:without_stacktrace":False,
        "boost:without_system":False,
        "boost:without_math":True,
        "boost:without_wave":True,
        "boost:without_container":True,
        "boost:without_contract":True,
        "boost:without_nowide":True,
        "boost:without_graph":True,
        "boost:without_iostreams":True,
        "boost:without_locale":True,
        "boost:without_log":True,
        "boost:without_program_options":True,
        "boost:without_random":True,
        "boost:without_regex":True,
        "boost:without_mpi":True,
        "boost:without_serialization":True,
        "boost:without_coroutine":True,
        "boost:without_fiber":True,
        "boost:without_context":True,
        "boost:without_timer":True,
        "boost:without_thread":True,
        "boost:without_chrono":True,
        "boost:without_date_time":True,
        "boost:without_atomic":True,
        "boost:without_graph_parallel":True,
        "boost:without_python":True,
        "boost:without_type_erasure":True
    }

    # def requirements(self):
    #     if self.settings.os == "Windows":
    #         self.requires("pdcurses/3.9@zethon/stable")
    #     else:
    #         self.requires("ncurses/6.1@conan/stable")