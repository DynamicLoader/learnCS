import sys
import argparse

parser = argparse.ArgumentParser(
    description="Run a simple service to manage the DBLP XML file"
)

parser.add_argument("-i", "--input", help="Path to the input file", required=True)
args = parser.parse_args(sys.argv[1:])
sys.path.append("./src/py")
import serviceWrapper

if __name__ == "__main__":
    s = serviceWrapper.ServiceWrapper()
    # s.setInput("xml/dblp_test.xml")

    s.setInput(args.input)

    s.parse()
    s.test_using()
    print("\n...TestStart...\n")
    print("\n...start testing queryByFullTitle()...\n")
    s.queryByFullTitle("Meltdown")
    print("\n...start testing queryByAuthor()...\n")
    s.queryByAuthor('Daniel Gruss')
    print("\n...start testing queryCoauthor()...\n")
    s.queryCoauthor('Daniel Gruss')

    print("\n...start getting top author...\n")
    s.queryTopAuthor()
    print("\n...start getting top keyword...\n")
    s.queryTopKeyword()
    print("\n...start getting keyword ")
    print(s.queryByKeyword("Meltdown"))
    print(s.queryByKeyword("Attacks"))
    s.saveTopYearKeyword()
    print(s.queryTopYearKeyword())