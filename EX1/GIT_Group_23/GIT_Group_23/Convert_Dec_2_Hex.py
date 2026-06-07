def Convert_Dec_2_Hex(num):
    isNegative = False          

    valid_Num = Valid_Length(num) and Valid_Num(num)
    if not valid_Num:
        return (False, "")      #num is not a valid number

    num = float(num)
    if num < 0:                         #check wether num is negative
        isNegative = True
        num = abs(num)

    hex_string = ""                     #represents the whole part of the hex number
    frac_hex = ""                       #represents the fractal part of the hex number (if exists)

    int_num, frac_num = Split_Num(num)   #to find the whole and fractal parts of the number               

    if int_num == 0:
        hex_string = "0"
    else:
        hex_string = Calc_Whole_Part_Hex_Value(hex_string, int_num)
        
    if frac_num > 0:                    #dealing with the fractal part (if exists)
        frac_hex = Calc_Frac_Part_Hex_Value(frac_num)
    else:
            frac_hex = ".0"
        

    if isNegative == True:              #originally num < 0
        return (True, "-" + hex_string + frac_hex)
    else:
        return (True, hex_string + frac_hex)



def Valid_Length (num_str):     #length check - not too long
    if len(num_str) > 30:
        return False
    return True

def Valid_Num(num_str):     #check if the input is a valid number
    dot_count = num_str.count('.')
    if dot_count == 1:
        dot_index = num_str.find('.')
        if dot_index == 0 or dot_index == len(num_str) - 1:
            return False            #to avoid cases like ".5" or "5."
        elif dot_index == 1 and num_str[0] == '-':
            return False            #to avoid cases like "-.5"
    try:
        float(num_str)
        return True
    except:
        return False
    

def Split_Num(num_str):             #split the number to whole and fractal parts
    int_num = int (num_str)         #to find the whole part of the number
    frac_num = num_str - int_num    #to find the fractal part of the number
    return int_num, frac_num


def Calc_Whole_Part_Hex_Value(hex_str, num_str):
    hex_digits = "0123456789ABCDEF"     #allowed hex digits
    while num_str > 0:              #dealing with the whole part
            remainder = num_str % 16
            hex_str = hex_digits[remainder] + hex_str   
            num_str //= 16
    return hex_str

def Calc_Frac_Part_Hex_Value(num_str):
    hex_digits = "0123456789ABCDEF"     #allowed hex digits
    frac_hex = "."
    for _ in range(3):              #dealing 3 numbers after 0 
        num_str *= 16
        digit_index = int(num_str) #get the whole number
        frac_hex += hex_digits[digit_index]
        num_str -= digit_index     #keep only the remaining decimal
    return frac_hex