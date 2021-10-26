# Process YUMA almanac
Utility to load and compute satellite position from YUMA almanac for specified satellite and time range. Current YUMA almanac files can be downloaded from [www.navcen.uscg.gov](https://www.navcen.uscg.gov/?pageName=gpsAlmanacs). Utility use YUMA almanac load and position computation using [GPSTk library](https://gitlab.com/sgl-ut/GPSTk). Code was compiled and tested under Linux.

## Build `ProcessYUMA` executable
This build require installed GPSTk library, see [Build of GPSTk library](#Build-of-GPSTk-library) section below. 
```bash
# Make sure you are in the root of ProcessYUMA repository
$ mkdir build && cd build
$ cmake ..
$ make
```
After build the `ProcessYUMA` can be run as following (executable is in `build` directory):
```
$ ./ProcessYUMA almanac PRN tStart tStop tInterval

Input parameters
    almanac  - path to YUMA almanac file
    SVN      - satellite number
    tStart   - GPS start time in format YYYY-mm-ddTHH:MM:SS (e.g. 20210101_000000)
    tStop    - GPS end time in format YYYY-mm-ddTHH:MM:SS
    Interval - interval of processing in seconds (integer)

Example:
$ ./ProcessYUMA ../example/almanacMultiple.alm 2 2021-10-17T00:00:00 2021-10-17T00:00:03 1
PRN;year;month;day;hour;minute;second;TOA;tDiff;x;y;z
2;2021;10;17;0;0;0.000000;10;10.000000;-7860393.641589;-14935078.271048;-19877604.623357
2;2021;10;17;0;0;1.000000;10;9.000000;-7857734.370616;-14934839.471722;-19878788.851615
2;2021;10;17;0;0;2.000000;10;8.000000;-7855074.929965;-14934600.803968;-19879972.632981
2;2021;10;17;0;0;3.000000;10;7.000000;-7852415.319700;-14934362.267815;-19881155.967426
```
## Conversions to YUMA almanac format

Python helper script [convertToYUMA.py](convertToYUMA.py) can be used to convert Galileo XML almanac file to YUMA format. Galileo almanacs can be downloaded from [European GNSS Service Centre](https://www.gsc-europa.eu/product-almanacs) website. Also support of conversion of Keplerian elements is supported. Script [convertToYUMA.py](convertToYUMA.py) will detect the type of input file based on the file extension (`.xml` for Galileo almanac, `.csv` for Keplerian elements). Generated YUMA almanac (files with `.alm` extension) can be used by `ProcessYUMA` executable.

```bash
# Create processYUMA Python virtual environment
$ python3 -m venv processYUMA
$ source processYUMA/bin/activate
(processYUMA) $ pip install -r requirements.txt

# Example of conversions to YUMA almanac format
(processYUMA) $ ./convertToYUMA.py example/2020-07-03.xml -o 2020-07-03.alm
(processYUMA) $ ./convertToYUMA.py example/kepler-input.csv -o kepler-input.alm
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
    $ make all -j 4 # Running on 4 cores
    $ make install
    ```
    After `make install` GPSTk library will be installed under `usr/local/lib` and include files are present in `usr/local/include/gpstk`

4. Running CMake tests
    ```bash
    $ ctest -v -j 4 # Running on 4 cores
    ```
