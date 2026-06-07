'''state variables'''

decimalnums = []
wholenums = []
wholepart = 0
decimalpart = 0.0
hexadict = {'-':'-', 0:'0', 1:'1', 2:'2', 3:'3', 4:'4', 5:'5',
              6:'6', 7:'7', 8:'8', 9:'9', 10:'A', 11:'B',
              12:'C', 13:'D', 14:'E', 15:'F'}




'''16 to 10 functions'''
def quotient16(numstring):
    # Converts the whole part of a hex string to its decimal integer value
    ans = 0
    for i in range(len(numstring)):
        power = len(numstring) - 1 - i
        num = reversedict(numstring[i])
        ans += num * (16 ** power)
    return ans

def decimal16(numstring):
    # Converts the fractional part of a hex string to its decimal float value
    ans = 0
    for i in range(len(numstring)):
        power = -1 - i
        num = reversedict(numstring[i])
        ans += num * (16 ** power)
    return ans


'''master 16 to 10 function'''
def master16to10(numstring):
    # Converts a full hex string (including sign and decimal point) to a decimal number
    negative = False
    ans = 0.0
    if numstring[0] == '-':
        numstring = numstring[1:]
        negative = True
    wholepart,decimalpart = split16to10(numstring)
    ans+= quotient16(wholepart)
    ans += decimal16(decimalpart)
    if negative:
        ans = ans * -1 
    return str(ans)
    




'''helper functions'''
def split(num):
    # Splits a float into its whole and fractional parts
    fullpart = num//1
    decimalpart = num - fullpart
    return fullpart, decimalpart

def split16to10(numstring):
    # Splits a hex string into whole and fractional parts at the '.'
    if '.' in numstring:
        wholepart, decimalpart = numstring.split('.')
    else:
        wholepart = numstring
        decimalpart = '0'
    return wholepart, decimalpart


def reversedict(char):
    # Looks up the decimal value of a hex character (e.g. 'A' -> 10)
    for key, value in hexadict.items():
        if char == value:
            return key




''' decimal part'''
def mult16(num):
    # Multiplies by 16 and returns the whole and fractional parts (used in repeated multiplication)
    multnum = num*16
    fullpart,decimalpart = split(multnum)
    return fullpart, decimalpart

def decimalpart16(num):
    # Extracts 3 hex digits from the fractional part using repeated multiplication by 16
    for i in range(3):
        fullpart, num = mult16(num)
        decimalnums.append(fullpart)


'''quotient'''
def divide16(num):
    # Returns the quotient and remainder when dividing by 16
    remainder = num % 16
    quotient = num // 16
    return quotient, remainder

def wholenum16(num):
    # Converts the whole part of a decimal number to hex digits via repeated division by 16
    if num == 0:
        wholenums.append(0)
    while num > 0:
        num, remainder = divide16(num)
        wholenums.append(remainder)


'''master 10 to 16 functions'''
def converttostring10to16():
    # Assembles the final hex string from the collected whole and decimal digit lists
    megastring = ''
    for i in range(len(wholenums)):
        megastring += hexadict[wholenums[i]]
    megastring += '.'
    for i in range(len(decimalnums)):
        megastring += hexadict[decimalnums[i]]
        
    return megastring


def master10to16(num):
    # Converts a decimal number to a hex string, handling negatives and fractions
    global decimalnums, wholenums
    decimalnums = []
    wholenums = []
    negative = False
    if num < 0:
        negative = True
        num = num * -1
    fullpart, decimalpart = split(num)
    decimalpart16(decimalpart)
    wholenum16(fullpart)
    if negative:
        wholenums.append('-')
    wholenums.reverse()
    return converttostring10to16()



'''console UI'''
def main():
    print("=== Base Converter: Decimal <-> Hexadecimal ===")
    while True:
        print("\n1. Decimal to Hexadecimal")
        print("2. Hexadecimal to Decimal")
        print("3. Exit")
        choice = input("Choose an option: ").strip()

        if choice == '1':
            raw = input("Enter a decimal number: ").strip()
            try:
                num = float(raw)
                result = master10to16(num)
                print(f"Result: {result}")
            except ValueError:
                print("Invalid input. Please enter a valid decimal number.")

        elif choice == '2':
            raw = input("Enter a hexadecimal number (e.g. 1F.A or -3C): ").strip()
            try:
                result = master16to10(raw)
                print(f"Result: {result}")
            except Exception:
                print("Invalid input. Please enter a valid hexadecimal number.")

        elif choice == '3':
            print("Goodbye.")
            break
        else:
            print("Invalid option. Please choose 1, 2, or 3.")


if __name__ == '__main__':
    main()