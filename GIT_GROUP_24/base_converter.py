def decimal_to_hexadecimal(int_part_str, frac_part_str):
    """
    Core logic for Decimal to Hexadecimal conversion.
    Implements conversion without using built-in functions.
    """
    hex_chars = "0123456789ABCDEF"
    
    # --- Part A: Integer Conversion ---
    int_val = int(int_part_str) 
    hex_int = ""
    
    if int_val == 0:
        hex_int = "0"
    else:
        while int_val > 0:
            remainder = int_val % 16
            hex_int = hex_chars[remainder] + hex_int
            int_val //= 16
            
    # --- Part B: Fraction Conversion ---
    hex_frac = ""
    
    if frac_part_str: 
        frac_val = float("0." + frac_part_str)
        precision_limit = 4 
        
        for _ in range(precision_limit):
            frac_val *= 16
            digit = int(frac_val)
            hex_frac += hex_chars[digit]
            frac_val -= digit
            
            # Avoid infinite loops from floating point inaccuracies
            frac_val = round(frac_val, 10)
            
            if frac_val == 0:
                break
                
    return hex_int, hex_frac


def hexadecimal_to_decimal(int_part_str, frac_part_str):
    """
    Core logic for Hexadecimal to Decimal conversion.
    Implements conversion without using built-in functions.
    """
    hex_chars = "0123456789ABCDEF"
    
    # --- Part A: Integer Conversion ---
    dec_int_val = 0
    for char in int_part_str:
        digit_val = hex_chars.index(char.upper())
        dec_int_val = dec_int_val * 16 + digit_val
        
    dec_int = str(dec_int_val)
    
    # --- Part B: Fraction Conversion ---
    dec_frac = ""
    if frac_part_str:
        dec_frac_val = 0.0
        power = 1
        
        for char in frac_part_str:
            digit_val = hex_chars.index(char.upper())
            power *= 16
            dec_frac_val += digit_val / power
            
        # Format the fraction cleanly to extract only the part after the decimal point
        dec_frac_val = round(dec_frac_val, 10)
        frac_str = str(dec_frac_val)
        if "." in frac_str:
            dec_frac = frac_str.split(".")[1]
            
    return dec_int, dec_frac


def process_and_validate(user_input, base_choice):
    """
    Validates characters, checks 16-bit limits, and splits the input.
    Returns: is_valid (bool), error_msg (str), is_negative (bool), int_part (str), frac_part (str)
    """
    if not user_input:
        return False, "Error: Input cannot be empty.", False, "", ""

    # Negative sign handling
    is_negative = False
    if user_input.startswith('-'):
        is_negative = True
        user_input = user_input[1:]

    # Fraction splitting
    parts = user_input.split('.')
    if len(parts) > 2:
        return False, "Error: Multiple decimal points found.", False, "", ""
    
    int_part = parts[0]
    frac_part = parts[1] if len(parts) > 1 else ""

    # Remove leading zeros for accurate length comparison
    int_part = int_part.lstrip('0')
    if not int_part: 
        int_part = "0"

    # Validation and Bounds Check
    if base_choice == '1': # Decimal Input
        for char in int_part + frac_part:
            if char < '0' or char > '9':
                return False, "Error: Invalid decimal character (only 0-9 allowed).", False, "", ""
        
        # 16-bit Bounds Check (Signed: -32768 to 32767)
        max_val = "32768" if is_negative else "32767"
        if len(int_part) > len(max_val) or (len(int_part) == len(max_val) and int_part > max_val):
            return False, "Error: Number exceeds 16-bit signed decimal limits.", False, "", ""

    elif base_choice == '2': # Hexadecimal Input
        valid_hex = "0123456789ABCDEFabcdef"
        for char in int_part + frac_part:
            if char not in valid_hex:
                return False, "Error: Invalid hexadecimal character.", False, "", ""
        
        # 16-bit Bounds Check (Max 8000 for negative, 7FFF for positive)
        max_val = "8000" if is_negative else "7FFF"
        int_part_upper = int_part.upper()
        if len(int_part_upper) > len(max_val) or (len(int_part_upper) == len(max_val) and int_part_upper > max_val):
            return False, "Error: Number exceeds 16-bit hexadecimal limits.", False, "", ""

    return True, "Success", is_negative, int_part, frac_part


def main():
    print("--- Welcome to the Base Conversion Calculator ---")
    
    while True:
        print("\nPlease select an operation:")
        print("1. Convert from Decimal to Hexadecimal")
        print("2. Convert from Hexadecimal to Decimal")
        print("3. Exit")
        
        choice = input("Enter your choice (1/2/3): ")
        
        if choice == '3':
            print("Exiting the calculator. Goodbye!")
            break
            
        if choice not in ['1', '2']:
            print("Error: Invalid choice. Please select 1, 2, or 3.")
            continue
            
        base_name = "decimal" if choice == '1' else "hexadecimal"
        user_input = input(f"Enter a {base_name} number to convert: ")
        
        is_valid, error_msg, is_negative, int_part, frac_part = process_and_validate(user_input, choice)
        
        if not is_valid:
            print(error_msg)
            continue
            
        # Routing to the correct algorithms
        if choice == '1':
            res_int, res_frac = decimal_to_hexadecimal(int_part, frac_part)
        else:
            res_int, res_frac = hexadecimal_to_decimal(int_part, frac_part)
            
        # Assembling the final result
        final_result = "-" if is_negative else ""
        final_result += res_int
        
        if res_frac: 
            final_result += "." + res_frac
            
        print(f"Result: {final_result}")

if __name__ == "__main__":
    main()