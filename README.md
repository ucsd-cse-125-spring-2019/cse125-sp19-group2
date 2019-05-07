# cse125-sp19-group2

### Setup and build
Before cloning, install the git LFS client and run:
```
git lfs install
```
Then clone recursively:
```
git clone --recursive https://github.com/ucsd-cse-125-spring-2019/cse125-sp19-group2.git
```
If already cloned, be sure to download nanoGUI dependencies via submodules:
```
git submodule update --init --recursive
```

Open ProjectBone.sln with Visual Studio 2017 or later. Only 64-bit builds are supported at the moment.
