
public class BaseConverter {
	
	private static int getHexChar(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        return -1; // Indicates invalid character
    }
	
	//Converts Hexadecimal to Decimal
	public static double hexToDecimal(String hex) {
		//Ensures input isn't empty or null. trim() discards spaces
		if(hex == null || hex.trim().isEmpty()) {
			System.out.println("Error: Empty input");
			return 0.0;
		}
		
		//Defines hex string scope 
		hex = hex.trim();
		boolean isNegative = (hex.charAt(0) == '-');
		if(isNegative) {
			if(hex.length() == 1){
				System.out.println("Error: Empty input");
				return 0.0;
			}
			hex = hex.substring(1);
		}
			
		//Defines integer part & fraction part
		int dotIndex = hex.indexOf('.'); // -1 if '.' not appears in input 
		String intPart = (dotIndex == -1)? hex : hex.substring(0, dotIndex);
		String fracPart = (dotIndex == -1)? "" : hex.substring(dotIndex + 1);
		
		//Process int part (right to left)
		int decimalInt = 0;
		int power = 1; // initiate as 16^0
		for(int i = intPart.length()-1; i>=0; i--) {
			char c = intPart.charAt(i);
			int charVal = getHexChar(c);
			if(charVal == -1) {
				System.out.println("Error: Invalid char in input");
				return 0.0;
			}
			decimalInt += charVal*power;
			power *= 16;			
		}
		//process fraction part(left to right)
		double decimalFrac = 0;
		double divisor = 16.0; // initiate as 16^1
		for(int i = 0; i<fracPart.length();i++) {
			char c = fracPart.charAt(i);
			int charVal = getHexChar(c);
			if(charVal == -1) {
				System.out.println("Error: Invalid char in input");
				return 0.0;
			}
			decimalFrac += charVal/divisor;
			divisor *= 16;	
		}
		double decimal  = decimalInt + decimalFrac;
		//if original number was negative add '-' sign
		return isNegative ? -decimal : decimal;
			
	}
	/////////////Converts Decimal to Hexadecimal/////////////
	public static String decimalToHex(double dec) {
		boolean isNegative = dec<0;
		if(isNegative)
			dec = -dec; // getting rid of the minus sign 
		
		// defines int and fraction part
		int intPart = (int) Math.floor(dec);
		double fracPart = dec - intPart;
		// local variables
		String hexadecimal = "";
		char[] hexNums = "0123456789ABCDEF".toCharArray(); // inserts each char in string into array
	
		//process int part
		if(intPart == 0)
			hexadecimal = "0";
		else {
			//chaining the value from bottom to up
			while(intPart>0) {
				int remainder = intPart % 16;
				hexadecimal = hexNums[remainder] + hexadecimal;
				intPart /= 16;
			}
		}
		
		//process fraction part if exists
		if(fracPart>0) {
			hexadecimal += ".";
			//assuming 9 digits max after decimal dot.
			int precision = 9;
			while(fracPart>0 && precision>0) {
				fracPart *= 16;
				int floorFrac = (int) Math.floor(fracPart);
				// chaining up to bottom
				hexadecimal += hexNums[floorFrac];
				fracPart -= floorFrac;
				precision--;
			}
		}
		//adding the negative sign if needed 
		if(isNegative && !hexadecimal.equals("0"))
			hexadecimal = "-" + hexadecimal;
		return hexadecimal;
	}		
	
	
	
	
	
	
	
	
	
	public static void main(String[] args) {
		System.out.println("Testing Hex to Decimal:");
		System.out.println("Input: '1A.5'     | Result: " + hexToDecimal("1A.5"));
		System.out.println("Input: '-FF'      | Result: " + hexToDecimal("-FF"));
		System.out.println("Input: '000'      | Result: " + hexToDecimal("000"));
		System.out.println("Input: '0.005'    | Result: " + hexToDecimal("0.005"));
		
		System.out.print("Input: '' (Empty) | Result: ");
		double res1 = hexToDecimal(""); // Should print error
		
		System.out.print("Input: '-'        | Result: ");
		double res2 = hexToDecimal("-"); // Should print error
		
		System.out.print("Input: '1G3'      | Result: ");
		double res3 = hexToDecimal("1G3"); // Should print error

		System.out.println("\nTesting Decimal to Hex:");
		System.out.println("Input: 26.3125    | Result: " + decimalToHex(26.3125));
		System.out.println("Input: -255.0     | Result: " + decimalToHex(-255.0));
		System.out.println("Input: 0.0        | Result: " + decimalToHex(0.0));
		System.out.println("Input: 0.005      | Result: " + decimalToHex(0.005));
		
		// Testing infinite fraction (precision limit)
		System.out.println("Input: 0.1        | Result: " + decimalToHex(0.1)); 
	}
}



	
	
	
	
	
	
	
	
	
	
	
	
	
	
	