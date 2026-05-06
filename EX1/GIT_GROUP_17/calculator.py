def decimal_to_hex(decimal_num):
    if decimal_num == 0:
        return "0"

    hex_chars = "0123456789ABCDEF"
    hex_result = ""

    temp_num = abs(decimal_num)

    while temp_num > 0:
        remainder = temp_num % 16
        hex_result = hex_chars[remainder] + hex_result
        temp_num //= 16

    return hex_result if decimal_num >= 0 else "-" + hex_result


def hex_to_decimal(hex_str):
    hex_str = hex_str.strip().upper()
    is_negative = False

    if hex_str.startswith("-"):
        is_negative = True
        hex_str = hex_str[1:]

    hex_map = {
        '0': 0, '1': 1, '2': 2, '3': 3, '4': 4, '5': 5, '6': 6, '7': 7,
        '8': 8, '9': 9, 'A': 10, 'B': 11, 'C': 12, 'D': 13, 'E': 14, 'F': 15
    }

    decimal_value = 0
    power = 0

    for char in reversed(hex_str): #going through the string backwards
        if char not in hex_map:
            raise ValueError(f"{char} is not a valid hex character.")

        decimal_value += hex_map[char] * (16 ** power)
        power += 1

    return -decimal_value if is_negative else decimal_value


def calculator():
    # Loop indefinitely until the user chooses to exit
    while True:
        print("\nBase Converter Calculator")
        print("1. Decimal to Hexadecimal")
        print("2. Hexadecimal to Decimal")
        print("3. Exit")

        choice = input("\nSelect an option (1/2/3): ").strip()

        if choice == '3':
            print("Exiting the calculator. Goodbye!")
            break  # Exit the while loop

        try:
            if choice == '1':
                # Convert string input to integer
                decimal_input = int(input("Enter a decimal integer: "))
                result = decimal_to_hex(decimal_input)
                print(f"Hexadecimal result: {result}")

            elif choice == '2':
                # Hexadecimal is handled as a string
                hex_input = input("Enter a hexadecimal string: ")
                result = hex_to_decimal(hex_input)
                print(f"Decimal result: {result}")

            else:
                print("Invalid choice! Please enter 1, 2, or 3.")

        except ValueError as e:
            # Catches invalid numbers or hex characters
            print(f"Error: {e}")
        except Exception as e:
            # Catches any other unexpected errors
            print(f"An unexpected error occurred: {e}")


if __name__ == "__main__":
    calculator()