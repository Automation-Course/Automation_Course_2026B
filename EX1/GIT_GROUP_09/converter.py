class HexCalculator:
# a class to convert between decimal and hexadecimal numbers 
    HEX_DIGITS = "0123456789ABCDEF"
#using abs to get the absolute value of the number 
    def decimal_to_hex(self, decimal_num):
        if decimal_num == 0:
            return "0"

        is_negative = decimal_num < 0
        temp_num = abs(decimal_num)
        hex_result = ""

        while temp_num > 0:
            remainder = temp_num % 16
            hex_result = self.HEX_DIGITS[remainder] + hex_result
            temp_num //= 16

        if is_negative:
            return "-" + hex_result
        return hex_result

    def hex_to_decimal(self, hex_str):
        hex_str = hex_str.strip().upper()

        if not hex_str:
            raise ValueError("Empty input")

        is_negative = False
        if hex_str[0] == "-":
            is_negative = True
            hex_str = hex_str[1:]
            if not hex_str:
                raise ValueError("Invalid input — minus sign without a number")

        decimal_result = 0
        length = len(hex_str)

        for i in range(length):
            char = hex_str[length - 1 - i]
            if char not in self.HEX_DIGITS:
                raise ValueError(f"Invalid character: '{char}'")
            digit_value = self.HEX_DIGITS.find(char)
            decimal_result += digit_value * (16 ** i)


        if is_negative:
            return -decimal_result
        return decimal_result


def main():
    calc = HexCalculator()
    print("--- Base Conversion Calculator ---")

    while True:
        print("\n1. Decimal → Hexadecimal")
        print("2. Hexadecimal → Decimal")
        print("0. Exit")
        choice = input("Choose: ")

        if choice == "0":
            print("Thank you for using the Base Conversion Calculator. Bye bye!")
            break
        elif choice == "1":
            user_input = input("Enter decimal number: ").strip()
            if not user_input.lstrip("-").isdigit():
                print("Error: integers only")
            else:
                print(f"Result: {calc.decimal_to_hex(int(user_input))}")
        elif choice == "2":
            try:
                print(f"Result: {calc.hex_to_decimal(input('Enter hexadecimal number: '))}")
            except ValueError as e:
                print(f"Error: {e}")
        else:
            print("Invalid choice")


if __name__ == "__main__":
    main()
