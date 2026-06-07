
def hex_to_dec(hex_string): #Convert from hexadecimal to decimal
    hex_string = hex_string.upper()  # Convert to uppercase
    # Handle negative numbers
    is_negative = False
    if hex_string.startswith('-'):
        is_negative = True
        hex_string = hex_string[1:]  # Remove the minus sign for calculation

    decimal_val = 0
    power = 0
    # Helper Mapping: Defines the value of each hex character (0-15)
    hexadecimal_digits = "0123456789ABCDEF"

    if not hex_string:
        return "INVALID"

    # Iterate over the string from end to start
    for char in reversed(hex_string):
        if char not in hexadecimal_digits:
            return "INVALID"

        # Get the numeric value of the character
        digit_value = hexadecimal_digits.index(char)

        # Formula: value multiplied by 16 to the power of position
        decimal_val += digit_value * (16 ** power)
        power += 1

    # Apply the negative sign if needed
    return -decimal_val if is_negative else decimal_val

def dec_to_hex(decimal_num): #Convert from decimal to hexadecimal
    if decimal_num == 0:
        return "0"

    #Handle negative numbers
    is_negative = False
    if decimal_num < 0:
        is_negative = True
        decimal_num = abs(decimal_num)  # Work with the absolute value

    #Helper Mapping: Defines the value of each hex character (0-15)
    hexadecimal_digits = "0123456789ABCDEF"
    hex_result = ""

    while decimal_num > 0:
        remainder = decimal_num % 16  # Get the remainder
        hex_result = hexadecimal_digits[remainder] + hex_result  # Add the corresponding character to the beginning
        decimal_num = decimal_num // 16  # Integer division to continue with the next number

    # Add the minus sign back if the original number was negative
    return "-" + hex_result if is_negative else hex_result


#  Program execution
if __name__ == "__main__":
    while True:
        print("\n" + "=" * 25)
        print("--- תפריט מחשבון בסיסים ---")
        print("1. המרה מעשרוני להקסדצימלי")
        print("2. המרה מהקסדצימלית לעשרוני")
        print("3. יציאה (Exit)")
        print("=" * 25)
        choice = input("בחרי אופציה (1-3): ").strip()

        # Option 1: Decimal To Hex
        if choice == '1':
            user_input = input("הכניסי מספר עשרוני שלם: ").strip()
            try:
                # int() will raise an error for invalid input (including non-numeric or decimal values)
                val = int(user_input)
                print(f"התוצאה בהקסדצימלית: {dec_to_hex(val)}")
            except ValueError:
                print("INVALID")
                # Automatically returns to the main menu

        # Option 2: Hex To Decimal
        elif choice == '2':
            hex_input = input("הכניסי מספר הקסדצימלי שלם: ").strip()
            result = hex_to_dec(hex_input)
            if result == "INVALID":
                print("INVALID")
            else:
                print(f"התוצאה בעשרוני: {result}")

        # Option 3: Exit
        elif choice == '3' or choice.lower() == 'q':
            print("להתראות!")
            break

        # If the user enters an unrelated value (e.g., '4' or a letter)
        else:
            print("INVALID")