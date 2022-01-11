set_policy("check.auto_ignore_flags", false)

add_cxxflags("-g")
add_cxxflags("-std=c++14")
add_cxxflags("-Wno-write-strings")
add_cxxflags("-finput-charset=UTF-8")
add_cxxflags("-Wall")
add_cxxflags("-Wextra")

target("test")
    set_kind("binary")
    set_targetdir("./")

    add_includedirs("../include")

    add_includedirs("src")
    add_files("./main.cpp")
target_end()

