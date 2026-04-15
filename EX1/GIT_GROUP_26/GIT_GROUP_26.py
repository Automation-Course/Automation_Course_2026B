import sys


def decimal_to_hexadecimal(decimal_str):
    # ניקוי רווחים מהתחלה וסוף
    decimal_str = decimal_str.strip()
    if not decimal_str:
        return "Error: Input is empty."

    # טיפול במספר שלילי
    is_negative = False #דגל לסימון מספר שלילי
    if decimal_str.startswith('-'):
        is_negative = True
        decimal_str = decimal_str[1:]

        # וידוא שאין רווחים פנימיים
    if " " in decimal_str:
        return "Error: Input contains spaces. Please enter a continuous number (no spaces)."

    # חסימת מצב של יותר מנקודה אחת
    if decimal_str.count('.') > 1:
        return "Error: Invalid input. The number contains more than one decimal point."

    if not decimal_str or decimal_str == ".":
        return "Error: No valid value entered."

    #פיצול לחלק שלם ושבר
    parts = decimal_str.split('.')
    int_str = parts[0]
    frac_str = parts[1] if len(parts) > 1 else ""


    # נוודא שמה שנשאר זה רק ספרות
    if (int_str and not int_str.isdigit()) or (frac_str and not frac_str.isdigit()):
        return "Error: Invalid decimal input. Please enter digits and a decimal point only."

    # המרת החלק השלם
    int_val = int(int_str) if int_str else 0
    hex_digits = "0123456789ABCDEF"

    if int_val == 0:
        hex_int_result = "0"
    else:
        hex_int_result = ""
        while int_val > 0:
            remainder = int_val % 16
            hex_int_result = hex_digits[remainder] + hex_int_result
            int_val //= 16

    # המרת החלק השברי
    hex_frac_result = ""
    if frac_str:
        frac_val = int(frac_str) / (10 ** len(frac_str))
        precision = 6  # הגבלת דיוק

        while frac_val > 0 and len(hex_frac_result) < precision:
            frac_val *= 16
            digit = int(frac_val) #בודק את כמות השלמים
            hex_frac_result += hex_digits[digit]
            frac_val -= digit

    # חיבור החלקים
    final_hex = hex_int_result
    if hex_frac_result:
        final_hex += "." + hex_frac_result

    if is_negative:
        final_hex = "-" + final_hex

    return final_hex


def hexadecimal_to_decimal(hex_str):
    #  הסרת רווחים מהסוף ומההתחלה והפיכת אותיות כולן לגדולות
    hex_str = hex_str.strip().upper()
    if not hex_str:
        return "Error: Input is empty."

    is_negative = False
    if hex_str.startswith('-'):
        is_negative = True
        hex_str = hex_str[1:]

    if " " in hex_str:
        return "Error: Input contains spaces inside the number."

    if hex_str.count('.') > 1:
        return "Error: Invalid input. The number contains more than one decimal point."

    if not hex_str or hex_str == ".":
        return "Error: No valid value entered."

    parts = hex_str.split('.')
    int_str = parts[0]
    frac_str = parts[1] if len(parts) > 1 else ""

    hex_digits_map = {
        '0': 0, '1': 1, '2': 2, '3': 3, '4': 4, '5': 5, '6': 6, '7': 7,
        '8': 8, '9': 9, 'A': 10, 'B': 11, 'C': 12, 'D': 13, 'E': 14, 'F': 15
    }

    #  המרת החלק השלם
    decimal_int_result = 0
    power = 0
    for i in range(len(int_str) - 1, -1, -1):
        digit = int_str[i]
        if digit not in hex_digits_map:
            return f"Error: invalid input."
        decimal_int_result += hex_digits_map[digit] * (16 ** power)
        power += 1

    # --- המרת החלק השברי (אם קיים) ---
    decimal_frac_result = 0.0
    power = -1
    for digit in frac_str:
        if digit not in hex_digits_map:
            return f"Error: invalid input."
        decimal_frac_result += hex_digits_map[digit] * (16 ** power)
        power -= 1

    # חיבור התוצאות
    final_decimal = decimal_int_result + decimal_frac_result

    # עיצוב התוצאה
    if not frac_str:
        final_result_str = str(int(final_decimal))
    else:
        final_result_str = str(final_decimal)

    if is_negative:
        final_result_str = "-" + final_result_str

    return final_result_str


def main():
    print("--- Base Converter: Decimal <-> Hexadecimal ---")
    while True:
        print("\nChoose an operation:")
        print("1. Convert Decimal to Hexadecimal")
        print("2. Convert Hexadecimal to Decimal")
        print("3. Exit")

        choice = input("Enter the number of the desired operation: ").strip()

        if choice == '1':
            dec_input = input("Enter a decimal number (e.g., 10.5 or -25): ")
            print(f"Hexadecimal result: {decimal_to_hexadecimal(dec_input)}")
        elif choice == '2':
            hex_input = input("Enter a hexadecimal number (e.g., A.8 or -FF): ")
            print(f"Decimal result: {hexadecimal_to_decimal(hex_input)}")
        elif choice == '3':
            print("Goodbye!")
            break
        else:
            print("Error: Invalid choice. Please choose 1, 2, or 3.")


if __name__ == "__main__":
    main()