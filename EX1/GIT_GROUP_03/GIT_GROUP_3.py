# Function to convert Decimal to Hexadecimal (including fractions)
def decimal_to_hex(decimal_num):
    if decimal_num == 0:
        return "0"

    # Separate integer part and fractional part
    integer_part = int(decimal_num)
    fractional_part = decimal_num - integer_part

    hex_chars = "0123456789ABCDEF"

    # 1. Handle integer part (Repeated division)
    res_integer = ""
    temp_int = integer_part
    if temp_int == 0:
        res_integer = "0"
    else:
        while temp_int > 0:
            remainder = temp_int % 16
            res_integer = hex_chars[remainder] + res_integer
            temp_int = temp_int // 16

    # 2. Handle fractional part (Repeated multiplication)
    if fractional_part == 0:
        return res_integer

    res_fraction = ""
    # Limit to 10 decimal places to avoid infinite loops
    precision = 10
    while fractional_part > 0 and len(res_fraction) < precision:
        fractional_part = fractional_part * 16
        digit = int(fractional_part)
        res_fraction = res_fraction + hex_chars[digit]
        fractional_part = fractional_part - digit

    return res_integer + "." + res_fraction

# Function to convert Hexadecimal to Decimal (including fractions)
def hex_to_decimal(hex_string):
    hex_chars = "0123456789ABCDEF"
    hex_string = hex_string.strip().upper()

    if not hex_string:
        return "Error: Input cannot be empty"
    if hex_string.startswith('-'):
        return "Error: Negative numbers are not supported"

    # Split into integer and fraction parts by the dot
    if "." in hex_string:
        parts = hex_string.split(".")
        if len(parts) > 2:
            return "Error: Invalid format (too many dots)"
        int_str = parts[0]
        frac_str = parts[1]
    else:
        int_str = hex_string
        frac_str = ""

    decimal_value = 0.0

    # 1. Convert integer part (Positive powers of 16)
    power = 0
    for char in reversed(int_str):
        if char not in hex_chars:
            return f"Error: '{char}' is not a valid Hex digit"
        decimal_value = decimal_value + (hex_chars.find(char) * (16 ** power))
        power = power + 1

    # 2. Convert fractional part (Negative powers of 16)
    power = -1
    for char in frac_str:
        if char not in hex_chars:
            return f"Error: '{char}' is not a valid Hex digit"
        decimal_value = decimal_value + (hex_chars.find(char) * (16 ** power))
        power = power - 1

    return decimal_value

# Helper function for y/n validation
def ask_yes_no(question):
    while True:
        answer = input(question).strip().lower()
        if answer == 'y': return True
        if answer == 'n': return False
        print("Invalid input. Please enter 'y' or 'n'.")

# --- Main Loop ---
def main():
    while True:
        print("\n--- Advanced Base Converter (Supports Decimals) ---")
        print("1. Decimal to Hexadecimal")
        print("2. Hexadecimal to Decimal")
        print("Type 'exit' to close")

        choice = input("\nEnter choice: ").strip().lower()
        if choice == 'exit': break

        if choice == '1':
            while True:
                user_input = input("\nEnter a positive decimal or 'menu': ").strip().lower()
                if user_input == 'menu': break
                if user_input == 'exit': return
                try:
                    num = float(user_input) # Changed from int to float
                    if num < 0:
                        print("Error: Negative numbers not supported.")
                        continue
                    print(f"Result: {decimal_to_hex(num)}")
                    if not ask_yes_no("Another decimal conversion? (y/n): "): break
                except ValueError:
                    print("Error: Please enter a valid number.")

        elif choice == '2':
            while True:
                user_input = input("\nEnter a positive Hex or 'menu': ").strip().lower()
                if user_input == 'menu': break
                if user_input == 'exit': return
                result = hex_to_decimal(user_input)
                if isinstance(result, str) and "Error" in result:
                    print(result)
                else:
                    print(f"Result: {result}")
                    if not ask_yes_no("Another hex conversion? (y/n): "): break
        else:
            print("Invalid choice.")

if __name__ == "__main__":
    main()

