#!/usr/bin/env python3
"""
Compare numériquement deux à deux les octets de deux fichiers binaires.
Si la différence entre deux octets de même indice est suffisament faible
(inférieure à max_offset) on considère les deux octets identiques.
"""
import sys


def main():
    """
    Fonction principale
    """
    max_offset = 4
    count = 0
    res = ["\033[93m", "\033[92m [OK] "]
    with open(sys.argv[1], "rb") as file1:
        with open(sys.argv[2], "rb") as file2:
            byte1 = file1.read(1)
            byte2 = file2.read(1)
            while byte1 and byte2:
                offset = abs((ord(byte1) - ord(byte2)))
                if offset > max_offset:
                    count += 1
                byte1 = file1.read(1)
                byte2 = file2.read(1)
    print(res[count == 0] + str(count) + " octets diffèrent\033[0m")
main()
