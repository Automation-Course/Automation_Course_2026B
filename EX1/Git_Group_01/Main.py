def get_value_of_char(char):
    # מיפוי ידני - ללא פונקציות עזר
    mapping = {
        '0': 0, '1': 1, '2': 2, '3': 3, '4': 4, '5': 5, '6': 6, '7': 7, '8': 8, '9': 9,
        'A': 10, 'B': 11, 'C': 12, 'D': 13, 'E': 14, 'F': 15,
        'a': 10, 'b': 11, 'c': 12, 'd': 13, 'e': 14, 'f': 15
    }
    if char in mapping:
        return mapping[char]
    raise ValueError(f"התו '{char}' אינו תו הקסדצימלי חוקי")

def get_char_of_value(val):
    mapping = {
        0: '0', 1: '1', 2: '2', 3: '3', 4: '4', 5: '5', 6: '6', 7: '7', 8: '8', 9: '9',
        10: 'A', 11: 'B', 12: 'C', 13: 'D', 14: 'E', 15: 'F'
    }
    return mapping[val]

def decimal_to_hex(num, precision=10):
    is_negative = False
    if num < 0:
        is_negative = True
        num = -num

    integer_part = int(num // 1)
    fractional_part = num - integer_part

    # המרת חלק שלם
    if integer_part == 0:
        res_int = "0"
    else:
        res_int = ""
        temp_int = integer_part
        while temp_int > 0:
            remainder = temp_int % 16
            res_int = get_char_of_value(remainder) + res_int
            temp_int //= 16

    # המרת חלק שברי
    res_frac = ""
    temp_frac = fractional_part
    while temp_frac > 0 and len(res_frac) < precision:
        temp_frac *= 16
        digit = int(temp_frac // 1)
        res_frac += get_char_of_value(digit)
        temp_frac -= digit

    final_res = res_int + ("." + res_frac if res_frac != "" else "")
    return "-" + final_res if is_negative else final_res

def hex_to_decimal(hex_str):
    is_negative = False
    # Check for a leading negative sign
    if hex_str.startswith('-'):
        is_negative = True
        hex_str = hex_str[1:] # Remove the sign for conversion

    # ניקוי רווחים
    clean_hex = "".join(char for char in hex_str if char != " ")

    integer_part_str = ""
    fractional_part_str = ""
    found_dot = False

    for char in clean_hex:
        if char == ".":
            if found_dot: raise ValueError("מספר לא תקין: יותר מנקודה עשרונית אחת")
            found_dot = True
            continue
        if not found_dot:
            integer_part_str += char
        else:
            fractional_part_str += char

    decimal_val = 0.0

    # חישוב חלק שלם
    length = len(integer_part_str)
    for i in range(length):
        char = integer_part_str[i]
        val = get_value_of_char(char)
        decimal_val += val * (16 ** (length - 1 - i))

    # חישוב חלק שברי
    for i in range(len(fractional_part_str)):
        char = fractional_part_str[i]
        val = get_value_of_char(char)
        decimal_val += val * (16 ** -(i + 1))

    return -decimal_val if is_negative else decimal_val

def main():
    while True:
        print("\n" + "="*40)
        print("      מחשבון המרה: עשרוני <-> הקס")
        print("="*40)
        print(" 1. המרה מעשרוני להקסדצימלי (10 -> 16)")
        print(" 2. המרה מהקסדצימלי לעשרוני (16 -> 10)")
        print(" 3. יציאה")
        print("-" * 40)

        choice = input("בחר אפשרות (1-3): ")

        if choice == '3':
            print("\nלהתראות! סיום תוכנית.")
            break

        try:
            if choice == '1':
                raw_input = input("\nהכנס מספר עשרוני (למשל 255.5): ")
                num = float(raw_input)
                result = decimal_to_hex(num)
                print(f"התוצאה בבסיס 16:\n{result}")

            elif choice == '2':
                hex_input = input("\nהכנס מספר הקסדצימלי (למשל FF.8): ")
                result = hex_to_decimal(hex_input)
                print(f"התוצאה בבסיס 10:\n{result}")

            else:
                print("\n[!] שגיאה: בחירה לא חוקית, נסה שנית.")

        except ValueError as e:
            print(f"\n[!] שגיאה בקלט: {e}")
        except Exception as e:
            print(f"\n[!] שגיאה לא צפויה: {e}")

        input("\nלחץ על מקש כלשהו על מנת להמשיך...")

if __name__ == "__main__":
    main()
