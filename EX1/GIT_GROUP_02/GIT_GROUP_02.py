# Calculator for conversions between base 10 and base 16

# Function to check if a string is a valid decimal number
def is_valid_decimal(s):
    # Empty input is not valid
    if s == "":
        return False

    # Handle optional negative sign
    if s[0] == "-":
        s = s[1:]
    if s == "":
        return False

    dot_count = 0
    for ch in s:
        if ch == ".":
            dot_count += 1
        elif ch < "0" or ch > "9":
            return False

    # Valid only if there is at most one decimal point
    return dot_count <= 1


# Function to check if a string is a valid hexadecimal number
def is_valid_hex(s):
    # Empty input is not valid
    if s == "":
        return False

    # Handle optional negative sign
    if s[0] == "-":
        s = s[1:]
    if s == "":
        return False

    dot_count = 0
    for ch in s:
        if ch == ".":
            dot_count += 1
        elif not (("0" <= ch <= "9") or ("A" <= ch <= "F") or ("a" <= ch <= "f")):
            return False

    # Valid only if there is at most one decimal point
    return dot_count <= 1


# Convert a hexadecimal character to its numeric value
def hex_char_to_value(ch):
    if "0" <= ch <= "9":
        return ord(ch) - ord("0")
    elif "A" <= ch <= "F":
        return ord(ch) - ord("A") + 10
    else:  # a-f
        return ord(ch) - ord("a") + 10


# Convert a numeric value (0-15) to a hexadecimal character
def value_to_hex_char(value):
    if 0 <= value <= 9:
        return chr(ord("0") + value)
    else:
        return chr(ord("A") + (value - 10))


# Convert decimal (base 10) string to hexadecimal (base 16)
def decimal_to_hex(decimal_str):
    if not is_valid_decimal(decimal_str):
        return "Error: Invalid decimal input"

    is_negative = False
    if decimal_str[0] == "-":
        is_negative = True
        decimal_str = decimal_str[1:]

    parts = decimal_str.split(".")
    int_part_str = parts[0] if parts[0] != "" else "0"

    # Convert integer part manually
    int_num = 0
    for ch in int_part_str:
        digit = ord(ch) - ord("0")
        int_num = int_num * 10 + digit

    hex_int_part = ""
    if int_num == 0:
        hex_int_part = "0"
    else:
        temp = int_num
        while temp > 0:
            remainder = temp % 16
            hex_int_part = value_to_hex_char(remainder) + hex_int_part
            temp = temp // 16

    # Convert fractional part manually (up to 5 digits precision)
    hex_frac_part = ""
    if len(parts) > 1 and parts[1] != "":
        frac_val = 0.0
        divisor = 10.0
        # Convert string fraction to float mathematically
        for ch in parts[1]:
            frac_val += (ord(ch) - ord("0")) / divisor
            divisor *= 10.0

        # Multiply by 16 up to 5 times to get hex digits
        for _ in range(5):
            frac_val *= 16
            digit = int(frac_val)
            hex_frac_part += value_to_hex_char(digit)
            frac_val -= digit

    # Remove trailing zeros from the fractional part
    hex_frac_part = hex_frac_part.rstrip("0")

    # Build final result string
    if hex_frac_part == "":
        result = hex_int_part
    else:
        result = hex_int_part + "." + hex_frac_part

    if is_negative and result != "0":
        result = "-" + result

    return result


# Convert hexadecimal (base 16) string to decimal (base 10)
def hex_to_decimal(hex_str):
    if not is_valid_hex(hex_str):
        return "Error: Invalid hexadecimal input"

    is_negative = False
    if hex_str[0] == "-":
        is_negative = True
        hex_str = hex_str[1:]

    parts = hex_str.split(".")
    int_part_str = parts[0] if parts[0] != "" else "0"

    # Convert integer part manually
    int_num = 0
    for ch in int_part_str:
        value = hex_char_to_value(ch)
        int_num = int_num * 16 + value

    # Convert fractional part manually
    frac_num = 0.0
    if len(parts) > 1 and parts[1] != "":
        multiplier = 16.0
        for ch in parts[1]:
            frac_num += hex_char_to_value(ch) / multiplier
            multiplier *= 16.0

    total_val = int_num + frac_num

    # Format up to 5 digits after the dot
    result_str = "{:.5f}".format(total_val)

    # Remove trailing zeros and trailing dot if it becomes an integer
    if "." in result_str:
        result_str = result_str.rstrip("0").rstrip(".")

    if is_negative and total_val != 0:
        result_str = "-" + result_str

    return result_str


# Calculator menu
def calculator():
    while True:
        print("\n--- Base Conversion Calculator (Base 10 ↔ Base 16) ---")
        print("1 - Convert Decimal to Hexadecimal")
        print("2 - Convert Hexadecimal to Decimal")
        print("3 - Exit")

        choice = input("Choose an option: ")

        if choice == "1":
            decimal_input = input("Enter a decimal number: ")
            result = decimal_to_hex(decimal_input)
            print("Result:", result)

        elif choice == "2":
            hex_input = input("Enter a hexadecimal number: ")
            result = hex_to_decimal(hex_input)
            print("Result:", result)

        elif choice == "3":
            print("Exiting program.")
            break

        else:
            print("Error: Please choose 1, 2, or 3 only.")


# Run tests
if __name__ == "__main__":
    # run_tests()
    calculator()
