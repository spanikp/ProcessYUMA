#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <cstdio>
#include <exception>

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

Input parameters:
    almanac   - path to YUMA almanac file
    PRN       - satellite number selection, can be one of:
              ':' - will compute for all satellites from almanac file
              'prnStart:prnEnd' - will compute for all sats in given range
              'prn1,prn2,prn3' - will compute for all listed satellites
    tStart    - GPS start time in format YYYY-mm-ddTHH:MM:SS (e.g. 2021-01-01T00:00:00)
    tStop     - GPS end time in format YYYY-mm-ddTHH:MM:SS
    tInterval - interval of processing in seconds (integer)

Example usage (all satellites, satellite range, individual satellite selection):
$ ./ProcessYUMA data/current.alm :     2021-01-01T00:00:00 2021-01-01T00:01:00 5
$ ./ProcessYUMA data/current.alm 1:5   2021-01-01T00:00:00 2021-01-01T00:01:00 5
$ ./ProcessYUMA data/current.alm 1,3,5 2021-01-01T00:00:00 2021-01-01T00:01:00 5
)";
    std::cout << s << std::endl;
}

void validate_timestamp(const std::string& s) {
    auto re = std::regex("([0-9]){4}-(1[0-2]|0[1-9])-(3[01]|0[1-9]|[12][0-9])T(2[0-3]|[01][0-9]):([0-5][0-9]):([0-5][0-9])");
    if (!std::regex_match(s,re))
        throw std::invalid_argument("Failed to parse tStart/tStop values! Required format is 'YYYY-mm-ddTHH:MM:SS'.");
}

std::vector<std::string> string_split(const std::string s, const char delimiter) {
    std::vector<std::string> s_split;
    std::stringstream ss(s);
    while (ss.good()) {
        std::string substr;
        getline(ss,substr,delimiter);
        s_split.push_back(substr);
    }

    return s_split;
}

std::vector<long> str_to_prns(const std::string &s) {

    std::vector<long> PRNs;

    // Validate input string
    auto re = std::regex("(:)|([0-9]+)|([0-9]+:[0-9]+)|([0-9]+(,[0-9]+)+)");

    // Proceed if regex is matched, otherwise raise error
    if (std::regex_match(s,re)) {
        if (s == ":") {
            // Use all satellites from YUMA almanac
            PRNs.push_back(-1);
        } 
        else if (s.find(":") != std::string::npos) {
            // Range of satellites
            std::vector<long> PRNs_range;
            auto PRNs_range_str = string_split(s,':');
            for (auto x : PRNs_range_str)
                PRNs_range.push_back(std::stol(x));

            // Handling exceptions
            auto invalid_range_error = std::invalid_argument("Incorrect satellite range definition. Required format: 's1:s2' (condition s2 > s1 has to be met)");;
            if (PRNs_range.size() != 2) {
                throw invalid_range_error;
            } else {
                if (!(PRNs_range[0] < PRNs_range[1]))
                    throw invalid_range_error;

                // Transform to satellite range
                for (long prn = PRNs_range[0]; prn <= PRNs_range[1]; prn++)
                    PRNs.push_back(prn);
            }
        }
        else {
            // Individual satellite selection (output unique satellite IDs)
            auto PRNs_str = string_split(s,',');
            long prn;
            for (auto x : PRNs_str) {
                prn = std::stol(x);
                if (std::find(PRNs.begin(),PRNs.end(),prn) == PRNs.end())
                    PRNs.push_back(prn);
            }
        }
    } else {
        throw std::invalid_argument("Failed to parse PRN values! Following inputs are allowed: ':' (all sats), '1:5' (range of sats), '1,3,5' (individual sats)");
    }

    return PRNs;
}

gpstk::CivilTime str_to_civiltime(const std::string& s, const gpstk::TimeSystem time_system) {
    int year, month, day, hour, minute, second;
    validate_timestamp(s);
    year   = std::stoi(s.substr(0, 4));
    month  = std::stoi(s.substr(5, 2));
    day    = std::stoi(s.substr(8, 2));
    hour   = std::stoi(s.substr(11, 2));
    minute = std::stoi(s.substr(14,2));
    second = std::stoi(s.substr(17,2));

    return gpstk::CivilTime(year,month,day,hour,minute,second,time_system);
}

void adjust_PRNs(std::vector<long>& prns, const std::map<long, std::vector<gpstk::AlmOrbit>>& almOrbits) {
    // List all PRNs from almOrbits
    std::vector<long> almOrbit_prns;
    for (const auto& almOrbit: almOrbits)
        almOrbit_prns.push_back(almOrbit.first);

    // Handling case of ':' -> all satellites present in YUMA file
    if ((prns.size() == 1) && (prns[0] == -1)) {
        prns = almOrbit_prns;
        return;
    }

    // Checking if all PRNs are available in YUMA file, if not raise error
    for (auto prn : prns) {
        if (std::find(almOrbit_prns.begin(), almOrbit_prns.end(), prn) == almOrbit_prns.end()) {
            std::string error_str = "Satellite with PRN=" + std::to_string(prn) + " is not present in input file!";
            throw std::invalid_argument(error_str);
        }
    }
}

double find_closest_time(const std::vector<gpstk::CommonTime>& alm_t, const gpstk::CommonTime& t, int& idx) {
    std::vector<double> tDiff(alm_t.size()), tDiff_abs(alm_t.size());
    for (int i = 0; i < alm_t.size(); ++i) {
        tDiff.at(i) = t - alm_t.at(i);
        tDiff_abs.at(i) = std::abs(tDiff.at(i));
    }

    // Find minimum element
    std::vector<double>::iterator min_elem_iterator = std::min_element(tDiff_abs.begin(),tDiff_abs.end());
    idx = std::distance(tDiff_abs.begin(), min_elem_iterator);

    return tDiff.at(idx);
}

std::map<long, std::vector<gpstk::CommonTime>> get_toa_from_almOrbits(const std::map<long, std::vector<gpstk::AlmOrbit>>& almOrbits) {
    std::map<long, std::vector<gpstk::CommonTime>> alm_toa_map;
    for (const auto& alm_vector : almOrbits) {
        for (const auto& alm : alm_vector.second)
            alm_toa_map[alm_vector.first].push_back(alm.getToaTime());
    }

    return alm_toa_map;
}

int main( int argc, char* argv[] ) {

    // Parsing input arguments
    if ( argc != 6 ) {
        std::cout << "Incorrect number of arguments!" << std::endl;
        print_help();
        return EXIT_FAILURE;
    }

    // Initialize input variables
    std::filesystem::path almanac_file;
    std::vector<long> PRNs;
    gpstk::CivilTime tStart, tStop;
    int dt;
    try {
        almanac_file = std::filesystem::canonical(argv[1]);
        PRNs = str_to_prns(std::string(argv[2]));
        tStart = str_to_civiltime(std::string(argv[3]), gpstk::TimeSystem::GPS);
        tStop = str_to_civiltime(std::string(argv[4]), gpstk::TimeSystem::GPS);
        dt = std::stoi(std::string(argv[5]));
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        print_help();
        return EXIT_FAILURE;
    }

    // Check parsed arguments
    if (tStart > tStop) {
        printf("Start time is after end time, program will exit!\n");
        print_help();
        return EXIT_FAILURE;
    }
    if (dt < 1) {
        printf("Invalid time interval value (tInterval=%d)! Allowed value is tInterval >= 1 (seconds)!\n",dt);
        print_help();
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
    std::map<long, std::vector<gpstk::AlmOrbit>> almOrbits;
    while (yuma_stream >> yuma_data)
    {
        // Check for GPS rollovers and correct it with timeAdjustWeekRollover function
        yuma_data.week = gpstk::timeAdjustWeekRollover(yuma_data.week,gpsWeek_t1);

        // Add almanac to almanac map according satellite PRN
        // Map almOrbits can handle multiple almanacs per satellite
        almOrbit = gpstk::AlmOrbit(yuma_data);
        almOrbits[(long)almOrbit.getPRNID()].push_back(almOrbit);
    }
    yuma_stream.close();

    // Check satellite PRNs (adapt range if needed)
    adjust_PRNs(PRNs,almOrbits);
    auto alm_toa_map = get_toa_from_almOrbits(almOrbits);
    
    // Helper variables (avoid initialization of variables in time loop)
    gpstk::Triple pos; // Computed coordinates
    int alm_idx; // Index to track closest almanac
    double tDiff;
    gpstk::CivilTime ct;

    // Create output file and write header
    // FILE *fout = fopen("output.csv", "w");
    printf("PRN;year;month;day;hour;minute;second;TOA;tDiff;x;y;z\n");

    // Looping all required times
    gpstk::CommonTime t = tStart.convertToCommonTime();
    while (t <= t2 ) {
        // Looping all required satellites
        for (const auto PRN : PRNs) {
            tDiff = find_closest_time(alm_toa_map[PRN],t,alm_idx);
            pos = almOrbits[PRN].at(alm_idx).svXvt(t).getPos();
            ct = gpstk::CivilTime(t);
            printf("%d;%d;%d;%d;%d;%d;%f;%d;%f;%f;%f;%f\n",(int)PRN,ct.year,ct.month,ct.day,ct.hour,ct.minute,ct.second,
                (int)almOrbits[PRN].at(alm_idx).getToaSOW(),tDiff,pos[0],pos[1],pos[2]);
        }
        
        // Increment computation time by given tInterval value
        t.addSeconds((long)dt);
    }
}
