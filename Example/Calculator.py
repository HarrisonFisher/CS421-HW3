import sys

if len(sys.argv) != 4:
    print("Usage: python calculator.py [integer] [+|-|*|/] [integer]")
    sys.exit(1)

num1 = int(sys.argv[1])
op = sys.argv[2]
num2 = int(sys.argv[3])
result = 0

if op == '+':
    result = num1 + num2
elif op == '-':
    result = num1 - num2
elif op == '*':
    result = num1 * num2
elif op == '/':
    if num2 == 0:
        print("Error: Division by zero")
        sys.exit(1)
    result = num1 / num2
else:
    print("Error: Invalid operator " + op)
    sys.exit(1)

print(result)
