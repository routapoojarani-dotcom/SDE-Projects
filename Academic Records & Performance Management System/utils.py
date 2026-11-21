"""
utils.py
--------
This file contains small, reusable HELPER functions used across the
project:
    - Input validation (email, phone, marks)
    - Safe number input (so the program never crashes on bad input)
    - Formatted printing (success/error messages, tables, headers)

Why a separate file?
----------------------
Without this file, we would end up copy-pasting the same validation
and printing code inside main.py multiple times. Keeping it here
follows the DRY principle: "Don't Repeat Yourself".
"""

import re  # 're' = Python's built-in regular expression module,
           # used here for basic email pattern matching.


# ----------------------------------------------------------------------
# VALIDATION FUNCTIONS
# ----------------------------------------------------------------------
def validate_email(email):
    """
    Checks whether 'email' looks like a valid email address,
    e.g. someone@example.com

    The pattern (in plain English) means:
        - one or more letters/numbers/._%+- characters
        - then an @ symbol
        - then letters/numbers/.- characters (the domain)
        - then a dot
        - then 2 or more letters (the domain extension, like com/in/org)
    """
    pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    return re.match(pattern, email) is not None


def validate_phone(phone):
    """
    A valid phone number here means exactly 10 digits, with no letters,
    spaces, or symbols.
    isdigit() returns True only if every character is a number.
    """
    return phone.isdigit() and len(phone) == 10


def validate_marks(marks):
    """Marks must be between 0 and 100 (inclusive)."""
    return 0 <= marks <= 100


# ----------------------------------------------------------------------
# SAFE INPUT FUNCTIONS
# ----------------------------------------------------------------------
def get_valid_integer(prompt):
    """
    Keeps asking the user for input until they type a valid whole number.
    This prevents the program from crashing with a ValueError if the
    user accidentally types letters instead of numbers.
    """
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print_error("Invalid input! Please enter a whole number.")


def get_valid_float(prompt):
    """
    Same idea as get_valid_integer(), but allows decimal numbers.
    Used for marks, e.g. 85.5
    """
    while True:
        try:
            return float(input(prompt))
        except ValueError:
            print_error("Invalid input! Please enter a valid number (e.g. 85 or 85.5).")


# ----------------------------------------------------------------------
# DISPLAY / FORMATTING FUNCTIONS
# ----------------------------------------------------------------------
def print_success(message):
    """Prints a clearly marked success message."""
    print(f"\n[SUCCESS] {message}\n")


def print_error(message):
    """Prints a clearly marked error message."""
    print(f"\n[ERROR] {message}\n")


def print_header(title):
    """Prints a neat, centered section header - makes the terminal output
    look clean and professional instead of just plain text."""
    print("\n" + "=" * 70)
    print(title.center(70))
    print("=" * 70)


def print_students_table(students):
    """
    Prints a list of student records in a neatly aligned table.

    'students' is a list of tuples, where each tuple has the shape:
        (roll_number, name, age, course, email, phone, marks)
    This is exactly the format sqlite3 returns from SELECT queries.
    """
    if not students:
        print_error("No student records found.")
        return

    # The :<N format means "left-align this text inside a field N characters wide"
    header = (
        f"{'Roll No':<10}{'Name':<20}{'Age':<6}{'Course':<12}"
        f"{'Email':<25}{'Phone':<15}{'Marks':<8}"
    )
    line = "-" * len(header)

    print("\n" + line)
    print(header)
    print(line)

    for student in students:
        roll_number, name, age, course, email, phone, marks = student
        row = (
            f"{roll_number:<10}{name:<20}{age:<6}{course:<12}"
            f"{email:<25}{phone:<15}{marks:<8.2f}"
        )
        print(row)

    print(line)
    print(f"Total Records: {len(students)}\n")