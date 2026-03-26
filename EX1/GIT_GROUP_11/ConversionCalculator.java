
import java.util.Scanner;

public class ConversionCalculator {
	
	public static boolean isValidHex(String hex) {

	    // Iterate over each character in the input string
	    for (int i = 0; i < hex.length(); i++) {
	        char ch = hex.charAt(i);

	        // Check if the character is a valid hexadecimal digit:
	        // 0-9  → numeric digits
	        // A-F  → upper hexadecimal letters
	        // a-f  → lower hexadecimal letters
	        if (!((ch >= '0' && ch <= '9') ||
	              (ch >= 'A' && ch <= 'F') ||
	              (ch >= 'a' && ch <= 'f'))) {

	            // If any character is invalid, return false immediately
	            return false;
	        }
	    }

	    // If all characters are valid, return true
	    return true;
	}
	
	public static int hexToDecimal(String hex) {
	    int decimal = 0; // Stores the resulting decimal value

	    // Iterate over each character in the hexadecimal string
	    for (int i = 0; i < hex.length(); i++) {
	        char ch = hex.charAt(i);
	        int value;

	        // Convert current character to its numeric value
	        if (ch >= '0' && ch <= '9') {
	            value = ch - '0'; // '0'–'9' → 0–9
	        } else if (ch >= 'A' && ch <= 'F') {
	            value = ch - 'A' + 10; // 'A'–'F' → 10–15
	        } else {
	            value = ch - 'a' + 10; // 'a'–'f' → 10–15
	        }

	        // Check if the next computation will exceed max int value (2147483647)
	        // This prevents overflow before performing the multiplication
	        if (decimal > (2147483647 - value) / 16) {
	            return -1; // Indicates that the value is too large
	        }

	        // Update the decimal value
	        // multiply previous result by 16 and add current digit
	        decimal = decimal * 16 + value;
	    }

	    // Return the final decimal result
	    return decimal;
	}
	
    public static String decimalToHexSafe(String input) {

        // Check that all characters are digits (0-9)
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);
            if (ch < '0' || ch > '9') {
                return "Invalid input"; // Non-digit character found
            }
        }

        int decimal=0;

        try {
            // Try converting to integer
            decimal = Integer.parseInt(input);
        } catch (NumberFormatException e) {
            // Number is too large for integer
            return "Invalid input - number is too large";
        }

        return decimalToHex(decimal);
    }
    
 // Converts a decimal integer to its hexadecimal representation
    public static String decimalToHex(int decimal) {

        // Special case: if the number is 0, return "0"
        if (decimal == 0) {
            return "0";
        }

        String hex = ""; // Will store the resulting hexadecimal string

        // Repeat until the number becomes 0
        while (decimal > 0) {

            // Get the remainder when dividing by 16 (next hex digit)
            int remainder = decimal % 16;
            char ch;

            // Convert remainder to corresponding hexadecimal character
            if (remainder <= 9) {
                ch = (char) ('0' + remainder); // 0–9 → '0'–'9'
            } else {
                ch = (char) ('A' + (remainder - 10)); // 10–15 → 'A'–'F'
            }

            // Add the character to the beginning of the result string
            // (since we build the number from least significant digit to most)
            hex = ch + hex;

            // Reduce the number by dividing by 16
            decimal = decimal / 16;
        }

        // Return the final hexadecimal string
        return hex;
    }
    
    public static void printMenu() {
        System.out.println("Hello, what would you like to do today?");
        System.out.println("1. Convert from hexadecimal to decimal");
        System.out.println("2. Convert from decimal to hexadecimal");
        System.out.println("3. Exit");
        System.out.print("Enter your choice: ");
    }
    
 // Checks if the given string contains only numeric digits (0-9)
    public static boolean isNumeric(String input) {

        // If the string is empty, it is not a valid number
        if (input.length() == 0) {
            return false;
        }

        // Iterate over each character in the string
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);

            // Check if the character is not a digit (0-9)
            if (ch < '0' || ch > '9') {
                return false; // Invalid character found
            }
        }

        // All characters are digits → valid numeric string
        return true;
    }
    
 // Handles user input for converting hexadecimal to decimal
    public static void handleHexToDecimal(Scanner scanner) {

        // Prompt the user to enter a hexadecimal number
        System.out.print("Enter a hexadecimal number: ");
        String hexInput = scanner.nextLine();

        // Validate that the input is a valid hexadecimal string
        if (!isValidHex(hexInput)) {
            System.out.println("Invalid input: not a valid hexadecimal number");
            return; // Stop execution if input is invalid
        }

        // Convert the valid hexadecimal string to decimal
        int result = hexToDecimal(hexInput);

        // Check if the result indicates overflow (number too large)
        if (result == -1) {
            System.out.println("Invalid input: value exceeds maximum allowed (2147483647)");
        } else {
            // Print the converted decimal value
            System.out.println("The decimal value is: " + result);
        }
    }
    
 // Handles user input for converting decimal to hexadecimal
    public static void handleDecimalToHex(Scanner scanner) {

        // Prompt the user to enter a decimal number
        System.out.print("Enter a decimal number: ");
        String decimalInput = scanner.nextLine();

        // Validate and convert the input using a safe conversion method
        // (handles invalid input and values that exceed integer range)
        String result = decimalToHexSafe(decimalInput);

        // Print the resulting hexadecimal value (or error message if invalid)
        System.out.println("The hexadecimal value is: " + result);
    }
    
 // Runs the main menu loop and handles user interaction
    public static void runMenu() {
        Scanner scanner = new Scanner(System.in);
        int choice = 0; // Stores the user's menu selection

        // Continue running until the user chooses to exit (option 3)
        do {
            // Display the menu options
            printMenu();

            // Read user input as a string
            String choiceInput = scanner.nextLine();

            // Validate that the input contains only digits
            if (!isNumeric(choiceInput)) {
                System.out.println("Invalid choice, please enter a value according to the menu options!");
                System.out.println();
                continue; // Restart loop if input is invalid
            }

            // Convert the valid numeric string to an integer
            choice = Integer.parseInt(choiceInput);

            // Handle user choice
            if (choice == 1) {
                handleHexToDecimal(scanner); // Option 1: Hex → Decimal
            } else if (choice == 2) {
                handleDecimalToHex(scanner); // Option 2: Decimal → Hex
            } else if (choice == 3) {
                System.out.println("Goodbye!"); // Exit message
            } else {
                // Input is numeric but not within valid menu options
                System.out.println("Invalid choice, please enter a value according to the menu options!");
            }

            // Print empty line for better readability between iterations
            System.out.println();

        } while (choice != 3); // Loop until user selects exit

        // Close the scanner to release system resources
        scanner.close();
    }
    
 // For String results
    public static void test(String name, String result, String expected) {
        if (result.equals(expected)) {
            System.out.println(name + " - PASSED");
        } else {
            System.out.println(name + " - FAILED");
            System.out.println("Expected: " + expected + ", Got: " + result);
        }
    }

    // For integer results
    public static void test(String name, int result, int expected) {
        if (result == expected) {
            System.out.println(name + " - PASSED");
        } else {
            System.out.println(name + " - FAILED");
            System.out.println("Expected: " + expected + ", Got: " + result);
        }
    }
    
    
    
    public static void main(String[] args) {
    	runMenu();
    }
		
	}


