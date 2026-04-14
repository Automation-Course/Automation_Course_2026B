"""Base Conversion Calculator (terminal-based).

Supports:
1) Decimal to Hexadecimal
2) Hexadecimal to Decimal
"""

def _is_all_zero_digits(digits):
    """True if the digit-only string represents 0 (empty counts as 0 for our usage)."""
    return digits == "" or all(c == "0" for c in digits)

def _normalize_decimal_for_output(is_negative, whole_str, frac_str):
    """Normalize decimal display: trim leading zeros and trailing fractional zeros."""
    whole_trim = whole_str.lstrip("0")
    if whole_trim == "":
        whole_trim = "0"
    frac_trim = frac_str.rstrip("0") if frac_str != "" else ""

    negative_effective = is_negative and not (whole_trim == "0" and frac_trim == "")
    prefix = "-" if negative_effective else ""

    if frac_trim == "":
        return prefix + whole_trim
    return prefix + whole_trim + "." + frac_trim

def value_to_hex_char(value):
    """Map an integer 0-15 to its hexadecimal character."""
    if 0 <= value <= 9:
        return chr(ord("0") + value)
    return chr(ord("A") + (value - 10))

def hex_char_to_value(char):
    """Map a hexadecimal character (0-9, A-F) to its integer value."""
    if "0" <= char <= "9":
        return ord(char) - ord("0")
    return ord(char) - ord("A") + 10

def validate_decimal_input(raw_text):
    text = raw_text.strip()
    if text == "":
        return False, "Decimal input cannot be empty."

    if text.count("-") > 1 or ("-" in text and not text.startswith("-")):
        return False, "Minus sign is only allowed once, at the beginning."
    if text == "-":
        return False, "Decimal input cannot be just a minus sign."
    if text.count(".") > 1:
        return False, "Decimal input may contain at most one decimal point."

    unsigned = text[1:] if text.startswith("-") else text
    if "." in unsigned:
        left, right = unsigned.split(".", 1)
        if left == "" or right == "":
            return False, "If a decimal point is used, there must be digits before and after it."
        if not left.isdigit() or not right.isdigit():
            return False, "Decimal input must contain digits only (0-9), with an optional leading '-' and one '.'"
    else:
        if not unsigned.isdigit():
            return False, "Decimal input must contain digits only (0-9), with an optional leading '-'"

    is_negative = text.startswith("-")
    if "." in unsigned:
        whole_str, frac_str = unsigned.split(".", 1)
    else:
        whole_str, frac_str = unsigned, ""
    return True, (is_negative, whole_str, frac_str)

def validate_hex_input(raw_text):
    text = raw_text.strip()
    if text == "":
        return False, "Hexadecimal input cannot be empty."

    if text.count("-") > 1 or ("-" in text and not text.startswith("-")):
        return False, "Minus sign is only allowed once, at the beginning."
    if text == "-":
        return False, "Hexadecimal input cannot be just a minus sign."
    if text.count(".") > 1:
        return False, "Hexadecimal input may contain at most one decimal point."

    is_negative = text.startswith("-")
    unsigned = text[1:] if is_negative else text
    unsigned = unsigned.upper()

    if "." in unsigned:
        left, right = unsigned.split(".", 1)
        if left == "" or right == "":
            return False, "If a decimal point is used, there must be hex digits before and after it."
        allowed = "0123456789ABCDEF"
        for char in left + right:
            if char not in allowed:
                return False, "Hexadecimal input must use only 0-9 and A-F, with an optional leading '-' and one '.'"
        return True, (is_negative, left, right)

    allowed = "0123456789ABCDEF"
    for char in unsigned:
        if char not in allowed:
            return False, "Hexadecimal input must use only 0-9 and A-F, with an optional leading '-'"
    return True, (is_negative, unsigned, "")

def decimal_to_hex_manual(is_negative, whole_str, frac_str):
    """
    Convert validated decimal parts to hexadecimal.
    Uses:
    - repeated division by 16 for the whole part
    - repeated multiplication by 16 for the fractional part (max 8 digits)
    """
    whole_value = int(whole_str)
    frac_value = int(frac_str) if frac_str != "" else 0
    negative_effective = is_negative and not (whole_value == 0 and frac_value == 0)
    # Whole-number part: repeated division by 16.
    if whole_value == 0:
        whole_hex = "0"
    else:
        remainders = []
        current = whole_value
        while current > 0:
            remainder = current % 16
            remainders.append(value_to_hex_char(remainder))
            current = current // 16
        whole_hex = "".join(reversed(remainders))
    # Fractional part: repeated multiplication by 16 (max 8 digits).
    if frac_str == "" or frac_value == 0:
        return "-" + whole_hex if negative_effective else whole_hex
    max_digits = 8
    denom = 10 ** len(frac_str)
    # Use float multiplication for fractional digit extraction (manual algorithm, no base shortcuts).
    # This matches the required truncation/ellipsis behavior expected in the assignment.
    remainder = frac_value  # integer only, no float
    frac_digits = []
    for _ in range(max_digits):
        if remainder == 0:
            break
        remainder *= 16
        digit = remainder // denom
        remainder = remainder % denom
        frac_digits.append(value_to_hex_char(digit))
    truncated = len(frac_digits) == max_digits and remainder != 0
    # Trim fractional trailing zeros only for non-truncated (terminating) results.
    if not truncated:
        while frac_digits and frac_digits[-1] == "0":
            frac_digits.pop()

    if frac_digits:
        frac_out = "".join(frac_digits)
        if truncated:
            frac_out = frac_out + "..."
        result = f"{whole_hex}.{frac_out}"
    else:
        result = whole_hex

    return "-" + result if negative_effective else result

def hex_to_decimal_manual(is_negative, whole_str, frac_str):
    """Convert validated hexadecimal parts to a decimal string."""
    # Whole-number part (positional algorithm): sum digit_value * 16^position.
    whole_value = 0
    power = 1
    for char in reversed(whole_str):
        digit_value = hex_char_to_value(char)
        whole_value += digit_value * power
        power *= 16
    # Fractional part: represent it as an integer numerator over 16^k,
    # then output decimal digits via manual long division.
    frac_numerator = 0
    for char in frac_str:
        frac_numerator = frac_numerator * 16 + hex_char_to_value(char)

    negative_effective = is_negative and not (whole_value == 0 and frac_numerator == 0)
    if frac_str == "" or frac_numerator == 0:
        return "-" + str(whole_value) if negative_effective else str(whole_value)

    denom = 16 ** len(frac_str)
    remainder = frac_numerator
    safety_limit = 10
    decimal_digits = []

    for _ in range(safety_limit):
        if remainder == 0:
            break
        remainder *= 10
        digit = remainder // denom
        remainder = remainder % denom
        decimal_digits.append(chr(ord("0") + digit))

    truncated = len(decimal_digits) == safety_limit and remainder != 0

    if not truncated:
        while decimal_digits and decimal_digits[-1] == "0":
            decimal_digits.pop()

    if not decimal_digits:
        result = str(whole_value)
    else:
        frac_out = "".join(decimal_digits)
        if truncated:
            frac_out = frac_out + "..."
        result = f"{whole_value}.{frac_out}"

    return "-" + result if negative_effective else result

def build_hex_display(is_negative, whole_str, frac_str):
    """Build canonical uppercase hex string for the output message (keep leading zeros)."""
    whole_zero = _is_all_zero_digits(whole_str)
    frac_zero = _is_all_zero_digits(frac_str)
    negative_effective = is_negative and not (whole_zero and frac_zero)
    prefix = "-" if negative_effective else ""
    if frac_str != "":
        return f"{prefix}{whole_str}.{frac_str}"
    return f"{prefix}{whole_str}"

def show_menu():
    print("\nBase Conversion Calculator")
    print("1. Decimal to Hexadecimal")
    print("2. Hexadecimal to Decimal")
    print("3. Exit")

def handle_decimal_to_hex_flow():
    user_input = input("Enter a decimal number (supports '-' and fractional part): ")
    is_valid, result = validate_decimal_input(user_input)

    if not is_valid:
        print(f"Invalid input: {result}")
        return

    is_negative, whole_str, frac_str = result
    hex_value = decimal_to_hex_manual(is_negative, whole_str, frac_str)
    decimal_display = _normalize_decimal_for_output(is_negative, whole_str, frac_str)
    print(f"Result: {decimal_display} (base 10) = {hex_value} (base 16)")

def handle_hex_to_decimal_flow():
    user_input = input("Enter a hexadecimal value (0-9, A-F, supports '-' and fractional part): ")
    is_valid, result = validate_hex_input(user_input)

    if not is_valid:
        print(f"Invalid input: {result}")
        return

    is_negative, whole_str, frac_str = result
    hex_value = build_hex_display(is_negative, whole_str, frac_str)
    decimal_value = hex_to_decimal_manual(is_negative, whole_str, frac_str)
    print(f"Result: {hex_value} (base 16) = {decimal_value} (base 10)")

def main():
    while True:
        show_menu()
        choice = input("Choose an option (1-3): ").strip()

        if choice == "1":
            handle_decimal_to_hex_flow()
        elif choice == "2":
            handle_hex_to_decimal_flow()
        elif choice == "3":
            print("Goodbye!")
            break
        else:
            print("Invalid menu selection. Please choose 1, 2, or 3.")

if __name__ == "__main__":
    main()