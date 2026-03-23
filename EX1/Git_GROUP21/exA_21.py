# Base converter: Decimal <-> Hexadecimal

HEX_DIGITS = "0123456789ABCDEF"  # all the hex digits we need
MAX_FRACTION_DIGITS = 8  # max digits after the decimal point in hex


def decimal_to_hex(number):
    # input checks
    if not isinstance(number, (int, float)):
        raise ValueError("הקלט חייב להיות מספר")
    if number < 0:
        raise ValueError("הקלט חייב להיות אי-שלילי")

    # split into integer and fractional parts
    integer_part = int(number)
    fraction_part = number - integer_part

    # convert the integer part
    if integer_part == 0:
        hex_integer = "0"
    else:
        hex_integer = ""
        temp = integer_part
        while temp > 0:
            remainder = temp % 16
            hex_integer = HEX_DIGITS[remainder] + hex_integer  # build the string right to left
            temp = temp // 16

    # convert the fractional part
    if fraction_part == 0:
        return hex_integer  # no fraction, we're done

    hex_fraction = ""
    count = 0
    while fraction_part > 0 and count < MAX_FRACTION_DIGITS:
        fraction_part *= 16  # multiply by 16
        digit = int(fraction_part)  # grab the integer part as our next digit
        hex_fraction += HEX_DIGITS[digit]  # add it to the result
        fraction_part -= digit  # keep only the fractional part for next round
        count += 1

    return hex_integer + "." + hex_fraction


def hex_to_decimal(hex_string):
    # input checks
    if not isinstance(hex_string, str) or hex_string == "":
        raise ValueError("הקלט חייב להיות מחרוזת לא ריקה")

    hex_string = hex_string.upper()

    # make sure there's at most one dot
    if hex_string.count(".") > 1:
        raise ValueError("קלט לא חוקי- יש להכניס מספר שלם או עשרוני")

    # split into integer and fractional parts
    if "." in hex_string:
        integer_str, fraction_str = hex_string.split(".")
    else:
        integer_str, fraction_str = hex_string, ""

    # check that every character is a valid hex digit
    for char in integer_str + fraction_str:
        if char not in HEX_DIGITS:
            raise ValueError(f"תו לא חוקי: '{char}'")

    # convert the integer part
    integer_result = 0
    power = 0
    for i in range(len(integer_str) - 1, -1, -1):
        digit_value = HEX_DIGITS.index(integer_str[i])
        integer_result += digit_value * (16 ** power)
        power += 1

    # convert the fractional part using negative powers of 16
    fraction_result = 0
    for i, char in enumerate(fraction_str):
        digit_value = HEX_DIGITS.index(char)
        fraction_result += digit_value * (16 ** -(i + 1))

    return integer_result + fraction_result


# user interface

def main():
    print("מחשבון המרת בסיסים")

    while True:
        print("\nבחר פעולה:")
        print("  1. עשרוני -> הקסדצימלי")
        print("  2. הקסדצימלי -> עשרוני")
        print("  3. יציאה")

        choice = input("בחירה: ").strip()

        if choice == "1":
            user_input = input("הכנס מספר עשרוני: ").strip()
            try:
                number = float(user_input)
            except ValueError:
                print("שגיאה: קלט לא חוקי- יש להכניס מספר שלם או עשרוני")
                continue
            try:
                print(f"תוצאה: {decimal_to_hex(number)}")
            except ValueError as e:
                print(f"שגיאה: {e}")

        elif choice == "2":
            user_input = input("הכנס מספר הקסדצימלי: ").strip()
            try:
                print(f"תוצאה: {hex_to_decimal(user_input)}")
            except ValueError as e:
                print(f"שגיאה: {e}")

        elif choice == "3":
            print("ביוש :)")
            break

        else:
            print("בחירה לא חוקית, נסה שוב.")


if __name__ == "__main__":
    main()