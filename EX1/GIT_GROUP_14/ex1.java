import java.util.Scanner;

public class ex1 {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        String choice;

        while (true) {
            // Display the main menu
            System.out.println("Hex / Decimal Calculator");
            System.out.println("1 - Convert Decimal to Hex");
            System.out.println("2 - Convert Hex to Decimal");
            System.out.println("3 - Exit");
            System.out.print("Enter your choice: ");

            choice = scanner.nextLine().trim();

            if (choice.equals("1")) {
                // Read decimal input from the user
                System.out.print("Enter a decimal number: ");
                String decimalInput = scanner.nextLine().trim();

                // Validate the input before conversion
                if (isValidDecimal(decimalInput)) {
                    long decimalValue = parseDecimal(decimalInput);
                    String hexResult = convertDecimalToHex(decimalValue);
                    System.out.println("Hexadecimal result: " + hexResult);
                } else {
                    System.out.println("Invalid decimal input.");
                }

            } else if (choice.equals("2")) {
                // Read hexadecimal input from the user
                System.out.print("Enter a hexadecimal number: ");
                String hexInput = scanner.nextLine().trim();

                // Validate the input before conversion
                if (isValidHex(hexInput)) {
                    long decimalResult = convertHexToDecimal(hexInput);
                    System.out.println("Decimal result: " + decimalResult);
                } else {
                    System.out.println("Invalid hexadecimal input.");
                }

            } else if (choice.equals("3")) {
                // Exit the program
                System.out.println("Program ended.");
                break;


            } else {
                // Handle invalid menu choice
                System.out.println("Invalid menu choice.");
            }

            System.out.println();
        }

        scanner.close();
    }

    // Check if the decimal input is valid
    public static boolean isValidDecimal(String input) {
        if (input == null || input.isEmpty()) {
            return false;
        }

        int startIndex = 0;

        // Allow optional plus or minus sign
        if (input.charAt(0) == '+' || input.charAt(0) == '-') {
            if (input.length() == 1) {
                return false;
            }
            startIndex = 1;
        }

        // All remaining characters must be digits
        for (int i = startIndex; i < input.length(); i++) {
            char currentChar = input.charAt(i);

            if (currentChar < '0' || currentChar > '9') {
                return false;
            }
        }

        return true;
    }

    // Check if the hexadecimal input is valid
    public static boolean isValidHex(String input) {
        if (input == null || input.isEmpty()) {
            return false;
        }

        int startIndex = 0;

        // Allow optional plus or minus sign
        if (input.charAt(0) == '+' || input.charAt(0) == '-') {
            if (input.length() == 1) {
                return false;
            }
            startIndex = 1;
        }

        // All remaining characters must be valid hexadecimal characters
        for (int i = startIndex; i < input.length(); i++) {
            char currentChar = input.charAt(i);

            boolean isDigit = currentChar >= '0' && currentChar <= '9';
            boolean isUpperCaseLetter = currentChar >= 'A' && currentChar <= 'F';
            boolean isLowerCaseLetter = currentChar >= 'a' && currentChar <= 'f';

            if (!isDigit && !isUpperCaseLetter && !isLowerCaseLetter) {
                return false;
            }
        }

        return true;
    }

    // Convert a decimal string to a long value manually
    public static long parseDecimal(String input) {
        boolean isNegative = false;
        int startIndex = 0;
        long result = 0;

        if (input.charAt(0) == '-') {
            isNegative = true;
            startIndex = 1;
        } else if (input.charAt(0) == '+') {
            startIndex = 1;
        }

        // Build the decimal value digit by digit
        for (int i = startIndex; i < input.length(); i++) {
            int digit = input.charAt(i) - '0';
            result = result * 10 + digit;
        }

        if (isNegative) {
            return -result;
        }

        return result;
    }

    // Convert a decimal number to hexadecimal manually
    public static String convertDecimalToHex(long number) {
        if (number == 0) {
            return "0";
        }

        boolean isNegative = false;

        if (number < 0) {
            isNegative = true;
            number = -number;
        }

        String hexDigits = "0123456789ABCDEF";
        String result = "";

        // Repeatedly divide by 16 and collect remainders
        while (number > 0) {
            int remainder = (int) (number % 16);
            result = hexDigits.charAt(remainder) + result;
            number = number / 16;
        }

        if (isNegative) {
            result = "-" + result;
        }

        return result;
    }

    // Convert a single hexadecimal character to its decimal value
    public static int hexCharToValue(char ch) {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        } else {
            return ch - 'a' + 10;
        }
    }

    // Convert a hexadecimal string to decimal manually
    public static long convertHexToDecimal(String input) {
        boolean isNegative = false;
        int startIndex = 0;
        long result = 0;

        if (input.charAt(0) == '-') {
            isNegative = true;
            startIndex = 1;
        } else if (input.charAt(0) == '+') {
            startIndex = 1;
        }

        // Build the decimal value using base 16
        for (int i = startIndex; i < input.length(); i++) {
            int value = hexCharToValue(input.charAt(i));
            result = result * 16 + value;
        }

        if (isNegative) {
            return -result;
        }

        return result;
    }
}