import java.util.*;

public class targil1 {
    static Scanner sc = new Scanner(System.in);

    public static void main(String[] args) {
        String choice = "";
        while (true) {
            System.out.println("Please choose one of the options below\n1. Convert Hexadecimal to Decimal \n2. Convert Decimal to Hexadecimal \n0. For exit ");
            choice = sc.next();
            switch (choice) {
                case "1": // Hexadecimal to Decimal
                    System.out.println("Please enter a hexadecimal number (0-9, A-F)");
                    sc.nextLine(); // לניקוי ה-Buffer
                    String number1 = sc.nextLine().trim().toUpperCase();

                    // בשיטת משלים ל-2, הקלט ההקסדצימלי לא מכיל מינוס. המקסימום הוא 16 תווים.
                    while (!number1.matches("[0-9A-F]+") || number1.length() > 16) {
                        System.out.println("Invalid hexadecimal number. Try again:");
                        number1 = sc.nextLine().trim().toUpperCase();
                    }

                    System.out.println("Result: " + hexadecimalToDecimal(number1));
                    continue;

                case "2": // Decimal to Hexadecimal
                    System.out.println("Please enter a decimal number");
                    sc.nextLine();
                    while (true) {
                        String input = sc.nextLine().trim();

                        if (input.isEmpty() || input.equals("-") || input.contains(" ")) {
                            System.out.println("Invalid input. Try again:");
                            continue;
                        }

                        int maxDecLen = input.startsWith("-") ? 20 : 19;
                        if (input.length() > maxDecLen) {
                            System.out.println("Number too long for Long type. Try again:");
                            continue;
                        }

                        try {
                            long number2 = Long.parseLong(input);
                            System.out.println("Result: " + decimalToHexadecimal(number2));
                            break;
                        } catch (NumberFormatException e) {
                            System.out.println("Invalid decimal number. Try again:");
                        }
                    }
                    continue;

                case "0":
                    System.out.println("See you next time");
                    break;

                default:
                    System.out.println("Wrong input please try again:");
                    continue;
            }
            break;
        }
    }

    // המרה מהקסדצימלי לעשרוני - מבוסס משלים ל-2
    public static long hexadecimalToDecimal(String hex) {
        long decimal = 0;
        for (int i = 0; i < hex.length(); i++) {
            char ch = hex.charAt(i);
            int value = (ch >= '0' && ch <= '9') ? (ch - '0') : (ch - 'A' + 10);

            // הכפלה ב-16 והוספת הערך.
            // אם הוזן מספר כמו FFFFFFFFFFFFFFFE, הגלישה בזיכרון תיצור -2 אוטומטית!
            decimal = (decimal * 16) + value;
        }
        return decimal;
    }

    // המרה מעשרוני להקסדצימלי - מבוסס משלים ל-2
    public static String decimalToHexadecimal(long decimal) {
        if (decimal == 0) return "0";

        String hex = "";
        long temp = decimal;

        while (temp != 0) {
            // פעולת AND על סיביות (Bitwise AND) לחילוץ 4 הביטים הימניים
            int rem = (int)(temp & 15);

            char hexDigit = (rem < 10) ? (char)(rem + '0') : (char)(rem - 10 + 'A');
            hex = hexDigit + hex;

            // הזזה ללא שמירת סימן (Unsigned Right Shift)
            // זה מה שמאפשר לדחוף אפסים משמאל למספרים שליליים עד שהם מתאפסים
            temp >>>= 4;
        }

        return hex;
    }
}