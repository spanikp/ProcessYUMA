#!/usr/bin/env python3

from argparse import ArgumentParser
from pathlib import Path
from datetime import datetime
from math import pi, sqrt

from dateutil import parser as datetime_parser
from dateutil import tz
import xmltodict


# Define constants
GPS_GM = 3.986004418e14
GPS_TIME_START = datetime(1980, 1, 6, 0, 0, 0, tzinfo=tz.UTC)
GALILEO_TIME_START = datetime(1999, 8, 21, 23, 59, 47, tzinfo=tz.UTC)
GALILEO_NOMINAL_SQRTA = sqrt(29600000.0)
GALILEO_NOMINAL_INCLINATION_RAD = pi*56/180


def get_gps_week(t: datetime) -> int:
    return (t - GPS_TIME_START).days // 7


class YUMAAlmanacSingle:
    def __init__(self, id: int, ecc: float, toa: float, inc: float, ra0: float,
                 rate_ra: float, sqrt_a: float, w: float, M0: float, week_short: int,
                 af0: float = 0.0, af1: float = 0.0, health: int = 0):
        self.id = id                # Satellite ID
        self.ecc = ecc              # Ellipse eccentricity (unitless)
        self.toa = toa              # Time of applicability (second of GPS week)
        self.inc = inc              # Orbital plance inclination (rad)
        self.ra0 = ra0              # Right ascension (rad)
        self.rate_ra = rate_ra      # Rate of Right ascension (rad/s)
        self.sqrt_a = sqrt_a        # Square root of semi-major axis (m^1/2)
        self.w = w                  # Argument of perigee (rad)
        self.M0 = M0                # Mean anomaly (rad)
        self.week_short = week_short   # GPS week number modulo 1024 (int)

        # Optional inputs
        self.af0 = af0              # Clock offset (s)
        self.af1 = af1              # Clock drift (s/s)
        self.health = health        # Satellite health status

    def __str__(self) -> str:
        return f"******** Week {self.week_short} almanac for PRN-{self.id:02d} ********\n" + \
            f"ID:                         {self.id:02d}\n" + \
            f"Health:                     {self.health:03d}\n" + \
            f"Eccentricity:            {self.ecc: 23.14E}\n" + \
            f"Time of Applicability(s): {self.toa: 11.4f}\n" + \
            f"Orbital Inclination(rad):{self.inc: 23.14E}\n" + \
            f"Rate of Right Ascen(r/s):{self.rate_ra: 23.14E}\n" + \
            f"SQRT(A)  (m 1/2):        {self.sqrt_a:>18.10f}\n" + \
            f"Right Ascen at Week(rad):{self.ra0:>23.14E}\n" + \
            f"Argument of Perigee(rad):{self.w:>23.14E}\n" + \
            f"Mean Anom(rad):          {self.M0:>23.14E}\n" + \
            f"Af0(s):                  {self.af0:>23.14E}\n" + \
            f"Af1(s/s):                {self.af1:>23.14E}\n" + \
            f"week:                       {self.week_short:>4d}\n\n"

    @classmethod
    def from_GalileoAlmanac(cls, alm_issue_date: datetime, svid: int, alm):
        full_week = get_gps_week(alm_issue_date)
        week_short = full_week % 1024
        aSqRoot = GALILEO_NOMINAL_SQRTA + float(alm['aSqRoot'])
        ecc = float(alm['ecc'])
        inc = GALILEO_NOMINAL_INCLINATION_RAD + pi*float(alm['deltai'])
        omega0 = pi*float(alm['omega0'])
        omegaDot = pi*float(alm['omegaDot'])
        w = pi*float(alm['w'])
        m0 = pi*float(alm['m0'])
        toa = float(alm['t0a'])
        af0 = float(alm['af0'])
        af1 = float(alm['af1'])

        return cls(id=svid, ecc=ecc, toa=toa, inc=inc, ra0=omega0, rate_ra=omegaDot,
                   sqrt_a=aSqRoot, w=w, M0=m0, week_short=week_short, af0=af0, af1=af1)

    @classmethod
    def from_KeplerianElements(cls, svid, toa_posix, ecc, a, inc, Omega0, w, tp_posix):
        toa_dt = datetime.fromtimestamp(toa_posix, tz=tz.UTC)
        full_gps_week = get_gps_week(toa_dt)
        week_short = full_gps_week % 1024
        week_day = toa_dt.weekday()+1
        if week_day == 7:
            week_day = 0
        toa = float(week_day*86400 + toa_dt.hour*3600 + toa_dt.minute*60 + toa_dt.second + toa_dt.microsecond/1000000)
        n = sqrt(GPS_GM/a**3)
        M0 = n*(toa_posix-tp_posix)

        return cls(id=svid, ecc=ecc, toa=toa, inc=inc, ra0=Omega0, rate_ra=0.0,
                   sqrt_a=sqrt(a), w=w, M0=M0, week_short=week_short)


def main(input_file: Path, output_file: Path) -> None:
    # Switch Galileo/Keplerian elements input file
    if input_file.suffix == '.xml':
        # Read Galileo XML almanac file
        with open(input_file) as f:
            d = xmltodict.parse(f.read())
            issue_date_str = d['signalData']['header']['GAL-header']['issueDate']
            issue_date = datetime_parser.parse(issue_date_str)
            almanacs = d['signalData']['body']['Almanacs']['svAlmanac']

            # If there is non-empty list of almanacs write to file
            if len(almanacs) > 0:
                # Open output file
                with open(output_file, "w") as fout:
                    # Looping through satellites in Galileo XML almanac file
                    for almanac in almanacs:
                        svid = int(almanac['SVID'])
                        almanac_dict = almanac['almanac']
                        yuma = YUMAAlmanacSingle.from_GalileoAlmanac(alm_issue_date=issue_date, svid=svid,
                                                                     alm=almanac_dict)
                        fout.write(str(yuma))
            else:
                f"Input file '{input_file.name}' does not contain any satellite data!"

    elif input_file.suffix == '.csv':
        # Read keplerian parameters file
        with open(input_file) as f:
            header_reference = "Satellite identifier,Time of applicability,Eccentricity,Semimajor axis,Inclination of the orbital plane,Right ascension of ascending node,Argument of perigee,Perigee passing time"
            d = f.readlines()

            if len(d) > 1:
                if header_reference == d[0][:-1]:
                    # Open output file
                    with open(output_file, "w") as fout:
                        # Looping through satellites in CSV file
                        for line_number, line in enumerate(d[1:]):
                            # Check number of delimiters on line
                            if line.count(',') == 7:
                                svid, toa_posix, ecc, a, inc, Omega0, w, tp_posix = tuple(map(float, line.split(',')))
                                yuma = YUMAAlmanacSingle.from_KeplerianElements(int(svid), toa_posix, ecc, a, inc,
                                                                                Omega0, w, tp_posix)
                                fout.write(str(yuma))
                            else:
                                print(
                                    f"Incorrect format of Keplerian elements on line {line_number + 2} - skipping satellite!")
                else:
                    print("Incorrect header of Keplerian parameter file!")
            else:
                f"Input file '{input_file.name}' does not contain any Keplerian elements data!"

    else:
        print("Not supported input file! Only Galileo almanc (.xml) or Keplerian orbit file (.csv) is allowed!")


if __name__ == "__main__":
    # Parse inputs
    arg_parser = ArgumentParser(description='Conversion to YUMA almanac format')
    arg_parser.add_argument('input_file', type=Path, help='name of Galileo almanac (.xml) or Keplerian parameters file (.csv)')
    arg_parser.add_argument('-o', '--output', dest='output', default='almanac_file.alm', help='output YUMA file  (default: almanac_file.alm)')
    args = arg_parser.parse_args()

    # Resolve relative paths of inputs
    input_file = args.input_file.resolve()
    output_file = Path(args.output).resolve()

    # Call main function
    main(input_file, output_file)

