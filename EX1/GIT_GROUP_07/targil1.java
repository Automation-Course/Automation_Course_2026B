import java.util.*;
public class targil1 {
    static Scanner sc = new Scanner(System.in);

    public static void main(String[] args) {
        String choice="";
        while(true) {
            System.out.println("Please choose one of the options below\n1. Convert Hexadecimal to Decimal \n2. Convert Decimal to Hexadecimal \n0. For exit ");
            choice=sc.next();
            switch(choice) {
                case "1": //hexadecimal to decimal
                    System.out.println("Please enter a hexadecimal number");
                    sc.nextLine();
                    String number1 = sc.nextLine().trim().toUpperCase();

                    while (!number1.matches("[0-9A-F]+") || number1.length() > 15) {
                        if (number1.length() > 15) {
                            System.out.println("Invalid hexadecimal number (too long, max 15 characters). Try again:");
                        } else if (number1.contains(" ")) {
                            System.out.println("Invalid hexadecimal number (no spaces allowed). Try again:");
                        } else {
                            System.out.println("Invalid hexadecimal number. Try again:");
                        }
                        number1 = sc.nextLine().trim().toUpperCase();
                    }
                    System.out.println(hexadecimalToDecimal(number1));
                    continue;

                case "2": //decimal to hexadecimal
                    System.out.println("Please enter a decimal number");
                    sc.nextLine();
                    while(true) {
                        String input = sc.nextLine().trim();

                        if (input.contains(" ")) {
                            System.out.println("Invalid decimal number (no spaces allowed). Try again:");
                            continue;
                        }
                        // הוספנו בדיקת אורך כדי למנוע קריסה במספרים ענקיים
                        if (input.length() > 18) {
                            System.out.println("Invalid decimal number (too long, max 18 digits). Try again:");
                            continue;
                        }

                        try {
                            // שינינו ל-Long במקום Integer
                            long number2 = Long.parseLong(input);
                            if (number2 < 0) {
                                System.out.println("Invalid decimal number (positive only). Try again:");
                                continue;
                            }
                            System.out.println(decimalToHexadecimal(number2));
                            break;
                        } catch (NumberFormatException e) { //valid check
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

    public static long hexadecimalToDecimal(String hexadecimal) { // hexadecimal to decimal
        long decimal = 0; // שונה מ-int ל-long
        int power = 0;
        for (int i = hexadecimal.length() - 1; i >= 0; i--) {
            char ch = hexadecimal.charAt(i);
            int value;
            if (ch >= '0' && ch <= '9') {
                value = ch - '0';
            } else {
                value = ch - 'A' + 10;
            }
            // הוספנו המרה ל-long כדי שהתוצאה של Math.pow לא תיחתך
            decimal += value * (long)Math.pow(16, power);
            power++;
        }
        return decimal;
    }

    // שינינו את סוג הפרמטר ל-long
    public static String decimalToHexadecimal(long decimal) { // decimal to hexadecimal
        if (decimal == 0) {
            return "0";
        }
        String hexadecimal = "";
        while (decimal > 0) {
            // שינינו גם את השארית ל-long
            long remainder = decimal % 16;
            if (remainder < 10) {
                hexadecimal = remainder + hexadecimal;
            } else {
                hexadecimal = (char)(remainder - 10 + 'A') + hexadecimal;
            }
            decimal = decimal / 16;
        }
        return hexadecimal;
    }
}
