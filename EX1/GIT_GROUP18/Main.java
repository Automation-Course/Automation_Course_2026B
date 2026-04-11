import java.util.Scanner;

public class Main {

    // Converts a single hex character (0-9, A-F) to its decimal value
    public static int hexCharToValue(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        } else {
            throw new IllegalArgumentException("Invalid hexadecimal character");
        }
    }

    // Converts a hexadecimal number (including fractional part) to decimal
    public static double hexToDecimal(String hexNumber) {

        // Check for empty input
        if (hexNumber == null || hexNumber.length() == 0) {
            throw new IllegalArgumentException("Empty input");
        }

        boolean isNegative = false;

        // Handle negative numbers
        if (hexNumber.charAt(0) == '-') {
            isNegative = true;
            hexNumber = hexNumber.substring(1);
        }

        if (hexNumber.length() == 0) {
            throw new IllegalArgumentException("Empty input");
        }

        hexNumber = hexNumber.toUpperCase();

        // Split into integer and fractional parts
        String[] parts = hexNumber.split("\\.");

        if (parts.length > 2) {
            throw new IllegalArgumentException("Invalid number format");
        }

        String integerPart = parts[0];
        String fractionPart = "";

        if (parts.length == 2) {
            fractionPart = parts[1];
        }

        int integerResult = 0;

        // Convert integer part
        if (integerPart.length() > 0) {
            for (int i = 0; i < integerPart.length(); i++) {
                char c = integerPart.charAt(i);
                int value = hexCharToValue(c);
                integerResult = integerResult * 16 + value;
            }
        }

        double fractionResult = 0;
        double base = 16;

        // Convert fractional part using negative powers of 16
        for (int i = 0; i < fractionPart.length(); i++) {
            char c = fractionPart.charAt(i);
            int value = hexCharToValue(c);
            fractionResult += value / base;
            base *= 16;
        }

        double result = integerResult + fractionResult;

        if (isNegative) {
            result = -result;
        }

        return result;
    }


    // Converts a decimal number (including fractional part) to hexadecimal
    public static String decimalToHex(double number) {

        // Handle zero case
        if (number == 0) {
            return "0";
        }

        boolean isNegative = false;

        // Handle negative numbers
        if (number < 0) {
            isNegative = true;
            number = -number;
        }

        int integerPart = (int) number;
        double fractionPart = number - integerPart;

        String integerResult = "";

        // Convert integer part using repeated division
        if (integerPart == 0) {
            integerResult = "0";
        } else {
            while (integerPart > 0) {
                int remainder = integerPart % 16;
                integerPart = integerPart / 16;

                char digit;

                if (remainder < 10) {
                    digit = (char) (remainder + '0');
                } else {
                    digit = (char) (remainder - 10 + 'A');
                }

                // Build result from right to left
                integerResult = digit + integerResult;
            }
        }

        String fractionResult = "";

        int maxDigits = 10; // Limit to avoid infinite loop
        int count = 0;

        // Convert fractional part using repeated multiplication
        while (fractionPart > 0 && count < maxDigits) {
            fractionPart *= 16;
            int digitValue = (int) fractionPart;

            char digit;

            if (digitValue < 10) {
                digit = (char) (digitValue + '0');
            } else {
                digit = (char) (digitValue - 10 + 'A');
            }

            fractionResult += digit;
            fractionPart = fractionPart - digitValue;
            count++;
        }

        String result;

        if (fractionResult.length() > 0) {
            result = integerResult + "." + fractionResult;
        } else {
            result = integerResult;
        }

        if (isNegative) {
            result = "-" + result;
        }

        return result;
    }


    public static void main(String[] args) {

        Scanner input = new Scanner(System.in);

        while (true) {
            System.out.println("\nChoose an option:");
            System.out.println("1 - Convert Hexadecimal to Decimal");
            System.out.println("2 - Convert Decimal to Hexadecimal");
            System.out.println("3 - Exit");

            String choice = input.nextLine();

            if (choice.equals("1")) {
                System.out.print("Enter a hexadecimal number: ");
                String hex = input.nextLine();

                try {
                    System.out.println("Decimal result: " + hexToDecimal(hex));
                } catch (Exception e) {
                    System.out.println("Error: " + e.getMessage());
                }

            } else if (choice.equals("2")) {
                System.out.print("Enter a decimal number: ");
                String numStr = input.nextLine();

                try {
                    double number = Double.parseDouble(numStr);
                    System.out.println("Hexadecimal result: " + decimalToHex(number));
                } catch (Exception e) {
                    System.out.println("Error: Invalid input");
                }

            } else if (choice.equals("3")) {
                System.out.println("Exiting...");
                break;

            } else {
                System.out.println("Invalid choice");
            }
        }

        input.close();
    }
}