"""
Group 25 - Final Python Homework
Custom Base Converter Calculator
Decimal <-> Hexadecimal conversion without built-in functions.
"""


def decimal_to_hex(decimal_value):
    """
    Convert a decimal (base 10) integer to a hexadecimal (base 16) string.
    Uses the repeated division method.
    No built-in conversion functions (hex, bin, oct, int with base) are used.

    Args:
        decimal_value: An integer in base 10.

    Returns:
        A string representing the hexadecimal value (uppercase, e.g. "FF").

    Raises:
        TypeError: If the input is not an integer.
        ValueError: If the input is a negative number.
    """
    if not isinstance(decimal_value, int) or isinstance(decimal_value, bool):
        raise TypeError("Input must be an integer.")
    if decimal_value < 0:
        raise ValueError("Input must be a non-negative integer.")
    if decimal_value == 0:
        return "0"

    hex_digits = "0123456789ABCDEF"
    result = ""
    value = decimal_value

    while value > 0:
        remainder = value % 16
        result = hex_digits[remainder] + result
        value = value // 16

    return result


def hex_to_decimal(hex_string):
    """
    Convert a hexadecimal (base 16) string to a decimal (base 10) integer.
    Uses the sum of powers method.
    No built-in conversion functions (hex, bin, oct, int with base) are used.

    Args:
        hex_string: A string representing a hexadecimal number (e.g. "FF", "a1b2").

    Returns:
        An integer representing the decimal value.

    Raises:
        TypeError: If the input is not a string.
        ValueError: If the string contains invalid hexadecimal characters or is empty.
    """
    if not isinstance(hex_string, str):
        raise TypeError("Input must be a string.")
    if len(hex_string) == 0:
        raise ValueError("Input string must not be empty.")

    hex_string = hex_string.upper()
    hex_chars = "0123456789ABCDEF"

    # Validate all characters
    for ch in hex_string:
        if ch not in hex_chars:
            raise ValueError(f"Invalid hexadecimal character: '{ch}'")

    result = 0
    for i, ch in enumerate(reversed(hex_string)):
        digit_value = hex_chars.index(ch)
        result += digit_value * (16 ** i)

    return result


def main():
    """Interactive command-line calculator loop."""
    print("=" * 45)
    print("  Custom Base Converter Calculator (Group 25)")
    print("  Decimal <-> Hexadecimal")
    print("=" * 45)

    while True:
        print("\nMenu:")
        print("  1: Decimal to Hexadecimal")
        print("  2: Hexadecimal to Decimal")
        print("  3: Exit")

        choice = input("\nEnter your choice (1/2/3): ").strip()

        if choice == "1":
            user_input = input("Enter a decimal number: ").strip()
            try:
                decimal_value = int(user_input)
                if decimal_value < 0:
                    print("Error: Please enter a non-negative integer.")
                    continue
                hex_result = decimal_to_hex(decimal_value)
                print(f"Result: {decimal_value} (Dec) = {hex_result} (Hex)")
            except ValueError:
                print(f"Error: '{user_input}' is not a valid integer.")

        elif choice == "2":
            hex_input = input("Enter a hexadecimal string: ").strip()
            try:
                decimal_result = hex_to_decimal(hex_input)
                print(f"Result: {hex_input.upper()} (Hex) = {decimal_result} (Dec)")
            except (TypeError, ValueError) as e:
                print(f"Error: {e}")

        elif choice == "3":
            print("Goodbye!")
            break

        else:
            print("Error: Invalid choice. Please enter 1, 2, or 3.")


if __name__ == "__main__":
    main()