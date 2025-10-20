## General build guide

<h3>Dependencies</h3>

[CMake](https://cmake.org/download/)
[Make](https://www.gnu.org/software/make/#download)
A C/C++ compiler

<h3>For Linux</h3>

```bash
$ cd {folderName}
$ cmake . -B build
$ cd build && make
```

<h3>For Windows</h3>

```powershell
$ cd {folderName}
$ cmake . -B build "MinGW Makefiles"
$ cd build && mingw32-make
```