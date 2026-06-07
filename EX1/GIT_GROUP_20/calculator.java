import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.Scanner;

public class calculator {
    /*
     * Lab notes:
     * 1. The program supports positive and negative numbers.
     * 2. The program supports whole numbers and fractional numbers.
     * 3. Spaces at the beginning or end are ignored, but spaces inside the
     *    number are not allowed.
     * 4. Hex input may use 0-9, A-F, or a-f.
     * 5. Hex output is always printed with uppercase letters.
     * 6. A number may contain only one decimal point.
     * 7. The input must contain at least one digit.
     * 8. When converting from decimal to hex, the fractional part is limited
     *    to 8 hex digits after the point. This keeps the output simple and
     *    avoids infinite repeating fractions.
     * 9. When converting from hex to decimal, the result is exact for any
     *    finite hex fraction.
     */

    private static final int MAX_HEX_FRACTION_DIGITS = 8;
    private static final BigInteger BIG_INTEGER_SIXTEEN = BigInteger.valueOf(16);
    private static final BigDecimal BIG_DECIMAL_SIXTEEN = BigDecimal.valueOf(16);

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        // Short introduction so the user knows what the program does.
        System.out.println("-------------------------------------------------");
        System.out.println("Welcome to Group 20 Base 10 / Base 16 Calculator!");
        System.out.println("-------------------------------------------------");

        while (true) {
            printMenu();
            String choice = scanner.nextLine().trim();

            if (choice.equals("0")) {
                System.out.println("Program ended.");
                break;
            }

            if (choice.equals("1")) {
                handleDecimalToHex(scanner);
            } else if (choice.equals("2")) {
                handleHexToDecimal(scanner);
            } else {
                System.out.println("Error: Please enter 1, 2, or 0.");
            }

            System.out.println();
        }

        scanner.close();
    }

    private static void printMenu() {
        // The menu is kept simple because this is meant for a beginner-friendly lab.
        System.out.println();
        System.out.println("Choose conversion:");
        System.out.println("1. Base 10 to Base 16");
        System.out.println("2. Base 16 to Base 10");
        System.out.println("0. Exit");
        System.out.print("Your choice: ");
    }

    private static void handleDecimalToHex(Scanner scanner) {
        // Read one line from the user, validate it, then convert it.
        System.out.print("Enter a decimal number: ");
        String input = scanner.nextLine();

        try {
            ParsedNumber parsedNumber = parseDecimalInput(input);
            String hexValue = decimalToHex(parsedNumber);
            System.out.println("Result: " + hexValue);
        } catch (IllegalArgumentException error) {
            System.out.println(error.getMessage());
        }
    }

    private static void handleHexToDecimal(Scanner scanner) {
        // Read one line from the user, validate it, then convert it.
        System.out.print("Enter a hexadecimal number: ");
        String input = scanner.nextLine();

        try {
            ParsedNumber parsedNumber = parseHexInput(input);
            String decimalValue = hexToDecimal(parsedNumber);
            System.out.println("Result: " + decimalValue);
        } catch (IllegalArgumentException error) {
            System.out.println(error.getMessage());
        }
    }

    private static ParsedNumber parseDecimalInput(String rawInput) {
        // First split the text into sign, integer part, and fractional part.
        ParsedTextParts parts = splitAndValidate(rawInput, false);

        // Check that every character in the integer part is a decimal digit.
        for (int index = 0; index < parts.integerPart.length(); index++) {
            char currentChar = parts.integerPart.charAt(index);
            if (!Character.isDigit(currentChar)) {
                throw new IllegalArgumentException(
                    "Error: Invalid decimal character '" + currentChar + "'."
                );
            }
        }

        // Check that every character in the fractional part is also a decimal digit.
        for (int index = 0; index < parts.fractionalPart.length(); index++) {
            char currentChar = parts.fractionalPart.charAt(index);
            if (!Character.isDigit(currentChar)) {
                throw new IllegalArgumentException(
                    "Error: Invalid decimal character '" + currentChar + "'."
                );
            }
        }

        BigInteger integerValue = parseUnsignedDecimalInteger(parts.integerPart);
        BigDecimal fractionalValue = parseDecimalFraction(parts.fractionalPart);

        return new ParsedNumber(parts.negative, integerValue, fractionalValue);
    }

    private static ParsedNumber parseHexInput(String rawInput) {
        // First split the text into sign, integer part, and fractional part.
        ParsedTextParts parts = splitAndValidate(rawInput, true);

        // Check that every character in the integer part is a valid hex digit.
        for (int index = 0; index < parts.integerPart.length(); index++) {
            char currentChar = parts.integerPart.charAt(index);
            if (hexDigitValue(currentChar) == -1) {
                throw new IllegalArgumentException(
                    "Error: Invalid hexadecimal character '" + currentChar + "'."
                );
            }
        }

        // Check that every character in the fractional part is also a valid hex digit.
        for (int index = 0; index < parts.fractionalPart.length(); index++) {
            char currentChar = parts.fractionalPart.charAt(index);
            if (hexDigitValue(currentChar) == -1) {
                throw new IllegalArgumentException(
                    "Error: Invalid hexadecimal character '" + currentChar + "'."
                );
            }
        }

        BigInteger integerValue = parseUnsignedHexInteger(parts.integerPart);
        BigDecimal fractionalValue = parseHexFraction(parts.fractionalPart);

        return new ParsedNumber(parts.negative, integerValue, fractionalValue);
    }

    private static ParsedTextParts splitAndValidate(String rawInput, boolean hexadecimal) {
        // Remove spaces around the number. Example: "  26.75  " becomes "26.75".
        String input = rawInput.trim();

        if (input.isEmpty()) {
            throw new IllegalArgumentException(
                hexadecimal
                    ? "Error: Hexadecimal input cannot be empty."
                    : "Error: Decimal input cannot be empty."
            );
        }

        boolean negative = false;
        int startIndex = 0;
        char firstChar = input.charAt(0);

        if (firstChar == '+' || firstChar == '-') {
            negative = firstChar == '-';
            startIndex = 1;
        }

        // Reject inputs like "+" or "-" with no digits after the sign.
        if (startIndex == input.length()) {
            throw new IllegalArgumentException(
                hexadecimal
                    ? "Error: Hexadecimal input must contain digits after the sign."
                    : "Error: Decimal input must contain digits after the sign."
            );
        }

        String unsignedPart = input.substring(startIndex);
        int pointCount = 0;
        boolean hasDigit = false;

        for (int index = 0; index < unsignedPart.length(); index++) {
            char currentChar = unsignedPart.charAt(index);

            // Only one decimal point is allowed.
            if (currentChar == '.') {
                pointCount++;
                if (pointCount > 1) {
                    throw new IllegalArgumentException("Error: Input cannot contain more than one decimal point.");
                }
                continue;
            }

            // A sign in the middle is always an error.
            if (currentChar == '+' || currentChar == '-') {
                throw new IllegalArgumentException("Error: Sign characters are allowed only at the beginning.");
            }

            // Internal spaces are not accepted because they make the format unclear.
            if (Character.isWhitespace(currentChar)) {
                throw new IllegalArgumentException("Error: Spaces inside the number are not allowed.");
            }

            hasDigit = true;
        }

        if (!hasDigit) {
            throw new IllegalArgumentException(
                hexadecimal
                    ? "Error: Hexadecimal input must contain at least one digit."
                    : "Error: Decimal input must contain at least one digit."
            );
        }

        int pointIndex = unsignedPart.indexOf('.');
        String integerPart;
        String fractionalPart;

        if (pointIndex == -1) {
            integerPart = unsignedPart;
            fractionalPart = "";
        } else {
            integerPart = unsignedPart.substring(0, pointIndex);
            fractionalPart = unsignedPart.substring(pointIndex + 1);
        }

        // Inputs like ".5" are allowed, so we treat the missing integer part as 0.
        if (integerPart.isEmpty()) {
            integerPart = "0";
        }

        return new ParsedTextParts(negative, integerPart, fractionalPart);
    }

    private static BigInteger parseUnsignedDecimalInteger(String digits) {
        // Build the decimal integer manually from left to right.
        // Example: "255" -> ((0 * 10 + 2) * 10 + 5) * 10 + 5
        BigInteger result = BigInteger.ZERO;

        for (int index = 0; index < digits.length(); index++) {
            int digit = digits.charAt(index) - '0';
            result = result.multiply(BigInteger.TEN).add(BigInteger.valueOf(digit));
        }

        return result;
    }

    private static BigDecimal parseDecimalFraction(String digits) {
        if (digits.isEmpty()) {
            return BigDecimal.ZERO;
        }

        // Example: "75" means 75 / 100 = 0.75
        BigInteger numerator = BigInteger.ZERO;
        BigInteger denominator = BigInteger.ONE;

        for (int index = 0; index < digits.length(); index++) {
            int digit = digits.charAt(index) - '0';
            numerator = numerator.multiply(BigInteger.TEN).add(BigInteger.valueOf(digit));
            denominator = denominator.multiply(BigInteger.TEN);
        }

        return new BigDecimal(numerator).divide(new BigDecimal(denominator));
    }

    private static BigInteger parseUnsignedHexInteger(String digits) {
        // Build the integer part using powers of 16.
        // Example: "1C" = (1 * 16) + 12
        BigInteger result = BigInteger.ZERO;
        BigInteger power = BigInteger.ONE;

        for (int index = digits.length() - 1; index >= 0; index--) {
            int digit = hexDigitValue(digits.charAt(index));
            result = result.add(power.multiply(BigInteger.valueOf(digit)));
            power = power.multiply(BIG_INTEGER_SIXTEEN);
        }

        return result;
    }

    private static BigDecimal parseHexFraction(String digits) {
        // Build the fractional part using negative powers of 16.
        // Example: ".8" = 8 / 16 = 0.5
        // Example: ".F" = 15 / 16 = 0.9375
        BigDecimal result = BigDecimal.ZERO;
        BigDecimal divisor = BIG_DECIMAL_SIXTEEN;

        for (int index = 0; index < digits.length(); index++) {
            int digit = hexDigitValue(digits.charAt(index));

            if (digit != 0) {
                BigDecimal digitValue = BigDecimal.valueOf(digit);
                result = result.add(digitValue.divide(divisor));
            }

            divisor = divisor.multiply(BIG_DECIMAL_SIXTEEN);
        }

        return result;
    }

    private static String decimalToHex(ParsedNumber value) {
        // Handle zero early so we do not print "-0" or "0.".
        if (value.isZero()) {
            return "0";
        }

        String integerHex = convertIntegerPartToHex(value.integerPart);
        String fractionalHex = convertFractionPartToHex(value.fractionalPart);
        StringBuilder builder = new StringBuilder();

        if (value.negative) {
            builder.append('-');
        }

        builder.append(integerHex);

        if (!fractionalHex.isEmpty()) {
            builder.append('.').append(fractionalHex);
        }

        return builder.toString();
    }

    private static String convertIntegerPartToHex(BigInteger integerPart) {
        if (integerPart.equals(BigInteger.ZERO)) {
            return "0";
        }

        // Repeated division by 16 gives the hex digits from right to left.
        StringBuilder builder = new StringBuilder();
        BigInteger currentValue = integerPart;

        while (!currentValue.equals(BigInteger.ZERO)) {
            BigInteger[] divisionResult = currentValue.divideAndRemainder(BIG_INTEGER_SIXTEEN);
            int remainder = divisionResult[1].intValue();
            builder.append(hexCharacter(remainder));
            currentValue = divisionResult[0];
        }

        return builder.reverse().toString();
    }

    private static String convertFractionPartToHex(BigDecimal fractionalPart) {
        if (fractionalPart.compareTo(BigDecimal.ZERO) == 0) {
            return "";
        }

        StringBuilder builder = new StringBuilder();
        BigDecimal currentValue = fractionalPart;

        /*
         * Repeated multiplication by 16 gives one digit at a time after the point.
         * Example:
         * 0.75 * 16 = 12.0, so the first hex digit is C.
         * We stop after MAX_HEX_FRACTION_DIGITS digits so repeating values do
         * not continue forever.
         */
        for (int index = 0; index < MAX_HEX_FRACTION_DIGITS; index++) {
            currentValue = currentValue.multiply(BIG_DECIMAL_SIXTEEN);
            int digit = currentValue.intValue();
            builder.append(hexCharacter(digit));
            currentValue = currentValue.subtract(BigDecimal.valueOf(digit));

            if (currentValue.compareTo(BigDecimal.ZERO) == 0) {
                break;
            }
        }

        return removeTrailingZeroHexDigits(builder.toString());
    }

    private static String removeTrailingZeroHexDigits(String digits) {
        // If the fractional result ends with zeros, remove them.
        // Example: "C000" becomes "C"
        int endIndex = digits.length();

        while (endIndex > 0 && digits.charAt(endIndex - 1) == '0') {
            endIndex--;
        }

        return digits.substring(0, endIndex);
    }

    private static String hexToDecimal(ParsedNumber value) {
        // Decimal value = integer part + fractional part, then apply the sign.
        BigDecimal decimalValue = new BigDecimal(value.integerPart).add(value.fractionalPart);

        if (value.negative && decimalValue.compareTo(BigDecimal.ZERO) != 0) {
            decimalValue = decimalValue.negate();
        }

        return formatDecimal(decimalValue);
    }

    private static String formatDecimal(BigDecimal value) {
        // Clean the output so 28.5000 becomes 28.5 and 10.0000 becomes 10.
        BigDecimal normalizedValue = value.stripTrailingZeros();

        if (normalizedValue.compareTo(BigDecimal.ZERO) == 0) {
            return "0";
        }

        return normalizedValue.toPlainString();
    }

    private static int hexDigitValue(char character) {
        // Convert one hex character into its numeric value.
        // If the character is not valid, return -1.
        if (character >= '0' && character <= '9') {
            return character - '0';
        }

        if (character >= 'A' && character <= 'F') {
            return character - 'A' + 10;
        }

        if (character >= 'a' && character <= 'f') {
            return character - 'a' + 10;
        }

        return -1;
    }

    private static char hexCharacter(int value) {
        // Convert a number from 0 to 15 into one hex character.
        if (value >= 0 && value <= 9) {
            return (char) ('0' + value);
        }

        return (char) ('A' + (value - 10));
    }

    private static class ParsedTextParts {
        // This class stores the raw text pieces after the input is split.
        private final boolean negative;
        private final String integerPart;
        private final String fractionalPart;

        private ParsedTextParts(boolean negative, String integerPart, String fractionalPart) {
            this.negative = negative;
            this.integerPart = integerPart;
            this.fractionalPart = fractionalPart;
        }
    }

    private static class ParsedNumber {
        // This class stores the numeric value after the text has been parsed.
        private final boolean negative;
        private final BigInteger integerPart;
        private final BigDecimal fractionalPart;

        private ParsedNumber(boolean negative, BigInteger integerPart, BigDecimal fractionalPart) {
            this.negative = negative;
            this.integerPart = integerPart;
            this.fractionalPart = fractionalPart;
        }

        private boolean isZero() {
            return integerPart.equals(BigInteger.ZERO) && fractionalPart.compareTo(BigDecimal.ZERO) == 0;
        }
    }
}
