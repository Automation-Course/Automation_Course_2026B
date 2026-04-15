import java.util.Scanner;

public class HexCalculator {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        
        System.out.println("--- Base Converter Calculator ---");

        // Main infinite loop
        while (true) {
            int choice = 0;

            // Menu loop: Input validation for choices 1, 2, or 3
            while (choice != 1 && choice != 2 && choice != 3) {
                System.out.println("\nChoose an option:");
                System.out.println("1: Decimal to Hexadecimal");
                System.out.println("2: Hexadecimal to Decimal");
                System.out.println("3: Exit");
                System.out.print("Enter your choice (1, 2, or 3): ");

                if (scanner.hasNextInt()) {
                    choice = scanner.nextInt();
                    if (choice != 1 && choice != 2 && choice != 3) {
                        System.out.println(">> Error: '" + choice + "' is not a valid option. Please choose 1, 2, or 3.");
                    }
                } else {
                    String invalidInput = scanner.next(); // Consume the invalid token
                    System.out.println(">> Error: '" + invalidInput + "' is not a number. Try again.");
                }
            }

            // Exit condition
            if (choice == 3) {
                System.out.println("Exiting program. Goodbye!");
                break; // Break the infinite loop
            }

            // CRITICAL: Consume the leftover newline character from the menu selection
            scanner.nextLine(); 

            // Process user choice
            try {
                if (choice == 1) {
                    System.out.print("Enter a decimal number: ");
                    
                    // Read the entire line and remove all whitespaces
                    String decInput = scanner.nextLine().replaceAll("\\s+", ""); 
                    
                    try {
                        int dec = Integer.parseInt(decInput); // Convert the cleaned string to integer
                        System.out.println(">> Result Hexadecimal: " + decimalToHex(dec));
                    } catch (NumberFormatException ex) {
                        System.out.println(">> Error: Invalid decimal number.");
                    }
                    
                } else if (choice == 2) {
                    System.out.print("Enter a hexadecimal string: ");
                    
                    // Read the entire line and remove all whitespaces
                    String hex = scanner.nextLine().replaceAll("\\s+", "");
                    
                    System.out.println(">> Result Decimal: " + hexToDecimal(hex));
                }
            } catch (Exception e) {
                // Catching any logic exceptions (like invalid characters)
                System.out.println(">> Error: " + e.getMessage());
            }
            
            System.out.println("----------------------------------------");
        }

        scanner.close();
    }

    // Function to convert Decimal (Base 10) to Hexadecimal (Base 16)
    public static String decimalToHex(int n) {
        // Handle the zero edge case explicitly
        if (n == 0) return "0";

        boolean isNegative = false;
        
        // Handle negative numbers mathematically
        if (n < 0) {
            isNegative = true;
            n = -n; // Make the number positive for the core calculation
        }

        String hexChars = "0123456789ABCDEF";
        StringBuilder hexResult = new StringBuilder();

        // Core conversion logic using modulo and division
        while (n > 0) {
            int remainder = n % 16;
            hexResult.insert(0, hexChars.charAt(remainder));
            n = n / 16;
        }

        // Prepend the minus sign if the original number was negative
        if (isNegative) {
            hexResult.insert(0, "-");
        }

        return hexResult.toString();
    }

    // Function to convert Hexadecimal (Base 16) to Decimal (Base 10)
    public static int hexToDecimal(String hex) {
        // Normalize input: convert to uppercase (whitespaces are already removed in main)
        hex = hex.toUpperCase();

        // Edge case: Check if input is empty
        if (hex.isEmpty()) {
            throw new IllegalArgumentException("Input cannot be empty.");
        }

        boolean isNegative = false;

        // Handle mathematical negative sign
        if (hex.charAt(0) == '-') {
            isNegative = true;
            hex = hex.substring(1); // Remove the minus sign for the core calculation
        }

        // Edge case: Check if the input was ONLY a minus sign
        if (hex.isEmpty()) {
            throw new IllegalArgumentException("Input cannot be just a minus sign.");
        }

        String hexChars = "0123456789ABCDEF";
        int decimalValue = 0;

        // Core conversion logic using base-16 multiplication
        for (int i = 0; i < hex.length(); i++) {
            char currentChar = hex.charAt(i);
            int charValue = hexChars.indexOf(currentChar);

            // Edge case: Throw exception if character is not a valid hex digit
            if (charValue == -1) {
                throw new IllegalArgumentException("Invalid Hexadecimal character: " + currentChar);
            }

            // Mathematical formula: shift left by base 16 and add current value
            decimalValue = 16 * decimalValue + charValue;
        }

        // Apply negative sign to the final result if needed
        if (isNegative) {
            decimalValue = -decimalValue;
        }

        return decimalValue;
    }
}
