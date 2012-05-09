#call "c:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\vsvars32.bat"
D:
cd \builds\yolk\submodules
mkdir llvm
cd llvm
cmake -G "NMake Makefiles JOM"  Z:\Projects\Cyberspace\yolk_repo\submodules\nacl-llvm\llvm
jom -j4
cd \builds
mkdir llvm
REM head llvm in \Projects\Cyberspace\llvm and head clang in llvm\tools\clang
cmake -G "NMake Makefiles JOM"  Z:\Projects\Cyberspace\llvm
