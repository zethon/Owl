
cmake --build . --config %configuration% --target install
cpack -G NSIS64 -C %configuration% --config Owl.CPackConfig.cmake
cpack -G NSIS64 -C %configuration% --config OwlConsole.CPackConfig.cmake