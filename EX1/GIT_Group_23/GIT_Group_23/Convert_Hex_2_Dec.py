
def Convert_Hex_2_Dec(num):         #convert Hex (num) to Decimal
    is_valid, int_num, frac_num, is_negative = validate_hex_input(num)

    if not is_valid:                    #num is not valid
        return is_valid, ""
    
    decimal_val = 0
    int_power = len(int_num) - 1        #variable to help us calculate the whole part
    frac_power = -1                     #variable to help us calculate the fractal part
    
    decimal_val = Calc_Decimal_Val(int_num, int_power, decimal_val)
    decimal_val = Calc_Decimal_Val(frac_num, frac_power, decimal_val)
   
    if is_negative:
        decimal_val = -1 *decimal_val

    return is_valid, round(float(decimal_val), 3)



def Calc_Decimal_Val(num_str, power, Value):
    hex_digits = "0123456789ABCDEF"     #allowed hex digits
    for i in range(len(num_str)):
        char = num_str[i]
        
        digit_value = hex_digits.index(char)  
        Value += digit_value * (16 ** power)
        power -= 1
    return Value



def validate_hex_input(num_str):            #validate the input
    Valid_Input = True
    is_negative = False

    clean_num = num_str.strip().upper()     #remove spaces from the begning and the end of the input
    frac_num = ""

    if clean_num.startswith('-'):           #negative number (or invalid one)
        is_negative = True
        clean_num = clean_num[1:]           #To get the abs value

    #validation checks on the input:
    Valid_Input = Valid_Length(clean_num) and Valid_Dot(clean_num) and Valid_Charts(clean_num)

    if not Valid_Input:                     #not a valid input
        return False, "", "", is_negative
    
    else:                                   #a valid input
        int_num, frac_num = Split_Num(clean_num)   #split the number to whole and fractal parts
        return Valid_Input, int_num, frac_num, is_negative




def Valid_Length (num_str):     #length check - not too long
    if len(num_str) > 30:
        return False
    return True

def Valid_Dot(num_str):         # '.' check - can be 0 or 1. if 1 - must be in a valid position
    dot_count = num_str.count('.')
    if dot_count > 1:
        return False
    
    elif dot_count == 1:
        dot_index = num_str.find('.')
        if dot_index == 0 or dot_index == len(num_str) - 1:     #if there is a dot at the beginning or at the end
            return False
        
    return True     # 0 dot(s), or single dot in a valid position

def Valid_Charts (num_str):     #chars validation check
    valid_chars = "0123456789ABCDEF."
    for char in num_str:
        if char not in valid_chars:
            return False
    return True

def Split_Num(num_str):         #split the number to whole and fractal parts
    dot_index = num_str.find('.')
    if dot_index != -1:
        int_num = num_str[:dot_index] 
        frac_num = num_str[dot_index+1:] 
    else:
        int_num = num_str 
        frac_num = "" 
    return int_num, frac_num