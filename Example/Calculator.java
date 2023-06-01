public class Calculator {
    public static void main(String[] args) {
        if (args.length != 3) {
            System.out.println(args.length);
            System.out.println(args[0]);
            System.out.println(args[1]);
            System.out.println(args[2]);
            System.out.println(args[3]);

            System.out.println("Usage: java Calculator [integer] [+|-|*|/] [integer]");
            System.exit(1);
        }

        int num1 = 0;
        int num2 = 0;
        int result = 0;
        char op = args[1].charAt(0);

        try {
            num1 = Integer.parseInt(args[0].trim());
            num2 = Integer.parseInt(args[2].trim());
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid input format");
            System.exit(1);
        }

        switch (op) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                if (num2 == 0) {
                    System.out.println("Error: Division by zero");
                    System.exit(1);
                }
                result = num1 / num2;
                break;
            default:
                System.out.println("Error: Invalid operator " + op);
                System.exit(1);
        }

        System.out.println(result);
    }
}
