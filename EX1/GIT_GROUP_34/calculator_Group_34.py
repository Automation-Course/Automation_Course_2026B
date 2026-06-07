from decimal import Decimal, InvalidOperation, localcontext


def _int_to_hex(non_negative_int):
    if non_negative_int == 0:
        return "0"

    hex_chars = "0123456789ABCDEF"
    hex_result = ""

    # Divide by 16 and build the result from remainders.
    while non_negative_int > 0:
        remainder = non_negative_int % 16
        hex_result = hex_chars[remainder] + hex_result
        non_negative_int = non_negative_int // 16

    return hex_result


def _fraction_to_hex(fraction_part, max_fraction_digits):
    hex_chars = "0123456789ABCDEF"
    digits = []

    # Multiply the fractional part by 16 repeatedly.
    while fraction_part != 0 and len(digits) < max_fraction_digits:
        fraction_part *= 16
        digit_value = int(fraction_part)
        digits.append(hex_chars[digit_value])
        fraction_part -= digit_value

    return "".join(digits).rstrip("0")


def decimal_to_hex(decimal_num, max_fraction_digits=12):
    if isinstance(decimal_num, bool):
        raise TypeError("decimal_num must be an integer or decimal number")

    if isinstance(decimal_num, int):
        decimal_value = Decimal(decimal_num)
    elif isinstance(decimal_num, float):
        decimal_value = Decimal(str(decimal_num))
    elif isinstance(decimal_num, Decimal):
        decimal_value = decimal_num
    else:
        raise TypeError("decimal_num must be an integer or decimal number")

    if decimal_value.is_nan() or decimal_value.is_infinite():
        raise ValueError("decimal_num must be a finite number")

    if max_fraction_digits < 0:
        raise ValueError("max_fraction_digits must be non-negative")

    sign = "-" if decimal_value < 0 else ""
    decimal_value = abs(decimal_value)

    integer_part = int(decimal_value)
    fraction_part = decimal_value - integer_part

    integer_hex = _int_to_hex(integer_part)
    fraction_hex = _fraction_to_hex(fraction_part, max_fraction_digits)

    if not fraction_hex:
        return sign + integer_hex

    return sign + integer_hex + "." + fraction_hex


def _hex_char_to_value(char):
    hex_chars = "0123456789ABCDEF"
    if char not in hex_chars:
        raise ValueError(f"invalid hexadecimal digit: {char}")
    return hex_chars.index(char)

def hex_to_decimal(hex_str):
    if not isinstance(hex_str, str):
        raise TypeError("hex_str must be a string")

    hex_str = hex_str.strip()
    if not hex_str:
        raise ValueError("hex_str cannot be empty")

    sign = 1
    if hex_str[0] == "-":
        sign = -1
        hex_str = hex_str[1:]
    elif hex_str[0] == "+":
        hex_str = hex_str[1:]

    if not hex_str:
        raise ValueError("hex_str must contain hexadecimal digits")

    hex_str = hex_str.upper()

    if hex_str.count(".") > 1:
        raise ValueError("hex_str can contain at most one decimal point")

    fraction_digits = 0
    decimal_point_seen = False
    digit_seen = False

    for char in hex_str:
        if char == ".":
            decimal_point_seen = True
        else:
            _hex_char_to_value(char)
            digit_seen = True
            if decimal_point_seen:
                fraction_digits += 1

    if not digit_seen:
        raise ValueError("hex_str must contain hexadecimal digits")

    if not decimal_point_seen:
        integer_result = 0
        for char in hex_str:
            value = _hex_char_to_value(char)
            integer_result = integer_result * 16 + value
        return sign * integer_result

    precision = max(28, fraction_digits * 3 + 10)
    with localcontext() as ctx:
        ctx.prec = precision
        decimal_result = Decimal(0)
        frac_divisor = Decimal(1)
        decimal_point_seen = False

        for char in hex_str:
            if char == ".":
                decimal_point_seen = True
                continue

            value = _hex_char_to_value(char)
            if not decimal_point_seen:
                decimal_result = decimal_result * 16 + Decimal(value)
            else:
                frac_divisor *= 16
                decimal_result += Decimal(value) / frac_divisor

    return Decimal(sign) * decimal_result

if __name__ == "__main__":
    while True:
        print("\nChoose a conversion type:")
        print("1. Decimal to Hexadecimal (10 -> 16)")
        print("2. Hexadecimal to Decimal (16 -> 10)")
        print("3. Exit")

        choice = input("Enter 1, 2, or 3: ").strip()

        try:
            if choice == "1":
                decimal_input = input("Enter a decimal number (integer or fractional): ").strip()
                decimal_num = Decimal(decimal_input)
                print(f"Result: {decimal_to_hex(decimal_num)}")
            elif choice == "2":
                hex_input = input("Enter a hexadecimal number: ").strip()
                print(f"Result: {hex_to_decimal(hex_input)}")
            elif choice == "3":
                print("Goodbye.")
                break
            else:
                print("Invalid choice. Please enter 1, 2, or 3.")
        except ValueError as error:
            print(f"Input error: {error}")
        except InvalidOperation:
            print("Input error: invalid decimal number format")
        except TypeError as error:
            print(f"Type error: {error}")