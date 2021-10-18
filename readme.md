# Process YUMA almanac
Utility to load and compute satellite position from YUMA almanac. Current YUMA almanac files can be downloaded from [www.navcen.uscg.gov](https://www.navcen.uscg.gov/?pageName=gpsAlmanacs). Utility uses handling of YUMA files using [GPSTk library](https://gitlab.com/sgl-ut/GPSTk). Code was compiled and tested under Linux.


## Build `ProcessYUMA` executable
This build require installed GPSTk library.
```bash
# Make sure you are in the root of ProcessYUMA repository
$ mkdir build && cd build
$ cmake ..
$ make
```
After build the `ProcessYUMA` can be run as following (executable is in `build` directory):
```bash
$ ./ProcessYUMA almanac PRN tStart tStop tInterval

Input parameters
    almanac  - path to YUMA almanac file
    SVN      - satellite number
    tStart   - GPS start time in format yyyymmdd_HHMMSS (e.g. 20210101_000000)
    tStop    - GPS end time in format yyyymmdd_HHMMSS
    Interval - interval of processing in seconds (integer)

Example:
$ ./ProcessYUMA ../example/almanacMultiple.alm 2 20211017_000000 20211017_000005 1
PRN;year;month;day;hour;minute;second;TOA;tDiff;x;y;z
2;2021;10;17;0;0;0.000000;10;10.000000;-7860393.641589;-14935078.271048;-19877604.623357
2;2021;10;17;0;0;1.000000;10;9.000000;-7857734.370616;-14934839.471722;-19878788.851615
2;2021;10;17;0;0;2.000000;10;8.000000;-7855074.929965;-14934600.803968;-19879972.632981
2;2021;10;17;0;0;3.000000;10;7.000000;-7852415.319700;-14934362.267815;-19881155.967426
```



## Build of GPSTk library

1. Install prerequisities (tested at Ubuntu 20.04 LTS)
    ```bash
    $ sudo apt install build-essential make cmake libgtest-dev libgmock-dev doxygen graphviz
    ```
2. Clone repository and prepare `build` directory
    ```bash
    $ git clone https://gitlab.com/sgl-ut/GPSTk.git
    $ cd GPSTk && mkdir build && cd build
    ```
3. Compilation and installation (`BUILD_EXT=ON` means we want to build also GPSTk library components present in `ext` directory, `TEST_SWITCH=ON` means we want to compile also tests)
    ```bash
    $ cmake -DBUILD_EXT=ON -DTEST_SWITCH=ON .. 
    $ make all -j 4 # Running of 4 cores
    $ make install
    ```
    After `make install` GPSTk library will be installed under `usr/local/lib` and include files are present in `usr/local/include/gpstk`

4. Running CMake tests
    ```bash
    $ ctest -v -j 4 # Running on 4 cores
    ```