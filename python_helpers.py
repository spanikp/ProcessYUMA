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

    def __repr__(self) -> str:
        # Handling spacing appends
        s_ra0 = '' if self.ra0 < 0 else ' '
        s_M0 = '' if self.M0 < 0 else ' '
        s_af0 = '' if self.af0 < 0 else ' '
        s_af1 = '' if self.af1 < 0 else ' '

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
            f"week:                       {self.week_short:>4d}\n"