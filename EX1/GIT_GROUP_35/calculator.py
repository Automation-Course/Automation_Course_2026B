import math

def hexadecimal_to_decimal(hex_str):
    """
    Converts a hexadecimal string to its decimal equivalent.
    """
    # Basic cleaning and empty check
    hex_str = hex_str.strip().upper()
    if not hex_str:
        return "Error! Input is empty"

    # Handle negative numbers
    is_negative = False
    if hex_str.startswith("-"):
        is_negative = True
        hex_str = hex_str[1:]
       # Case where input was just "-"
        if not hex_str:
            return "Error! Invalid input"

    # Handle multiple decimal points
    if hex_str.count(".") > 1:
        return "Error! Invalid number: more than one decimal point"

    # Split into whole and fractional parts
    if "." in hex_str:
        parts = hex_str.split(".")
        whole_part = parts[0]
        fractional_part = parts[1]
    else:
        whole_part = hex_str
        fractional_part = ""

    # Handle cases like ".A" by treating empty whole part as "0"
    if not whole_part:
        whole_part = "0"

    hex_chars = "0123456789ABCDEF"
    decimal_whole = 0
    decimal_fraction = 0

    # Calculate whole part 
    for i, char in enumerate(reversed(whole_part)):
        if char not in hex_chars:
            return f"Error! Invalid character '{char}'"
        value = hex_chars.find(char)
        decimal_whole += value * (16 ** i)

    # Calculate fractional part 
    for i, char in enumerate(fractional_part):
        if char not in hex_chars:
            return f"Error! Invalid character '{char}'"
        value = hex_chars.find(char)
        decimal_fraction += value * (16 ** -(i + 1))

    # Combine and apply sign
    final_result = decimal_whole + decimal_fraction
    return -final_result if is_negative else final_result


def decimal_to_hexadecimal(decimal_num, precision=4):
    """
    Converts a decimal number to hexadecimal.
    """
    # Handle negative numbers
    is_negative = False
    if decimal_num < 0:
        is_negative = True
        decimal_num = abs(decimal_num)
    
    # Split into whole and fractional parts
    whole_part = int(decimal_num)
    fractional_part = decimal_num - whole_part
    
    hex_chars = "0123456789ABCDEF"
    
    # Convert whole part
    if whole_part == 0:
        res_whole = "0"
    else:
        res_whole = ""
        temp_whole = whole_part
        while temp_whole > 0:
            remainder = temp_whole % 16
            res_whole = hex_chars[remainder] + res_whole
            temp_whole //= 16
            
    # Convert fractional part
    res_fraction = ""
    temp_frac = fractional_part
    
    # limit precision to prevent infinite loops
    for _ in range(precision):
        if temp_frac == 0:
            break
        
        temp_frac *= 16
        digit = int(temp_frac) 
        res_fraction += hex_chars[digit]
        temp_frac -= digit 
        
    # Combine results
    final_hex = res_whole
    if res_fraction:
        final_hex += "." + res_fraction
        
    return "-" + final_hex if is_negative else final_hex

def run_tests():
    """
    Automated test suite to verify the logic for various inputs.
    Updated to handle float/int comparison differences.
    """
    print("\n" + "="*50)
    print("   Group GIT_GROUP_35: Automated Testing Suite   ")
    print("="*50)

    test_cases = [
        (1, 255, "FF", "Integer Dec to Hex"),
        (1, 26.9375, "1A.F", "Fractional Dec to Hex"),
        (1, -10, "-A", "Negative Dec to Hex"),
        (2, "FF", 255, "Integer Hex to Dec"),
        (2, "1A.F", 26.9375, "Fractional Hex to Dec"),
        (2, "-A", -10, "Negative Hex to Dec"),
        (2, ".A", 0.625, "Point-start Hex to Dec"),
        (2, "1.2.3", "Error", "Invalid Input (Multiple Dots)")
    ]

    passed_count = 0
    for mode, inp, expected, name in test_cases:
        if mode == 1:
            actual = decimal_to_hexadecimal(inp)
        else:
            actual = hexadecimal_to_decimal(inp)
        
        # Convert both to string for comparison or handle numeric equality
        is_match = False
        if expected == "Error":
            is_match = "Error" in str(actual)
        else:
            # Check numeric equality
            try:
                is_match = float(actual) == float(expected)
            except:
                is_match = str(actual) == str(expected)

        if is_match:
            status = "[V] PASS"
            passed_count += 1
        else:
            status = f"[X] FAIL (Expected: {expected}, Got: {actual})"
            
        print(f"{name:<30} : {status}")

    print("="*50)
    print(f" FINAL SUMMARY: {passed_count}/{len(test_cases)} tests passed.")
    print("="*50 + "\n")

def main():
    """
    Main execution loop providing a menu for the user.
    """
    while True:
        print("\n--- Group GIT_GROUP_35: Hex <-> Decimal Converter ---")
        print("1. Hexadecimal to Decimal")
        print("2. Decimal to Hexadecimal")
        print("3. Run Automated System Tests")
        print("0. Exit")
        
        choice = input("\nSelect option: ").strip()
        
        if choice == '0':
            print("Goodbye :)")
            break
            
        try:
            if choice == '1':
                hex_input = input("Enter Hexadecimal: ")
                result = hexadecimal_to_decimal(hex_input)
                print(f"Result (Decimal): {result}")
                
            elif choice == '2':
                dec_input = input("Enter Decimal: ")
                num = float(dec_input) # Validate numeric input
                result = decimal_to_hexadecimal(num)
                print(f"Result (Hexadecimal): {result}")
                
            elif choice == '3':
                # Trigger the testing suite
                run_tests() 
                
            else:
                print("Invalid choice. Please select 0, 1, 2 or 3.")
        except ValueError:
            print("Error: Please enter a valid numerical value.")

if __name__ == "__main__":
    main()