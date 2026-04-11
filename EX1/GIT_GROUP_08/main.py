def decimal_digit_to_char(value):
    """Converts a value from 0-15 to its hexadecimal character."""
    hex_chars = "0123456789ABCDEF"
    return hex_chars[value]

def hex_char_to_decimal(char):
    """Converts a hexadecimal character to its decimal value."""
    hex_chars = "0123456789ABCDEF"
    upper_char = char.upper()
    for index in range(len(hex_chars)):
        if hex_chars[index] == upper_char:
            return index
    return -1

def is_valid_decimal_input(user_input):
    """ Cheak if the input is a valid decimal number """
    if user_input == "":
        return False
    if " " in user_input:
        return False
    
    minus_count = 0
    dot_count = 0
    for i in range(len(user_input)):
        char = user_input[i]

        if char == "-":
            minus_count += 1
            if i != 0:
                return False
            if minus_count > 1:
                return False
        elif char == ".":
            dot_count += 1
            if dot_count > 1:
                return False
        elif char < "0" or char > "9":
            return False
    if user_input == "-" or user_input == "." or user_input == "-.":
        return False
    return True

def is_valid_hex_input(user_input):
    """Cheak if the input is a valid hexadecimal number"""
    if user_input == "":
        return False
    if " " in user_input:
        return False
    minus_count = 0
    dot_count = 0
    for i in range(len(user_input)):
        char = user_input[i]
        if char == "-":
            minus_count += 1
            if i != 0:
                return False
            if minus_count > 1:
                return False
        elif char == ".":
            dot_count += 1
            if dot_count > 1:
                return False
        else:
            value = hex_char_to_decimal(char)
            if value == -1:
                return False
    if user_input == "-" or user_input == "." or user_input == "-.":
        return False
    return True

def decimal_string_to_int(decimal_str):
    """Converts a decimal string without a point into an integer manually."""
    if decimal_str == "":
        return 0

    result = 0
    for i in range(len(decimal_str)):
        digit_value = ord(decimal_str[i]) - ord("0")
        result = result * 10 + digit_value

    return result

def decimal_fraction_to_float(fraction_str):
    """ Converts the fractional part of a decimal number manually."""
    result = 0.0
    divisor = 10.0

    for i in range(len(fraction_str)):
        digit_value = ord(fraction_str[i]) - ord("0")
        result += digit_value / divisor
        divisor *= 10.0

    return result

def decimal_integer_to_hex(integer_number):
    """ Converts a non-negative integer to hexadecimal manually."""
    if integer_number == 0:
        return "0"

    hex_result = ""

    while integer_number > 0:
        remainder = integer_number % 16
        hex_result = decimal_digit_to_char(remainder) + hex_result
        integer_number = integer_number // 16

    return hex_result

def decimal_to_hex(decimal_str):
    """Converts a decimal number string (integer or fractional) to hexadecimal manually.
    Supports negative numbers."""
    is_negative = False

    if decimal_str[0] == "-":
        is_negative = True
        decimal_str = decimal_str[1:]

    if "." in decimal_str:
        parts = decimal_str.split(".")
        integer_part_str = parts[0]
        fraction_part_str = parts[1]
    else:
        integer_part_str = decimal_str
        fraction_part_str = ""

    if integer_part_str == "":
        integer_part_str = "0"

    integer_part = decimal_string_to_int(integer_part_str)
    fraction_part = decimal_fraction_to_float(fraction_part_str)

    hex_integer_part = decimal_integer_to_hex(integer_part)

    hex_fraction_part = ""
    max_digits_after_point = 10
    count = 0

    while fraction_part > 0 and count < max_digits_after_point:
        fraction_part *= 16
        digit = 0

        while fraction_part >= 1:
            fraction_part -= 1
            digit += 1

        hex_fraction_part += decimal_digit_to_char(digit)
        count += 1

    if hex_fraction_part != "":
        result = hex_integer_part + "." + hex_fraction_part
    else:
        result = hex_integer_part

    if is_negative and result != "0":
        result = "-" + result

    return result

def hex_integer_to_decimal(integer_str):
    """ Converts a hexadecimal integer string to decimal integer manually."""
    result = 0

    for i in range(len(integer_str)):
        digit_value = hex_char_to_decimal(integer_str[i])
        result = result * 16 + digit_value

    return result

def hex_fraction_to_decimal(fraction_str):
    """ Converts the fractional part of a hexadecimal number manually. """
    result = 0.0
    divisor = 16.0

    for i in range(len(fraction_str)):
        digit_value = hex_char_to_decimal(fraction_str[i])
        result += digit_value / divisor
        divisor *= 16.0

    return result

def float_to_string(number, max_digits_after_point=10):
    """Converts a number to string. """
    if number == 0:
        return "0"
    is_negative = False
    if number < 0:
        is_negative = True
        number = -number
    integer_part = int(number)
    fraction_part = number - integer_part
    integer_str = str(integer_part)
    if fraction_part == 0:
        result = integer_str
    else:
        fraction_str = ""
        count = 0
        while fraction_part > 0 and count < max_digits_after_point:
            fraction_part *= 10
            digit = int(fraction_part)
            fraction_str += chr(ord("0") + digit)
            fraction_part -= digit
            count += 1

        while len(fraction_str) > 0 and fraction_str[-1] == "0":
            fraction_str = fraction_str[:-1]
        if fraction_str == "":
            result = integer_str
        else:
            result = integer_str + "." + fraction_str
    if is_negative:
        result = "-" + result
    return result

def hex_to_decimal(hex_str):
    """Converts a hexadecimal number string (integer or fractional) to decimal manually.Supports negative numbers. """
    is_negative = False
    if hex_str[0] == "-":
        is_negative = True
        hex_str = hex_str[1:]
    if "." in hex_str:
        parts = hex_str.split(".")
        integer_part_str = parts[0]
        fraction_part_str = parts[1]
    else:
        integer_part_str = hex_str
        fraction_part_str = ""
    if integer_part_str == "":
        integer_part_str = "0"
    integer_part = hex_integer_to_decimal(integer_part_str)
    fraction_part = hex_fraction_to_decimal(fraction_part_str)
    result_number = integer_part + fraction_part
    if is_negative:
        result_number = -result_number
    return float_to_string(result_number)

def print_menu():
    print("Base Conversion Calculator:")
    print("0 - Convert hexadecimal to decimal")
    print("1 - Convert decimal to hexadecimal")
    print("2 - Exit")

def main():
    while True:
        print_menu()
        choice = input("Enter your choice: ")
        if choice == "2":
            print("Program ended.")
            break
        elif choice == "0":
            while True:
                hex_input = input("Enter a hexadecimal number: ")
                if not is_valid_hex_input(hex_input):
                    print("Error: Invalid hexadecimal input.Try again.")
                    print("Allowed input: optional '-' only at the beginning,")
                    print("digits 0-9, letters A-F/a-f,")
                    print("optional single decimal point, no spaces.")
                else:
                    decimal_result = hex_to_decimal(hex_input)
                    print("Decimal result:", decimal_result)
                    break
        elif choice == "1":
            while True:
                decimal_input = input("Enter a decimal number: ")
                if not is_valid_decimal_input(decimal_input):
                    print("Error: Invalid decimal input.")
                    print("Allowed input: optional '-' only at the beginning,")
                    print("digits 0-9, optional single decimal point, no spaces.")
                else:
                    hex_result = decimal_to_hex(decimal_input)
                    print("Hexadecimal result:", hex_result)
                    break
        else:
            print("Error: Invalid menu choice. Please enter 0, 1, or 2 (no spaces).")

if __name__ == "__main__":
    main()