DIGITS = "0123456789ABCDEF"
def decimal_to_hex(num):
    if num == 0:
        return "0"
    negative = False
    if num < 0:
        negative = True
        num = -num
    integer_part = int(num)
    fraction_part = num - integer_part
    integer_result = ""
    if integer_part == 0:
        integer_result = "0"
    else:
        while integer_part > 0:
            remainder = integer_part % 16
            integer_result = DIGITS[remainder] + integer_result
            integer_part = integer_part // 16
    if fraction_part == 0:
        result = integer_result
    else:
        fraction_result = ""
        count = 0
        while fraction_part > 0 and count < 8:
            fraction_part = fraction_part * 16
            digit = int(fraction_part)
            fraction_result += DIGITS[digit]
            fraction_part = fraction_part - digit
            count += 1
        result = integer_result + "." + fraction_result
    if negative:
        result = "-" + result
    return result


def hex_to_decimal(h):
    if h == "":
        return None
    h = h.upper()
    negative = False
    if h[0] == "-":
        negative = True
        h = h[1:]
    if h == "" or h.count(".") > 1:
        return None
    parts = h.split(".")
    integer_part = parts[0]
    fraction_part = parts[1] if len(parts) == 2 else ""
    if integer_part == "":
        integer_part = "0"
    result = 0
    for char in integer_part:
        if char not in DIGITS:
            return None
        value = DIGITS.index(char)
        result = result * 16 + value
    power = 16
    for char in fraction_part:
        if char not in DIGITS:
            return None
        value = DIGITS.index(char)
        result += value / power
        power *= 16
    if negative:
        result = -result
    return result


def parse_decimal_input(s):
    if s == "":
        return None
    negative = False
    if s[0] == "-":
        negative = True
        s = s[1:]
    if s == "" or s.count(".") > 1:
        return None
    parts = s.split(".")
    integer_part = parts[0]
    fraction_part = parts[1] if len(parts) == 2 else ""
    if integer_part == "":
        integer_part = "0"
    number = 0
    for char in integer_part:
        if char not in "0123456789":
            return None
        number = number * 10 + (ord(char) - ord("0"))
    divisor = 10
    for char in fraction_part:
        if char not in "0123456789":
            return None
        number += (ord(char) - ord("0")) / divisor
        divisor *= 10
    if negative:
        number = -number
    return number

def run_tests():
    print("\nStarting Tests...\n")

    tests = [
        ("Decimal to Hex", "263", decimal_to_hex(263), "107"),
        ("Decimal to Hex", "-26", decimal_to_hex(-26), "-1A"),
        ("Decimal to Hex", "10.625", decimal_to_hex(10.625), "A.A"),
        ("Decimal to Hex", "0.5", decimal_to_hex(0.5), "0.8"),
        ("Hex to Decimal", "1A", hex_to_decimal("1A"), 26),
        ("Hex to Decimal", "107", hex_to_decimal("107"), 263),
        ("Hex to Decimal", "-1A", hex_to_decimal("-1A"), -26),
        ("Hex to Decimal", "A.A", hex_to_decimal("A.A"), 10.625),
        ("Invalid Hex", "1G", hex_to_decimal("1G"), None),
        ("Invalid Decimal", "12..3", parse_decimal_input("12..3"), None),
    ]
    passed = 0
    for test_type, input_val, result, expected in tests:
        if isinstance(expected, float):
            ok = abs(result - expected) < 0.000001
        else:
            ok = result == expected
        if ok:
            print(f"[{test_type}] Input: {input_val} -> Output: {result} -> PASSED")
            passed += 1
        else:
            print(f"[{test_type}] Input: {input_val} -> FAILED")
            print(f"   Expected: {expected}")
            print(f"   Got:      {result}")
    print(f"\nSummary: {passed}/{len(tests)} tests passed.\n")


def main():
    while True:
        print("\nBase Converter")
        print("1 - Decimal to Hex")
        print("2 - Hex to Decimal")
        print("3 - Run Tests")
        print("4 - Exit")
        choice = input("Choose an option: ")
        if choice == "1":
            decimal_input = input("Enter decimal number: ")
            number = parse_decimal_input(decimal_input)
            if number is None:
                print("Invalid decimal input")
            else:
                print("Hex:", decimal_to_hex(number))
        elif choice == "2":
            hex_input = input("Enter hex number: ")
            result = hex_to_decimal(hex_input)
            if result is None:
                print("Invalid hexadecimal input")
            else:
                print("Decimal:", result)
        elif choice == "3":
            run_tests()
        elif choice == "4":
            print("Goodbye!")
            break
        else:
            print("Invalid choice, please try again.")

if __name__ == "__main__":
    main()