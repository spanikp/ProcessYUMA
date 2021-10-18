#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cstdio>

#include "YumaData.hpp"
#include "YumaStream.hpp"
#include "YumaBase.hpp"
#include "GPSWeekZcount.hpp"
#include "TimeCorrection.hpp"
#include "SystemTime.hpp"
#include "GPSWeekSecond.hpp"


void print_help() { std::string s = R"(
Compute satellite position from YUMA almanac
$ ./ProcessYUMA almanac PRN tStart tStop tInterval

Input parameters
    almanac  - path to YUMA almanac file
    SVN      - satellite number
    tStart   - GPS start time in format yyyymmdd_HHMMSS (e.g. 20210101_000000)
    tStop    - GPS end time in format yyyymmdd_HHMMSS
    Interval - interval of processing in seconds (integer)

Example usage:
$ ./ProcessYUMA almanac.alm 1 20210101_000000 20210101_000100 5)";
    std::cout << s << std::endl;
}

gpstk::CivilTime str_to_civiltime(const std::string& s, const gpstk::TimeSystem time_system) {
    int year, month, day, hour, minute, second;
    year = std::stoi(s.substr(0, 4));
    month = std::stoi(s.substr(4, 2));
    day = std::stoi(s.substr(6, 2));
    hour = std::stoi(s.substr(9, 2));
    minute = std::stoi(s.substr(11,2));
    second = std::stoi(s.substr(13,2));

    return gpstk::CivilTime(year,month,day,hour,minute,second,time_system);
}

double find_closest_time(const std::vector<gpstk::CommonTime>& alm_t, const gpstk::CommonTime& t, int& idx) {
    std::vector<double> tDiff(alm_t.size());
    for (int i = 0; i < alm_t.size(); ++i) {
        tDiff.at(i) = std::abs(alm_t.at(i) - t);
        //std::cout << tDiff.at(i) << std::endl;
    }

    // Find minimum element
    std::vector<double>::iterator min_elem_iterator = std::min_element(tDiff.begin(),tDiff.end());
    idx = std::distance(tDiff.begin(), min_elem_iterator);

    return tDiff.at(idx);
}

int main( int argc, char* argv[] )
{
    // Parsing input arguments
    if ( argc != 6 ) {
        std::cout << "Incorrect number of arguments!" << std::endl;
        print_help();
        return EXIT_FAILURE;
    }

    const auto almanac_file = std::filesystem::canonical(argv[1]);
    const long PRN = std::stol(std::string(argv[2]));
    const auto tStart = str_to_civiltime(std::string(argv[3]), gpstk::TimeSystem::GPS);
    const auto tStop = str_to_civiltime(std::string(argv[4]), gpstk::TimeSystem::GPS);
    const auto dt = std::stoi(std::string(argv[5]));

    // std::cout << "Parsed arguments:"
    //           << " almanac=" << almanac_file.filename()
    //           << " PRN=" << PRN
    //           << " tStart=" << gpstk::printTime(tStart,"%Y/%m/%d %H:%M:%S")
    //           << " tStop=" << gpstk::printTime(tStop,"%Y/%m/%d %H:%M:%S")
    //           << " tInterval=" << dt << std::endl;

    // Check parsed arguments
    if (tStart > tStop) {
        printf("Start time is after end time, program will exit!\n");
        return EXIT_FAILURE;
    }
    if (PRN < 1) {
        printf("Invalid PRN value (PRN=%l)!\n",PRN);
        return EXIT_FAILURE;
    }
    if (dt < 1) {
        printf("Invalid time interval value (tInterval=%d)! Allowed values tInterval >= 1.",dt);
        return EXIT_FAILURE;
    }

    // Convert given timestamps to CommonTime type
    gpstk::CommonTime t1 = tStart.convertToCommonTime();
    gpstk::CommonTime t2 = tStop.convertToCommonTime();
    long gpsWeek_t1 = gpstk::GPSWeekSecond(t1).getWeek();

    // Load YUMA almanac
    gpstk::YumaStream yuma_stream(almanac_file.c_str());
    gpstk::YumaData yuma_data;

    // Iterate through almanacs from stream
    gpstk::AlmOrbit almOrbit;
    std::map<int, std::vector<gpstk::AlmOrbit>> almOrbits;
    while (yuma_stream >> yuma_data)
    {
        // Check for GPS rollovers and correct it with timeAdjustWeekRollover function
        yuma_data.week = gpstk::timeAdjustWeekRollover(yuma_data.week,gpsWeek_t1);

        // Add almanac to almanac map according satellite PRN
        // Map almOrbits can handle multiple almanacs per satellite
        almOrbit = gpstk::AlmOrbit(yuma_data);
        almOrbits[almOrbit.getPRNID()].push_back(almOrbit);
    }
    yuma_stream.close();

    // Check if required satellite is present in the almanac file
    if (almOrbits.count(PRN) == 1) {
        std::vector<gpstk::CommonTime> alm_toa;
        for (auto& alm : almOrbits[PRN]) {
            alm_toa.push_back(alm.getToaTime());
        }

        // Helper variables (avoid initialization of variables in time loop)
        gpstk::Triple pos; // Computed coordinates
        int alm_idx; // Index to track closest almanac
        double tDiff;
        gpstk::CivilTime ct;

        // Create output file and write header
        //FILE *fout = fopen("output.csv", "w");
        printf("PRN;year;month;day;hour;minute;second;TOA;tDiff;x;y;z\n");

        // Looping all required times
        gpstk::CommonTime t = tStart.convertToCommonTime();
        while (t <= t2 ) {
            tDiff = find_closest_time(alm_toa,t,alm_idx);
            pos = almOrbits[PRN].at(alm_idx).svXvt(t).getPos();
            ct = gpstk::CivilTime(t);
            printf("%d;%d;%d;%d;%d;%d;%f;%d;%f;%f;%f;%f\n",PRN,ct.year,ct.month,ct.day,ct.hour,ct.minute,ct.second,
                almOrbits[PRN].at(alm_idx).getToaSOW(),tDiff,pos[0],pos[1],pos[2]);
            
            // Increment time to compute position by given tInterval value
            t.addSeconds((long)dt);
        }

        // Close output file
        //fclose(fout);

    } else {
        printf("No almanac found for PRN %02d!\n",(int)PRN);
        return EXIT_FAILURE;
    }    
}
