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
    output_file = Path(args.output).resolve()

    # Read Galileo XML almanac file
    with open(almanac_file) as f:
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
                    yuma = YUMAAlmanacSingle.from_GalileoAlmanac(alm_issue_date=issue_date, svid=svid, alm=almanac_dict)
                    fout.write(str(yuma))
        else:
            f"Input file '{almanac_file.name}' does not contain any satellite data!"
