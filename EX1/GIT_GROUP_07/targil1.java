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
                    System.out.println("Please enter a hexadecimal number (0-9, A-F, optional dot)");
                    sc.nextLine(); // לניקוי ה-Buffer
                    String number1 = sc.nextLine().trim().toUpperCase();

                    // הקלט יכול להכיל אותיות, מספרים ונקודה אחת מקסימום. אין מינוסים!
                    while (!number1.matches("[0-9A-F]+(\\.[0-9A-F]+)?") || number1.length() > 17) {
                        System.out.println("Invalid hexadecimal number. Try again:");
                        number1 = sc.nextLine().trim().toUpperCase();
                    }

                    System.out.println("Result: " + hexadecimalToDecimal(number1));
                    continue;

                case "2": // Decimal to Hexadecimal
                    System.out.println("Please enter a decimal number (optional dot)");
                    sc.nextLine();
                    while (true) {
                        String input = sc.nextLine().trim();

                        if (input.isEmpty() || input.equals("-") || input.contains(" ")) {
                            System.out.println("Invalid input. Try again:");
                            continue;
                        }

                        // מוודא שהמשתמש הכניס פורמט עשרוני תקין (מספרים, מינוס אופציונלי ונקודה אופציונלית)
                        if (!input.matches("-?[0-9]+(\\.[0-9]+)?") || input.length() > 20) {
                            System.out.println("Invalid decimal number. Try again:");
                            continue;
                        }

                        System.out.println("Result: " + decimalToHexadecimal(input));
                        break;
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

    // המרה מהקסדצימלי לעשרוני (מודל Fixed-Point 32.32)
    public static String hexadecimalToDecimal(String hex) {
        int dotIdx = hex.indexOf('.');
        String intH, fracH;
        // הפרדה לחלק שלם ולחלק שברי
        if (dotIdx == -1) {
            intH = hex;
            fracH = "00000000";
        } else {
            intH = hex.substring(0, dotIdx);
            fracH = hex.substring(dotIdx + 1);}
        // ריפוד (Padding) ל-8 תווים בכל צד כדי למלא את כל 64 הסיביות לחישוב מדויק
        while (intH.length() < 8) intH = "0" + intH;
        while (fracH.length() < 8) fracH += "0";
        if (fracH.length() > 8) fracH = fracH.substring(0, 8);
        String cleanHex = intH + fracH; // אורך קבוע של 16 תווים
        // המרה ידנית ל-long
        long combined = 0;
        for (int i = 0; i < cleanHex.length(); i++) {
            char ch = cleanHex.charAt(i);
            long value = (ch >= '0' && ch <= '9') ? (ch - '0') : (ch - 'A' + 10);
            combined = (combined << 4) | value;
        }
        // בדיקת ביט הסימן (ביט 63) - אם הוא 1, המספר שלילי במשלים ל-2
        boolean isNegative = combined < 0;
        if (isNegative) {
            combined = -combined; // הפיכת הערך לחיובי (ביטול המשלים ל-2)
        }
        // חילוץ החלקים חזרה מתוך ה-64 סיביות
        long intPart = combined >>> 32;
        long fracPart = combined & 0xFFFFFFFFL;
        // המרת השבר חזרה לעשרוני
        double fracVal = (double) fracPart / 4294967296.0; // חלוקה ב-2^32
        double total = intPart + fracVal;
        if (isNegative && total != 0) {
            total = -total;}
        // מונע הדפסה של ".0" אם המספר שלם בעשרוני
        if (total == (long) total) {
            return String.valueOf((long) total);
        } else {
            return String.valueOf(total);
        }
    }

    // המרה מעשרוני להקסדצימלי (מודל Fixed-Point 32.32)
    public static String decimalToHexadecimal(String decimalStr) {
        boolean isNegative = decimalStr.startsWith("-");
        if (isNegative) decimalStr = decimalStr.substring(1);
        int dotIdx = decimalStr.indexOf('.');
        String intStr = dotIdx == -1 ? decimalStr : decimalStr.substring(0, dotIdx);
        String fracStr = dotIdx == -1 ? "" : decimalStr.substring(dotIdx + 1);
        // המרה ידנית לחלוטין של המחרוזת למספר שלם
        long intVal = 0;
        for (int i = 0; i < intStr.length(); i++) {
            intVal = intVal * 10 + (intStr.charAt(i) - '0');}
        // המרה ידנית לחלוטין של המחרוזת לשבר
        double fracVal = 0.0;
        double div = 10.0;
        for (int i = 0; i < fracStr.length(); i++) {
            fracVal += (fracStr.charAt(i) - '0') / div;
            div *= 10.0;
        }
        // המרת השבר העשרוני ל-32 סיביות של שבר בינארי
        long fractionBits = (long) (fracVal * 4294967296.0); // הכפלה ב-2^32
        // איחוד החלק השלם (מוזז 32 סיביות שמאלה) עם החלק השברי
        long combined = (intVal << 32) | (fractionBits & 0xFFFFFFFFL);
        // אם המספר המקורי היה שלילי, נחיל את המשלים ל-2 על כל 64 הסיביות יחד
        if (isNegative) {
            combined = -combined;}
        // המרה חזרה למחרוזת הקסדצימלית ידנית - יצירת מבנה 16 התווים המלא
        String hex = "";
        long temp = combined;
        for (int i = 0; i < 16; i++) {
            if (i == 8) hex = "." + hex; // מיקום הנקודה בדיוק באמצע
            int rem = (int)(temp & 15);
            char hexDigit = (rem < 10) ? (char)(rem + '0') : (char)(rem - 10 + 'A');
            hex = hexDigit + hex;
            temp >>>= 4;
        }// --- ניקוי אסתטי של התצוגה ---
        String[] parts = hex.split("\\.");
        String intHex = parts[0].replaceFirst("^0+", ""); // מחיקת אפסים מובילים בשלם
        if (intHex.isEmpty()) intHex = "0";               // אם המספר היה 0
        String fracHex = parts[1].replaceAll("0+$", "");  // מחיקת אפסים נגררים בשבר
        if (fracHex.isEmpty()) {
            return intHex; // מחזיר מספר שלם נקי (למשל: FF או FFFFFFFE)
        } else {
            return intHex + "." + fracHex; // מחזיר מספר שברי נקי (למשל: A.8)
        }
    }
}