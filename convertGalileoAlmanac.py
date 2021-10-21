from argparse import ArgumentParser
from pathlib import Path
from dateutil import parser as datetime_parser
import xmltodict
from python_helpers import YUMAAlmanacSingle


if __name__ == "__main__":
    # Parse inputs
    arg_parser = ArgumentParser(description='Convert Galileo almanac to YUMA format')
    arg_parser.add_argument('almanac_file', metavar='N', type=Path, help='name of Galileo almanac XML file')
    arg_parser.add_argument('--output', dest='output', default='almanac_file.alm', help='output YUMA file  (default: almanac_file.alm)')
    args = arg_parser.parse_args()

    # Resolve relative paths of inputs
    almanac_file = args.almanac_file.resolve()
    args_file = Path(args.output).resolve()

    # Read XML file
    with open(almanac_file) as f:
        d = xmltodict.parse(f.read())
        issue_date_str = d['signalData']['header']['GAL-header']['issueDate']
        issue_date = datetime_parser.parse(issue_date_str)
        almanacs = d['signalData']['body']['Almanacs']['svAlmanac']

        yuma = YUMAAlmanacSingle(id=5,ecc=0.05,toa=47456.0,inc=0.9828047129,ra0=-1.056233407,rate_ra=-0.000000007783181344,
                                 sqrt_a=5153.651367,w=0.821905483,M0=1.97760571,week_short=92)

        print(yuma)
