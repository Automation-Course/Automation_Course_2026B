def show_menu():
    """Displays the main menu and returns the user's choice."""
    print("\n--- Base Conversion Calculator ---")
    print("1. Convert Decimal (Base 10) to Hexadecimal (Base 16)")
    print("2. Convert Hexadecimal (Base 16) to Decimal (Base 10)")
    print("3. Exit")

    choice = input("\nPlease select an option (1-3): ")
    return choice


def positive_decimal_to_hex(num):
    # Mapping table for values 10-15 to letters A-F
    hex_chars = "0123456789ABCDEF"

    # Edge case: if the number is 0
    if num == 0:
        return "0"

    hex_result = ""

    while num > 0:
        # 1. Find the remainder when dividing by 16
        remainder = num % 16
        # 2. Get the corresponding hex character and add it to the LEFT
        hex_result = hex_chars[remainder] + hex_result
        # 3. Update the number to be the integer result of division by 16
        num = num // 16

    return hex_result


def decimal_to_hex_with_fraction(num):
    """Converts a decimal number (including fractional parts) to hexadecimal."""
    hex_chars = "0123456789ABCDEF"

    # Separate whole and fractional parts
    whole_part = int(num)
    fractional_part = num - whole_part

    # Get hex for the whole part
    hex_whole = positive_decimal_to_hex(whole_part)

    # If there is no fraction, return only the whole part
    if fractional_part == 0:
        return hex_whole

    # Handle fractional part (Multiplication by 16)
    hex_fractional = ""
    precision = 8  # Limit result to 8 decimal places

    while fractional_part > 0 and precision > 0:
        fractional_part *= 16
        digit = int(fractional_part)  # Get the integer part
        hex_fractional += hex_chars[digit]  # Add corresponding hex char
        fractional_part -= digit  # Keep only the remaining fraction
        precision -= 1

    return f"{hex_whole}.{hex_fractional}"


def get_valid_decimal_input():
    while True:
        user_input = input("\nPlease enter a decimal number: ").strip()

        # Check for empty input
        if not user_input:
            print("Error: Input cannot be empty. Please try again.")
            continue

        # Handle optional minus sign at the beginning
        start_index = 0
        if user_input.startswith('-'):
            # Check if there is anything after the minus sign
            if len(user_input) == 1:
                print("Error: Invalid input. A '-' must be followed by numbers.")
                continue
            start_index = 1

        dot_count = 0
        is_valid = True

        # Check each character (starting after the minus sign, if exists)
        for char in user_input[start_index:]:
            if char == '.':
                dot_count += 1
            elif not ('0' <= char <= '9'):
                is_valid = False
                break

        # Check validation results
        if not is_valid:
            print("Error: Invalid characters! Use numbers (0-9), a decimal point, and an optional '-' at the start.")
        elif dot_count > 1:
            print("Error: Too many decimal points! A number can have only one '.'")
        else:
            # If we reached here, the input is valid
            return user_input

def handle_decimal_to_hex():
    # Get validated decimal input from the user (now supports leading '-')
    clean_input = get_valid_decimal_input()

    # Identify if the input represents a negative number
    is_negative = clean_input.startswith('-')

    # Extract the magnitude (absolute value) as a string
    # We remove the minus sign using replace to process the digits only
    if is_negative:
        abs_input_str = clean_input.replace("-", "")
    else:
        abs_input_str = clean_input

    # Convert the cleaned string to a float for mathematical processing
    num = float(abs_input_str)

    #Perform the mathematical conversion on the positive magnitude
    result = decimal_to_hex_with_fraction(num)

    #Re-attach the minus sign to the hex result string if necessary
    if is_negative:
        final_result = "-" + result
    else:
        final_result = result

    #Display the final conversion results to the user
    print(f"\n>>> Conversion Results:")
    print(f"Decimal: {clean_input}")
    print(f"Hexadecimal: {final_result}")

def positive_hex_to_decimal(hex_str):
    # Mapping to find the value of each hex character
    hex_chars = "0123456789ABCDEF"
    hex_str = hex_str.upper()

    decimal_result = 0
    power = 0

    # Start at the last index of the string
    index = len(hex_str) - 1

    # Run the loop as long as the index is 0 or greater
    while index >= 0:
        # Get the character at the current position
        char = hex_str[index]
        # Get the numeric value (e.g., 'A' -> 10)
        value = hex_chars.find(char)
        # Add (value * 16^power) to the total sum
        decimal_result = decimal_result + (value * (16 ** power))
        # Move the index one step to the LEFT
        index = index - 1
        # Increase the power for the next digit
        power = power + 1

    return decimal_result


def hex_to_decimal_with_fraction(hex_input):
    """Converts a hexadecimal string (including fractional parts) to decimal."""
    hex_chars = "0123456789ABCDEF"
    hex_input = hex_input.upper()

    # Separate whole and fractional parts
    if "." in hex_input:
        parts = hex_input.split(".")
        whole_part = parts[0]
        fractional_part = parts[1]
    else:
        whole_part = hex_input
        fractional_part = ""

    # Convert the whole part to decimal
    whole_decimal = positive_hex_to_decimal(whole_part)

    # If there is no fraction, return the whole decimal part
    if fractional_part == "":
        return whole_decimal

    # Handle fractional part (Summation using negative powers of 16)
    fractional_decimal = 0
    power = -1
    i = 0

    while i < len(fractional_part):
        char = fractional_part[i]
        digit = hex_chars.find(char)

        # Add (value * 16^power) to the sum
        fractional_decimal += digit * (16 ** power)

        power -= 1
        i += 1

    return whole_decimal + fractional_decimal


def get_valid_hex_input():
    while True:
        user_input = input("\nPlease enter a hexadecimal number: ").strip()

        # Check for empty input
        if not user_input:
            print("Error: Input cannot be empty. Please try again.")
            continue

        # Handle optional minus sign at the beginning (Support for negative hex)
        start_index = 0
        if user_input.startswith('-'):
            # Ensure the '-' is followed by actual digits
            if len(user_input) == 1:
                print("Error: Invalid input. A '-' must be followed by digits.")
                continue
            start_index = 1

        dot_count = 0
        is_valid = True

        # Check each character (starting after the minus sign, if it exists)
        for char in user_input[start_index:]:
            if char == '.':
                dot_count += 1
            # Check for valid hex digits (0-9, A-F, a-f)
            elif not (('0' <= char <= '9') or ('a' <= char <= 'f') or ('A' <= char <= 'F')):
                is_valid = False
                break

        # Check validation results
        if not is_valid:
            print(
                "Error: Invalid characters detected! Use numbers (0-9), letters (A-F), and an optional '-' at the start.")
        elif dot_count > 1:
            print("Error: Too many points! A number can have only one '.'")
        else:
            # If we reached here, the input is valid
            return user_input

def handle_hex_to_decimal():
    # Get validated hexadecimal input from the user
    clean_input = get_valid_hex_input()

    # Check if the input represents a negative number
    is_negative = clean_input.startswith('-')

    # Extract the magnitude of the number (absolute value)
    # If a minus sign exists, remove it to process only the digits
    if is_negative:
        abs_input = clean_input.replace("-", "")
    else:
        abs_input = clean_input

    #Perform the mathematical conversion on the positive magnitude
    result = hex_to_decimal_with_fraction(abs_input)

    # Apply the negative sign to the final numerical result if necessary
    if is_negative:
        final_result = -result
    else:
        final_result = result

    # Display the final conversion results to the user
    print(f"\n>>> Conversion Results:")
    print(f"Hexadecimal: {clean_input.upper()}")
    print(f"Decimal: {final_result}")

def main():
    while True:
        user_choice = show_menu()

        if user_choice == '1':
            print("\nOption selected: Decimal to Hexadecimal.")
            handle_decimal_to_hex()

        elif user_choice == '2':
            print("\nOption selected: Hexadecimal to Decimal.")
            handle_hex_to_decimal()

        elif user_choice == '3':
            print("Exiting the program... Goodbye!")
            break

        else:
            print("Invalid selection. Please try again (1, 2, or 3).")


if __name__ == "__main__":
    main()