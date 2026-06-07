# Exercise A - Git
# Note: No built-in conversion functions are used
# (no hex(), int() with base, format(), oct(), bin())

import sys

HEX_DIGITS = "0123456789ABCDEF"


def parse_decimal_string(s):
    """
    Manually parses a decimal number string (integer or fraction)
    into integer_part and fractional_part as integers, plus the
    number of fractional digits.
    """

    dot_count = 0
    for ch in s:
        if ch == '.':
            dot_count += 1

    # If there is more than one decimal point, the number is invalid
    if dot_count > 1:
        raise ValueError("Invalid number: more than one decimal point.")

    if dot_count == 0:
        # Integer only
        for ch in s:
            if ch < '0' or ch > '9':
                raise ValueError(f"Invalid decimal digit: '{ch}'.")
        int_part = 0
        for ch in s:
            int_part = int_part * 10 + (ord(ch) - ord('0'))
        return int_part, 0, 0

    else:
        # Has decimal point
        parts = s.split('.')
        int_str = parts[0]
        frac_str = parts[1]

        # Validate integer part
        for ch in int_str:
            if ch < '0' or ch > '9':
                raise ValueError(f"Invalid decimal digit: '{ch}'.")
        # Validate fractional part
        for ch in frac_str:
            if ch < '0' or ch > '9':
                raise ValueError(f"Invalid decimal digit: '{ch}'.")

        int_part = 0
        if int_str != "":
            for ch in int_str:
                int_part = int_part * 10 + (ord(ch) - ord('0'))

        frac_part = 0
        frac_digits = len(frac_str)
        if frac_str != "":
            for ch in frac_str:
                frac_part = frac_part * 10 + (ord(ch) - ord('0'))

        return int_part, frac_part, frac_digits


def decimal_to_hex(decimal_str):
    """
    Converts a decimal (base 10) number string to hexadecimal (base 16).
    """

    # Remove whitespace to avoid invalid characters during manual validation
    decimal_str = decimal_str.strip()

    if decimal_str == "":
        raise ValueError("Input is empty.")

    # Handle negative sign
    negative = False
    if decimal_str[0] == '-':
        negative = True
        decimal_str = decimal_str[1:]
    elif decimal_str[0] == '+':
        decimal_str = decimal_str[1:]

    if decimal_str == "":
        raise ValueError("Input contains only a sign with no digits.")

    # Parse the decimal number manually
    int_part, frac_part, frac_digits = parse_decimal_string(decimal_str)

    # Convert integer part to hex by repeated division by 16
    if int_part == 0:
        hex_int = "0"
    else:
        hex_int = ""
        temp = int_part
        while temp > 0:
            remainder = temp % 16
            hex_int = HEX_DIGITS[remainder] + hex_int
            temp = temp // 16

    # Convert fractional part to hex by repeated multiplication by 16
    hex_frac = ""
    if frac_part > 0 and frac_digits > 0:
        # Build the fractional value as numerator / denominator
        numerator = frac_part
        denominator = 1
        for _ in range(frac_digits):
            denominator = denominator * 10

        max_frac_hex = 10  # limit fractional hex digits
        for _ in range(max_frac_hex):
            numerator = numerator * 16
            digit = numerator // denominator
            hex_frac = hex_frac + HEX_DIGITS[digit]
            numerator = numerator - digit * denominator
            if numerator == 0:
                break

    # Build final result
    if hex_frac != "":
        result = hex_int + "." + hex_frac
    else:
        result = hex_int

    if negative and result != "0":
        result = "-" + result

    return result


def hex_digit_value(ch):
    """Returns the numeric value of a hex digit (0-15)."""
    for i in range(16):
        if HEX_DIGITS[i] == ch:
            return i
    return -1


def hex_to_decimal(hex_str):
    """
    Converts a hexadecimal (base 16) string to decimal (base 10).
    """
    # Remove whitespace to avoid invalid characters during manual validation
    hex_str = hex_str.strip()

    if hex_str == "":
        raise ValueError("Input is empty.")

    # Handle negative sign
    negative = False
    if hex_str[0] == '-':
        negative = True
        hex_str = hex_str[1:]
    elif hex_str[0] == '+':
        hex_str = hex_str[1:]

    # Remove optional 0x / 0X prefix
    if len(hex_str) >= 2 and hex_str[0] == '0' and (hex_str[1] == 'x' or hex_str[1] == 'X'):
        hex_str = hex_str[2:]

    if hex_str == "":
        raise ValueError("Input contains no hexadecimal digits.")

    # Convert to uppercase for uniform processing
    hex_str = hex_str.upper()

    # Split into integer and fractional parts
    dot_count = 0
    for ch in hex_str:
        if ch == '.':
            dot_count += 1
    if dot_count > 1:
        raise ValueError("Invalid number: more than one decimal point.")

    if dot_count == 0:
        int_str = hex_str
        frac_str = ""
    else:
        parts = hex_str.split('.')
        int_str = parts[0]
        frac_str = parts[1]

    # Validate integer part
    for ch in int_str:
        if ch not in HEX_DIGITS:
            raise ValueError(f"Invalid hexadecimal digit: '{ch}'.")

    # Validate fractional part
    for ch in frac_str:
        if ch not in HEX_DIGITS:
            raise ValueError(f"Invalid hexadecimal digit: '{ch}'.")

    # Convert integer part: multiply by 16 for each digit
    int_result = 0
    for ch in int_str:
        int_result = int_result * 16 + hex_digit_value(ch)

    # Convert fractional part using exact arithmetic (numerator / denominator)
    if frac_str == "":
        # Integer only
        result_str = str_from_int(int_result)
    else:
        frac_numerator = 0
        frac_denominator = 1
        for ch in frac_str:
            frac_numerator = frac_numerator * 16 + hex_digit_value(ch)
            frac_denominator = frac_denominator * 16

        # Compute fractional decimal digits
        # frac_value = frac_numerator / frac_denominator
        # Multiply to get decimal digits
        frac_decimal = ""
        temp_num = frac_numerator
        temp_den = frac_denominator
        max_digits = 10
        for _ in range(max_digits):
            temp_num = temp_num * 10
            digit = temp_num // temp_den
            frac_decimal = frac_decimal + str_from_int(digit)
            temp_num = temp_num - digit * temp_den
            if temp_num == 0:
                break

        # Remove trailing zeros from fractional part
        while len(frac_decimal) > 1 and frac_decimal[-1] == '0':
            frac_decimal = frac_decimal[:-1]

        if frac_decimal == "" or frac_decimal == "0":
            result_str = str_from_int(int_result)
        else:
            result_str = str_from_int(int_result) + "." + frac_decimal

    if negative and result_str != "0":
        result_str = "-" + result_str

    return result_str


def str_from_int(n):
    """Convert a non-negative integer to string manually."""
    if n == 0:
        return "0"
    result = ""
    temp = n
    while temp > 0:
        digit = temp % 10
        result = HEX_DIGITS[digit] + result
        temp = temp // 10
    return result


def main():
    """
    Main interactive menu for the base converter calculator.
    """
    print("=" * 55)
    print("     Base Converter Calculator")
    print("     Hexadecimal <-> Decimal")
    print("=" * 55)

    while True:
        print("\n1. Decimal -> Hexadecimal")
        print("2. Hexadecimal -> Decimal")
        print("3. Exit")
        sys.stdout.flush()
        choice = input("Choose (1-3): ").strip()

        # Convert decimal input to hexadecimal
        if choice == "1":
            sys.stdout.flush()
            dec_input = input("Enter decimal number: ").strip()
            try:
                result = decimal_to_hex(dec_input)
                print("Result: " + result)
            except ValueError as e:
                print("Error:", e)
            sys.stdout.flush()

        # Convert hexadecimal input to decimal
        elif choice == "2":
            sys.stdout.flush()
            hex_input = input("Enter hexadecimal number: ").strip()
            try:
                result = hex_to_decimal(hex_input)
                print("Result: " + result)
            except ValueError as e:
                print("Error:", e)
            sys.stdout.flush()

        elif choice == "3":
            print("Goodbye!")
            break

        else:
            print("Invalid option. Please choose 1-3.")


if __name__ == "__main__":
    main()