add_executable(appanubis main.cpp)
target_compile_features(appanubis PRIVATE cxx_std_17)

target_link_libraries(appanubis PRIVATE libanubis)
