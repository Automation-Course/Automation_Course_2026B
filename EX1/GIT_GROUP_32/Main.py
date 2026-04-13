def is_hex(s):
"""בדיקה אם המחרוזת היא מספר הקסדצימלי תקין (כולל שלילי עם '-')"""
if not s:
return False
# טיפול בסימן מינוס בתחילת הקלט
start_index = 1 if s.startswith('-') else 0
if len(s) == start_index: # מחרוזת שהיא רק "-" אינה תקינה
return False

valid_chars = "0123456789ABCDEF"
for char in s[start_index:].upper():
if char not in valid_chars:
return False
return True

def is_integer(s):
"""בדיקה אם המחרוזת היא מספר עשרוני שלם (כולל שלילי)"""
if not s:
return False
start_index = 1 if s.startswith('-') else 0
if len(s) == start_index:
return False
return s[start_index:].isdigit()

def hex_to_decimal(hex_str):
"""המרה מהקסדצימלי לעשרוני ללא פונקציות מובנות"""
hex_str = hex_str.upper()
is_negative = hex_str.startswith('-')
# הסרת הסימן לצורך החישוב המתמטי
value_part = hex_str[1:] if is_negative else hex_str

result = 0
hex_chars = "0123456789ABCDEF"
# חישוב הערך המלא של כל הספרות לפי חזקות של 16 [cite: 77, 302]
for i in range(len(value_part)):
char = value_part[len(value_part) - 1 - i]
digit_value = hex_chars.find(char)
result += digit_value * (16 ** i)

return -result if is_negative else result

def decimal_to_hex(num):
"""המרה מעשרוני להקסדצימלי ללא פונקציות מובנות"""
if num == 0:
return "0"

is_negative = num < 0
abs_num = abs(num)
result = ""
hex_chars = "0123456789ABCDEF"
# אלגוריתם חילוק ב-16 ומציאת שארית [cite: 151, 318]
while abs_num > 0:
remainder = abs_num % 16
result = hex_chars[remainder] + result
abs_num //= 16

return "-" + result if is_negative else result

def main():
print("Welcome to the Standard Hexadecimal Converter!")

while True:
# הצגת תפריט המשתמש [cite: 63, 225]
print("\nChoose an option:")
print("1. Convert from hexadecimal to decimal")
print("2. Convert from decimal to hexadecimal")
print("3. Exit")

choice = input("Enter your choice: ")

if choice == "1":
hex_val = input("Enter a hexadecimal number (e.g., A5 or -A5): ")
if is_hex(hex_val):
print(f"Decimal value: {hex_to_decimal(hex_val)}")
else:
print("Invalid input! Use 0-9, A-F and optionally '-' at the start.")

elif choice == "2":
dec_val = input("Enter a decimal number: ")
if is_integer(dec_val):
# המרה ל-int ושליחה לפונקציה [cite: 122, 247]
print(f"Hexadecimal value: {decimal_to_hex(int(dec_val))}")
else:
print("Invalid input! Please enter a valid integer.")

elif choice == "3":
print("Goodbye!") # [cite: 174, 257]
break
else:
print("Invalid choice. Try again.") # [cite: 181, 262]

if __name__ == "__main__":
main()
