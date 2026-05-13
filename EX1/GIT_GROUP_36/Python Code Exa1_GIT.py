# Base converter - Hexadecimal / Decimal

# Checks if the string is a valid decimal number
def is_valid_decimal(num_str):
    if len(num_str) == 0:
        return False

    start = 0
    if num_str[0] == '-' or num_str[0] == '+':
        start = 1

    if start >= len(num_str):
        return False

    for i in range(start, len(num_str)):
        if num_str[i] < '0' or num_str[i] > '9':
            return False

    return True


# Checks if the string is a valid hexadecimal number
def is_valid_hex(hex_str):
    if len(hex_str) == 0:
        return False

    valid_chars = "0123456789abcdefABCDEF"

    start = 0
    if hex_str[0] == '-' or hex_str[0] == '+':
        start = 1

    if start >= len(hex_str):
        return False

    for i in range(start, len(hex_str)):
        found = False
        for c in valid_chars:
            if hex_str[i] == c:
                found = True
                break
        if not found:
            return False

    return True


# Converts a single hex character to its numeric value
def hex_char_to_val(c):
    if c >= '0' and c <= '9':
        return ord(c) - ord('0')
    elif c >= 'a' and c <= 'f':
        return ord(c) - ord('a') + 10
    elif c >= 'A' and c <= 'F':
        return ord(c) - ord('A') + 10
    else:
        return -1


# Converts a numeric value (0-15) to its hex character
def val_to_hex_char(val):
    if val >= 0 and val <= 9:
        return chr(ord('0') + val)
    elif val >= 10 and val <= 15:
        return chr(ord('A') + val - 10)
    else:
        return '?'


# Converts a decimal string to a hexadecimal string
def decimal_to_hex(decimal_str):
    if not is_valid_decimal(decimal_str):
        return "Invalid input - not a decimal number"

    negative = False
    start = 0
    if decimal_str[0] == '-':
        negative = True
        start = 1
    elif decimal_str[0] == '+':
        start = 1

    num = 0
    for i in range(start, len(decimal_str)):
        digit = ord(decimal_str[i]) - ord('0')
        num = num * 10 + digit

    if num == 0:
        return "0"

    # Repeated division by 16
    hex_digits = []
    while num > 0:
        remainder = num % 16
        hex_digits.append(val_to_hex_char(remainder))
        num = num // 16

    result = ""
    for i in range(len(hex_digits) - 1, -1, -1):
        result = result + hex_digits[i]

    if negative:
        result = "-" + result

    return result


# Converts a hexadecimal string to a decimal string
def hex_to_decimal(hex_str):
    if not is_valid_hex(hex_str):
        return "Invalid input - not a hexadecimal number"

    negative = False
    start = 0
    if hex_str[0] == '-':
        negative = True
        start = 1
    elif hex_str[0] == '+':
        start = 1

    result = 0
    for i in range(start, len(hex_str)):
        digit_val = hex_char_to_val(hex_str[i])
        result = result * 16 + digit_val

    if negative:
        result = -result

    return str(result)


# Runs tests on the conversion functions
def run_tests():
    print("=" * 50)
    print("Running tests...")
    print("=" * 50)

    passed = 0
    failed = 0

    tests_dec_to_hex = [
        ("0", "0"),
        ("10", "A"),
        ("15", "F"),
        ("16", "10"),
        ("255", "FF"),
        ("256", "100"),
        ("1000", "3E8"),
        ("4096", "1000"),
        ("-255", "-FF"),
        ("1", "1"),
        ("100", "64"),
    ]

    print("\n--- Decimal -> Hexadecimal ---")
    for dec_input, expected in tests_dec_to_hex:
        result = decimal_to_hex(dec_input)
        if result == expected:
            print(f"  PASS: {dec_input} -> {result}")
            passed += 1
        else:
            print(f"  FAIL: {dec_input} -> {result} (expected {expected})")
            failed += 1

    tests_hex_to_dec = [
        ("0", "0"),
        ("A", "10"),
        ("F", "15"),
        ("10", "16"),
        ("FF", "255"),
        ("100", "256"),
        ("3E8", "1000"),
        ("1000", "4096"),
        ("-FF", "-255"),
        ("ff", "255"),
        ("aB", "171"),
    ]

    print("\n--- Hexadecimal -> Decimal ---")
    for hex_input, expected in tests_hex_to_dec:
        result = hex_to_decimal(hex_input)
        if result == expected:
            print(f"  PASS: {hex_input} -> {result}")
            passed += 1
        else:
            print(f"  FAIL: {hex_input} -> {result} (expected {expected})")
            failed += 1

    print("\n--- Invalid inputs ---")
    invalid_tests = [
        ("decimal", "12.5"),
        ("decimal", "abc"),
        ("decimal", ""),
        ("decimal", "-"),
        ("hex", "GHI"),
        ("hex", "12.5"),
        ("hex", ""),
        ("hex", "-"),
    ]

    for test_type, bad_input in invalid_tests:
        if test_type == "decimal":
            result = decimal_to_hex(bad_input)
        else:
            result = hex_to_decimal(bad_input)

        if "Invalid" in result:
            print(f"  PASS: '{bad_input}' ({test_type}) -> detected as invalid")
            passed += 1
        else:
            print(f"  FAIL: '{bad_input}' ({test_type}) -> not detected as invalid")
            failed += 1

    print("\n" + "=" * 50)
    print(f"Total: {passed} passed, {failed} failed out of {passed + failed}")
    print("=" * 50)


# Main program - user menu
def main():
    print("=" * 50)
    print("  Base Converter - Decimal / Hexadecimal")
    print("=" * 50)

    while True:
        print("\nChoose an option:")
        print("  1 - Decimal to Hexadecimal")
        print("  2 - Hexadecimal to Decimal")
        print("  3 - Run tests")
        print("  4 - Exit")

        choice = input("\nChoice: ")

        if choice == "1":
            num = input("Enter a decimal number: ")
            result = decimal_to_hex(num)
            print(f"Result in base 16: {result}")

        elif choice == "2":
            num = input("Enter a hexadecimal number: ")
            result = hex_to_decimal(num)
            print(f"Result in base 10: {result}")

        elif choice == "3":
            run_tests()

        elif choice == "4":
            print("Goodbye!")
            break

        else:
            print("Invalid choice, try again")


if __name__ == "__main__":
    main()
