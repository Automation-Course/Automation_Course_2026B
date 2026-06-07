

def fromDecToHex(n_str):
    hex_chars = "0123456789ABCDEF"
    
    # Handle negative sign
    is_negative = n_str.startswith('-')
    if is_negative:
        n_str = n_str[1:]
    
    # Split into integer and fractional parts
    if '.' in n_str:
        parts = n_str.split('.')
        int_part = int(parts[0]) if parts[0] else 0
        frac_part = float("0." + parts[1]) if parts[1] else 0.0
    else:
        int_part = int(n_str)
        frac_part = 0.0
        
    # Convert Integer Part (Successive Division)
    hex_int = ""
    temp_int = int_part
    if temp_int == 0:
        hex_int = "0"
    while temp_int > 0:
        hex_int = hex_chars[temp_int % 16] + hex_int
        temp_int //= 16
        
    # Convert Fractional Part (Successive Multiplication)
    hex_frac = ""
    temp_frac = frac_part
    for _ in range(10): # 10 digits precision
        if temp_frac == 0:
            break
        temp_frac *= 16
        digit = int(temp_frac)
        hex_frac += hex_chars[digit]
        temp_frac -= digit
        
    result = hex_int + ("." + hex_frac if hex_frac else "")
    return "-" + result if is_negative else result


def fromHexToDec(n_str):
    hex_map = {c: i for i, c in enumerate("0123456789ABCDEF")}
    n_str = n_str.upper()
    
    is_negative = n_str.startswith('-')
    if is_negative:
        n_str = n_str[1:]
        
    if '.' in n_str:
        parts = n_str.split('.')
        int_str, frac_str = parts[0], parts[1]
    else:
        int_str, frac_str = n_str, ""
        
    # Integer part conversion: sum of (digit * 16^i)
    dec_int = 0
    for i, char in enumerate(reversed(int_str)):
        dec_int += hex_map[char] * (16 ** i)
        
    # Fractional part conversion: sum of (digit * 16^-i)
    dec_frac = 0.0
    for i, char in enumerate(frac_str):
        dec_frac += hex_map[char] * (16 ** (-(i + 1)))
        
    final_val = dec_int + dec_frac
    if is_negative:
        final_val = -final_val
        
    if dec_frac == 0:
        return str(int(final_val))
    # Return with 10 decimal places, stripping trailing zeros
    return "{:.10f}".format(final_val).rstrip('0').rstrip('.')


def main():
    while True:
        print("\n--- Hello there! Welcome to the Best Base Converter Calculator to ever exist ---")
        print("Enter 1 for Decimal to Hexadecimal")
        print("Enter 2 for Hexadecimal to Decimal")
        print("Enter 3 for Exit")
        
        choice = input("Enter your choice (1/2/3): ").strip()
        
        if choice == '3':
            print("Until next time!")
            break
            
        if choice not in ['1', '2']:
            print("Invalid input, please choose an alternative - 1/2/3")
            continue
            
        num_input = input("Enter the number: ") # No strip here to detect spaces
        
        # Validation Rules
        # 1. Length check (max 16 chars)
        if len(num_input) > 16 or len(num_input) == 0:
            print("Invalid input, please enter a number between 1-16 chars long")
            continue
            
        # 2. Space check
        if ' ' in num_input:
            print("Invalid input, cannot enter the char space")
            continue

        # 3. Sign and Format check
        # Only '-' allowed at the start. '+' is forbidden.
        if num_input.startswith('+'):
            print("Invalid input, + is not allowed")
            continue
            
        # 4. Character and logical check
        valid_chars = ""
        if choice == '1':
            valid_chars = "0123456789.-"
        else:
            valid_chars = "0123456789ABCDEFabcdef.-"

        is_valid = True
        # Check if all characters are allowed
        if not all(c in valid_chars for c in num_input):
            is_valid = False
        # Check if '-' is only at the beginning
        if '-' in num_input and num_input.find('-') != 0:
            is_valid = False
        # Check if there's only one dot
        if num_input.count('.') > 1:
            is_valid = False
        # Special case: input is just '-' or '.' or '-.'
        if num_input in ["-", ".", "-."]:
            is_valid = False

        if not is_valid:
            print("Invalid input")
            continue

        # Execution
        try:
            if choice == '1':
                print(f"Hexadecimal result: {fromDecToHex(num_input)}")
            else:
                print(f"Decimal result: {fromHexToDec(num_input)}")
        except:
            print("Invalid input")

if __name__ == "__main__":
    main()