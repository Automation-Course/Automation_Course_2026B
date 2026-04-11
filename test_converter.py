from converter import HexCalculator

calc = HexCalculator()


def test_decimal_to_hex():
    assert calc.decimal_to_hex(0)    == "0"
    assert calc.decimal_to_hex(1)    == "1"
    assert calc.decimal_to_hex(16)   == "10"
    assert calc.decimal_to_hex(255)  == "FF"
    assert calc.decimal_to_hex(256)  == "100"
    assert calc.decimal_to_hex(-255) == "-FF"
    assert calc.decimal_to_hex(-16)  == "-10"
    print("PASSED: decimal_to_hex — all tests passed")


def test_hex_to_decimal():
    assert calc.hex_to_decimal("0")   == 0
    assert calc.hex_to_decimal("1")   == 1
    assert calc.hex_to_decimal("10")  == 16
    assert calc.hex_to_decimal("FF")  == 255
    assert calc.hex_to_decimal("ff")  == 255   # lowercase letters
    assert calc.hex_to_decimal("100") == 256
    assert calc.hex_to_decimal("-FF") == -255
    assert calc.hex_to_decimal("-10") == -16
    print("PASSED: hex_to_decimal — all tests passed")


def test_invalid_inputs():
    # invalid character
    try:
        calc.hex_to_decimal("!")
        print("FAILED: did not catch '!'")
    except ValueError:
        print("PASSED: caught invalid character '!'")

    # letter outside base 16
    try:
        calc.hex_to_decimal("GZ")
        print("FAILED: did not catch 'GZ'")
    except ValueError:
        print("PASSED: caught invalid character 'GZ'")

    # empty input
    try:
        calc.hex_to_decimal("")
        print("FAILED: did not catch empty input")
    except ValueError:
        print("PASSED: caught empty input")

    # minus sign only
    try:
        calc.hex_to_decimal("-")
        print("FAILED: did not catch minus only")
    except ValueError:
        print("PASSED: caught minus sign only")

    print("PASSED: invalid inputs — all tests passed")


if __name__ == "__main__":
    test_decimal_to_hex()
    test_hex_to_decimal()
    test_invalid_inputs()
