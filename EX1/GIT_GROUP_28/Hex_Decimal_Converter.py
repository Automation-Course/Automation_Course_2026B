def manual_int_to_hex(n):
    """Manual conversion of a positive integer to hexadecimal string without using hex()."""
    if n == 0:
        return "0"
    hex_chars = "0123456789ABCDEF"
    result = ""
    while n > 0:
        # Get the remainder (0-15) and prepend the corresponding character
        result = hex_chars[n % 16] + result
        n //= 16
    return result


def manual_hex_char_to_int(char):
    """Manual conversion of a single hex character to its decimal integer value."""
    char = char.upper()
    hex_chars = "0123456789ABCDEF"
    if char not in hex_chars:
        raise ValueError("Invalid Hex Character")
    # Return the index which corresponds to the decimal value
    return hex_chars.index(char)


def decimal_to_hex(decimal_num):
    # Strip whitespace to handle cases with only spaces
    val = str(decimal_num).strip()
    if not val:
        return "Error: Input is empty."

    try:
        float_val = float(val)
        is_negative = float_val < 0
        float_val = abs(float_val)

        int_part = int(float_val)
        frac_part = float_val - int_part

        # Convert integer part using the manual function
        hex_int = manual_int_to_hex(int_part)

        # Convert fractional part manually
        hex_frac = ""
        precision = 10  # Limit precision for repeating fractions (like 0.1)
        hex_chars = "0123456789ABCDEF"

        temp_frac = frac_part
        while temp_frac > 0 and len(hex_frac) < precision:
            temp_frac *= 16
            digit = int(temp_frac)
            hex_frac += hex_chars[digit]
            temp_frac -= digit

        result = f"{hex_int}.{hex_frac}" if hex_frac else hex_int
        return f"-{result}" if is_negative else result

    except ValueError:
        return "Error: Invalid decimal characters."


def hex_to_decimal(hex_str):
    val = str(hex_str).strip()
    if not val:
        return "Error: Input is empty."

    try:
        # Handle sign
        is_negative = val.startswith('-')
        if is_negative:
            val = val[1:]

        if val.count('.') > 1:
            return "Error: Invalid hexadecimal characters."

        parts = val.split('.')
        int_part_str = parts[0]
        frac_part_str = parts[1] if len(parts) > 1 else ""

        # Convert integer part manually using powers of 16
        decimal_int = 0
        for i, char in enumerate(reversed(int_part_str)):
            decimal_int += manual_hex_char_to_int(char) * (16 ** i)

        # Convert fractional part manually using negative powers of 16
        decimal_frac = 0.0
        for i, char in enumerate(frac_part_str):
            decimal_frac += manual_hex_char_to_int(char) * (16 ** -(i + 1))

        result = decimal_int + decimal_frac

        # Format as int if there's no fractional part
        if result == int(result):
            result = int(result)

        return -result if is_negative else result

    except ValueError:
        return "Error: Invalid hexadecimal characters."


def run_test_suite():
    """Automated test suite with specific error descriptions."""
    tests = [
        ("255", "dec_to_hex"),
        ("10", "dec_to_hex"),
        ("0", "dec_to_hex"),
        ("A", "hex_to_dec"),
        ("ff", "hex_to_dec"),
        ("G1", "hex_to_dec"),
        ("12.5", "dec_to_hex"),
        ("C.8", "hex_to_dec"),
        ("-12.5", "dec_to_hex"),
        ("-C.8", "hex_to_dec"),
        ("0.1", "dec_to_hex"),
        ("abc", "dec_to_hex"),
        ("", "dec_to_hex"),
        ("    ", "hex_to_dec")
    ]

    print("\n" + "=" * 65)
    print(f"{'Input':<15} | {'Test Type':<15} | {'Result / Error Message':<30}")
    print("-" * 65)

    for val, test_type in tests:
        if test_type == "dec_to_hex":
            result = decimal_to_hex(val)
            t_name = "Dec -> Hex"
        else:
            result = hex_to_decimal(val)
            t_name = "Hex -> Dec"

        print(f"{repr(val):<15} | {t_name:<15} | {str(result):<30}")
    print("=" * 65 + "\n")


def main():
    while True:
        print("--- Base Converter (Manual Logic) ---")
        print("1. Decimal to Hexadecimal")
        print("2. Hexadecimal to Decimal")
        print("3. Run Automated Test Suite")
        print("4. Exit")

        choice = input("Select choice (1-4): ")

        if choice == '1':
            val = input("Enter decimal: ")
            print(f"Result: {decimal_to_hex(val)}\n")
        elif choice == '2':
            val = input("Enter hex: ")
            print(f"Result: {hex_to_decimal(val)}\n")
        elif choice == '3':
            run_test_suite()
        elif choice == '4':
            break
        else:
            print("Invalid choice.\n")


if __name__ == "__main__":
    main()