from math import pi, sqrt
from datetime import datetime
from dateutil import tz

# Constants
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
            f"Eccentricity:            {self.ecc: 19.10E}\n" + \
            f"Time of Applicability(s): {self.toa: 11.4f}\n" + \
            f"Orbital Inclination(rad):{self.inc: 19.10E}\n" + \
            f"Rate of Right Ascen(r/s):{self.ra0: 19.10E}\n" + \
            f"SQRT(A)  (m 1/2):        {self.sqrt_a:>14.6f}\n" + \
            f"Right Ascen at Week(rad):{self.ra0:>19.10E}\n" + \
            f"Argument of Perigee(rad):{self.w:>19.10E}\n" + \
            f"Mean Anom(rad):          {self.M0:>19.10E}\n" + \
            f"Af0(s):                  {self.af0:>19.10E}\n" + \
            f"Af1(s/s):                {self.af1:>19.10E}\n" + \
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



