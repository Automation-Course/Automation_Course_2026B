"""
Group 25 - Final Python Homework
Custom Base Converter Calculator
Decimal <-> Hexadecimal conversion without built-in functions.
"""


# LIMIT ENFORCEMENT: 9-digit max constants for INPUT validation (both directions)
MAX_INTEGER_DIGITS = 9       # Max digits before decimal point (excluding minus sign)
MAX_FRACTIONAL_DIGITS = 9    # Max digits after decimal point (excluding decimal point)

# Internal precision for OUTPUT (unrestricted from input limits)
MAX_OUTPUT_PRECISION = 12    # Max hex digits in fractional output (handles periodic fractions)


def decimal_to_hex(decimal_value):
    """
    Convert a decimal (base 10) number to a hexadecimal (base 16) string.
    Supports both integers and floating-point numbers.
    Uses the repeated division method for the integer part and
    repeated multiplication method for the fractional part.
    No built-in conversion functions (hex, bin, int with base) are used.

    Args:
        decimal_value: A number (int or float) in base 10.

    Returns:
        A string representing the hexadecimal value (uppercase, e.g. "FF", "1A.A").
        Output precision is up to MAX_OUTPUT_PRECISION (12) digits for periodic fractions.

    Raises:
        TypeError: If the input is not a number (int or float).
        ValueError: If the integer part exceeds MAX_INTEGER_DIGITS (9) digits.
    """
    # Type validation
    if isinstance(decimal_value, bool):
        raise TypeError("Input must be a number (int or float).")
    if not isinstance(decimal_value, (int, float)):
        raise TypeError("Input must be a number (int or float).")
    
    # Validate input digit limits
    abs_value = abs(decimal_value)
    integer_part_check = int(abs_value)
    
    # LIMIT ENFORCEMENT: 9-digit max - Count digits in integer part via repeated division
    if integer_part_check == 0:
        integer_digit_count = 1
    else:
        integer_digit_count = 0
        temp = integer_part_check
        while temp > 0:
            integer_digit_count += 1
            temp //= 10
    
    # LIMIT ENFORCEMENT: 9-digit max - Raise error if exceeded
    if integer_digit_count > MAX_INTEGER_DIGITS:
        raise ValueError(f"Integer part exceeds maximum of {MAX_INTEGER_DIGITS} digits. Got {integer_digit_count} digits.")
    
    # Note: Fractional part validation for floats is not strictly enforced
    # since Python floats have inherent precision limitations.
    # The output is limited to MAX_FRACTIONAL_DIGITS instead.
    
    hex_digits = "0123456789ABCDEF"
    
    # Handle negative numbers
    is_negative = decimal_value < 0
    if is_negative:
        decimal_value = abs(decimal_value)
    
    # Separate integer and fractional parts
    integer_part = int(decimal_value)
    fractional_part = decimal_value - integer_part
    
    # Convert integer part using repeated division
    if integer_part == 0:
        integer_hex = "0"
    else:
        integer_hex = ""
        value = integer_part
        while value > 0:
            remainder = value % 16
            integer_hex = hex_digits[remainder] + integer_hex
            value = value // 16
    
    # OUTPUT PRECISION: Unrestricted from input limits, uses MAX_OUTPUT_PRECISION for periodic fractions
    fractional_hex = ""
    if fractional_part > 0:
        for _ in range(MAX_OUTPUT_PRECISION):  # OUTPUT: Up to 12 digits for full precision
            fractional_part *= 16
            digit = int(fractional_part)
            fractional_hex += hex_digits[digit]
            fractional_part -= digit
            # Stop if fractional part becomes 0
            if fractional_part == 0:
                break
    
    # Build result
    if fractional_hex:
        result = f"{integer_hex}.{fractional_hex}"
    else:
        result = integer_hex
    
    # Add negative sign if needed
    if is_negative:
        result = "-" + result
    
    return result


def hex_to_decimal(hex_string):
    """
    Convert a hexadecimal (base 16) string to a decimal (base 10) number.
    Supports integers, fractions, and negative numbers.
    Uses the sum of powers method.
    No built-in conversion functions (hex, bin, oct, int with base) are used.

    Args:
        hex_string: A string representing a hexadecimal number (e.g. "FF", "1A.A", "-1A.A").
                   Input limited to MAX_INTEGER_DIGITS (9) and MAX_FRACTIONAL_DIGITS (9).

    Returns:
        A number (int or float) representing the decimal value.

    Raises:
        TypeError: If the input is not a string.
        ValueError: If the string contains invalid characters, is empty, has multiple decimal points,
                   or has more than MAX_FRACTIONAL_DIGITS digits after the decimal point.
    """
    if not isinstance(hex_string, str):
        raise TypeError("Input must be a string.")
    if len(hex_string) == 0:
        raise ValueError("Input string must not be empty.")

    hex_string = hex_string.upper()
    hex_chars = "0123456789ABCDEF"
    
    # Handle negative sign
    is_negative = False
    if hex_string.startswith('-'):
        is_negative = True
        hex_string = hex_string[1:]
        if len(hex_string) == 0:
            raise ValueError("Input string must not be empty after minus sign.")
    
    # Validate no multiple minus signs or misplaced minus
    if '-' in hex_string:
        raise ValueError("Invalid minus sign position.")
    
    # Check for multiple decimal points
    if hex_string.count('.') > 1:
        raise ValueError("Input contains multiple decimal points.")
    
    # Split into integer and fractional parts
    if '.' in hex_string:
        parts = hex_string.split('.')
        # Check for just a dot with nothing else
        if not parts[0] and not parts[1]:
            raise ValueError("Input string cannot be just a decimal point.")
        integer_part = parts[0] if parts[0] else "0"
        fractional_part = parts[1] if parts[1] else ""
    else:
        integer_part = hex_string
        fractional_part = ""
    
    # Validate all characters in integer part
    for ch in integer_part:
        if ch not in hex_chars:
            raise ValueError(f"Invalid hexadecimal character: '{ch}'")
    
    # Validate all characters in fractional part
    for ch in fractional_part:
        if ch not in hex_chars:
            raise ValueError(f"Invalid hexadecimal character: '{ch}'")
    
    # LIMIT ENFORCEMENT: 9-digit max - Validate integer part length via len()
    if len(integer_part) > MAX_INTEGER_DIGITS:
        raise ValueError(f"Integer part exceeds maximum of {MAX_INTEGER_DIGITS} digits. Got {len(integer_part)} digits.")
    
    # LIMIT ENFORCEMENT: 9-digit max - Validate fractional part length via len()
    if len(fractional_part) > MAX_FRACTIONAL_DIGITS:
        raise ValueError(f"Fractional part exceeds maximum of {MAX_FRACTIONAL_DIGITS} digits. Got {len(fractional_part)} digits.")
    
    # Convert integer part: sum of digit * 16^position
    integer_result = 0
    for i, ch in enumerate(reversed(integer_part)):
        digit_value = hex_chars.index(ch)
        integer_result += digit_value * (16 ** i)
    
    # Convert fractional part: sum of digit * 16^(-position)
    fractional_result = 0.0
    for i, ch in enumerate(fractional_part):
        digit_value = hex_chars.index(ch)
        fractional_result += digit_value * (16 ** -(i + 1))
    
    # Combine results
    result = integer_result + fractional_result
    
    # Apply negative sign
    if is_negative:
        result = -result
    
    # Return int if no fractional part, otherwise float
    if fractional_result == 0 and not is_negative:
        return int(result)
    elif fractional_result == 0:
        return int(result)
    else:
        return result


def validate_decimal_input(input_string):
    """
    LIMIT ENFORCEMENT: Validate decimal input string BEFORE parsing.
    Checks digit limits on the raw string to prevent float rounding bypass.
    
    Args:
        input_string: Raw user input string (e.g., "123.456", "-999999999.123456789")
    
    Returns:
        None if valid
    
    Raises:
        ValueError: If integer or fractional part exceeds 9 digits, or invalid format.
    """
    s = input_string.strip()
    
    # Check for invalid minus sign patterns
    if s.count('-') > 1:
        raise ValueError(f"'{s}' contains multiple minus signs.")
    if s.endswith('-'):
        raise ValueError(f"'{s}' has minus sign in wrong position.")
    if '-' in s and not s.startswith('-'):
        raise ValueError(f"'{s}' has minus sign in wrong position.")
    
    # Remove leading minus sign for digit counting (minus doesn't count)
    if s.startswith('-'):
        s = s[1:]
    
    # Check for multiple decimal points
    if s.count('.') > 1:
        raise ValueError("Input contains multiple decimal points.")
    
    # Split into integer and fractional parts
    if '.' in s:
        parts = s.split('.')
        integer_part = parts[0] if parts[0] else "0"
        fractional_part = parts[1] if parts[1] else ""
    else:
        integer_part = s
        fractional_part = ""
    
    # Remove leading zeros from integer part for accurate count (except "0" itself)
    integer_part_stripped = integer_part.lstrip('0') or '0'
    
    # LIMIT ENFORCEMENT: 9-digit max for integer part
    if len(integer_part_stripped) > MAX_INTEGER_DIGITS:
        raise ValueError(f"Integer part exceeds maximum of {MAX_INTEGER_DIGITS} digits. Got {len(integer_part_stripped)} digits.")
    
    # LIMIT ENFORCEMENT: 9-digit max for fractional part
    if len(fractional_part) > MAX_FRACTIONAL_DIGITS:
        raise ValueError(f"Fractional part exceeds maximum of {MAX_FRACTIONAL_DIGITS} digits. Got {len(fractional_part)} digits.")


def validate_hex_input(input_string):
    """
    LIMIT ENFORCEMENT: Validate hex input string BEFORE conversion.
    Checks digit limits on the raw string.
    
    Args:
        input_string: Raw user input string (e.g., "FF", "1A.A", "-ABCD.1234")
    
    Returns:
        None if valid
    
    Raises:
        ValueError: If integer or fractional part exceeds 9 digits, or invalid format.
    """
    s = input_string.strip().upper()
    
    if len(s) == 0:
        raise ValueError("Input string must not be empty.")
    
    # Remove leading minus sign for digit counting (minus doesn't count)
    if s.startswith('-'):
        s = s[1:]
        if len(s) == 0:
            raise ValueError("Input string must not be empty after minus sign.")
    
    # Check for misplaced minus signs
    if '-' in s:
        raise ValueError("Invalid minus sign position.")
    
    # Check for multiple decimal points
    if s.count('.') > 1:
        raise ValueError("Input contains multiple decimal points.")
    
    # Split into integer and fractional parts
    if '.' in s:
        parts = s.split('.')
        if not parts[0] and not parts[1]:
            raise ValueError("Input string cannot be just a decimal point.")
        integer_part = parts[0] if parts[0] else "0"
        fractional_part = parts[1] if parts[1] else ""
    else:
        integer_part = s
        fractional_part = ""
    
    # LIMIT ENFORCEMENT: 9-digit max for integer part
    if len(integer_part) > MAX_INTEGER_DIGITS:
        raise ValueError(f"Integer part exceeds maximum of {MAX_INTEGER_DIGITS} digits. Got {len(integer_part)} digits.")
    
    # LIMIT ENFORCEMENT: 9-digit max for fractional part
    if len(fractional_part) > MAX_FRACTIONAL_DIGITS:
        raise ValueError(f"Fractional part exceeds maximum of {MAX_FRACTIONAL_DIGITS} digits. Got {len(fractional_part)} digits.")


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
                # LIMIT ENFORCEMENT: Validate input string BEFORE float() parsing
                validate_decimal_input(user_input)
                
                # Try to parse as float to support both integers and decimals
                decimal_value = float(user_input)
                # Convert to int if it's a whole number for cleaner display
                if decimal_value == int(decimal_value):
                    decimal_value = int(decimal_value)
                hex_result = decimal_to_hex(decimal_value)
                print(f"Result: {decimal_value} (Dec) = {hex_result} (Hex)")
            except ValueError as e:
                print(f"Error: {e}")

        elif choice == "2":
            hex_input = input("Enter a hexadecimal string: ").strip()
            try:
                # LIMIT ENFORCEMENT: Validate input string BEFORE conversion
                validate_hex_input(hex_input)
                
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