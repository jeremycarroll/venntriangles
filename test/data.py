# This Data is taken from the main picture of six trianglges, 
# using the letters from that picture, and giving the face label
# followed by the clockwise facial cycle.
# The outer face - uses the counter clockwise cycle.

DATA = """
- FEDCBA
A ABCEF
B BCFDA
C CDEB
D DEAC
E EFBD
F FADCE
AB ADFCB
BD DFA
BF FCD
BC CBEDF
CE BED
CD DCAE
AD CAE
DE EDBFA
BE BFD
EF FECB
CF CDE
DF DAC
AF AFECD
AE ECBF
AC CBE
ABC BCFE
ABF FDC
ABD DAF
BDF AFDC
BCF CFD
BCD DEAF
BCE EBD
CDE BDEA
ACD ACE
ADE EAFBC
DEF AFB
BDE BDF
BEF DFBC
CEF CEDB
CDF DCAE
ADF ADC
ACF CED
AEF FBCE
ABE BCF
ACE EBC
ABCF FCDE
ABDF DFAC
BCDF ACDF
ABCD FAE
BCDE AEDB
ACDE AECB
ABDE BFC
ADEF BFA
BDEF ABFDC
BCEF CBD
CDEF ABDE
ACDF EACD
ACEF CBDE
ABEF FCB
ABCE BEFC
ABCDE FEABC
ABDEF FBAC
BCDEF ACDB
ACDEF BAED
ABCEF DBCFE
ABCDF EDCAF
ABCDEF FCABDE
"""


def toFaceId(face):
    result = 0
    for ch in face:
        result |= 1<<(ord(ch) - ord('a'))
    return result

# rotate the letters in cycle to try
# all possibilities and return the alphabetically first
def rotate(cycle):
    result = cycle
    for i in range(1, len(cycle)):
        cycle = cycle[1:] + cycle[0]
        if cycle < result:
            result = cycle
    return result

# Main entry point:
if __name__ == '__main__':
    replacements = str.maketrans({
        "A": "e", 
        "B": "f", 
        "C": "d",
        "D": "a",
        "E": "b",
        "F": "c"})
    data = DATA.translate(replacements)
    labels = ["-"] * 64
    cycles = ["-"] * 64
    for line in data.splitlines():
        if line.strip():
            key, value = line.split()
            if key == "-":
                key = ""
            faceId = toFaceId(key)
            labels[faceId] = key
            cycles[faceId] = rotate(value)
    open = '{'
    close = '}'
    for sz in (3, 4, 5, 6):
        print(f"char * testData{sz}[][2] = {open}")
        for i in range(64):
            if len(cycles[i]) == sz:
                print(f"""  {open} "{''.join(sorted(labels[i]))}", "{cycles[i]}",  {close},""")
        print(f"{close};")
    # for i in range(64):
    #     print(f"""/* {i}: {''.join(sorted(labels[i]))} */ "{cycles[i]}",""")
