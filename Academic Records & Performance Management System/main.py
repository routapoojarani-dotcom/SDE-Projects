"""
main.py
-------
This is the ENTRY POINT of the Student Management System.

It displays a menu-driven terminal interface and connects all the
other modules together:
    database.py -> stores/retrieves data (SQLite)
    student.py  -> represents one student (OOP class)
    utils.py    -> validation + formatted printing helpers

To run the program:
    python main.py
"""

from database import DatabaseManager
from student import Student
from utils import (
    validate_email,
    validate_phone,
    validate_marks,
    get_valid_integer,
    get_valid_float,
    print_success,
    print_error,
    print_header,
    print_students_table,
)


class StudentManagementSystem:
    """
    This class ties together the menu logic and the database operations.

    Why wrap this in a class instead of plain functions + global variables?
    It keeps the DatabaseManager object (self.db) neatly contained inside
    the system instead of floating around as a global variable, and makes
    the code easier to extend later (e.g. adding a GUI on top).
    """

    def __init__(self):
        # One DatabaseManager instance is created and reused for the
        # entire program session (it holds the open database connection).
        self.db = DatabaseManager()

    # --------------------------------------------------------------
    # 1. ADD STUDENT
    # --------------------------------------------------------------
    def add_student(self):
        print_header("ADD NEW STUDENT")
        try:
            roll_number = get_valid_integer("Enter Roll Number: ")

            # Check uniqueness FIRST, before asking for the rest of the details.
            if self.db.roll_number_exists(roll_number):
                print_error(f"Roll Number {roll_number} already exists!")
                return

            name = input("Enter Name: ").strip()
            age = get_valid_integer("Enter Age: ")
            course = input("Enter Course: ").strip()

            email = input("Enter Email: ").strip()
            if not validate_email(email):
                print_error("Invalid email format! Example: name@example.com")
                return

            phone = input("Enter Phone Number (10 digits): ").strip()
            if not validate_phone(phone):
                print_error("Phone number must contain exactly 10 digits!")
                return

            marks = get_valid_float("Enter Marks (0-100): ")
            if not validate_marks(marks):
                print_error("Marks must be between 0 and 100!")
                return

            # All checks passed -> build a Student object and save it.
            student = Student(roll_number, name, age, course, email, phone, marks)
            if self.db.insert_student(student):
                print_success(f"Student '{name}' added successfully!")
            else:
                print_error("Could not add student. Roll number might already exist.")

        except Exception as e:
            # A safety net for anything unexpected, so the whole program
            # never crashes because of one bad operation.
            print_error(f"Something went wrong: {e}")

    # --------------------------------------------------------------
    # 2. VIEW ALL STUDENTS
    # --------------------------------------------------------------
    def view_all_students(self):
        print_header("ALL STUDENTS")
        students = self.db.get_all_students()
        print_students_table(students)

    # --------------------------------------------------------------
    # 3. SEARCH STUDENT BY ROLL NUMBER
    # --------------------------------------------------------------
    def search_student(self):
        print_header("SEARCH STUDENT")
        roll_number = get_valid_integer("Enter Roll Number to search: ")
        student = self.db.get_student_by_roll(roll_number)

        if student:
            print_students_table([student])
        else:
            print_error(f"No student found with Roll Number {roll_number}")

    # --------------------------------------------------------------
    # 4. UPDATE STUDENT DETAILS
    # --------------------------------------------------------------
    def update_student(self):
        print_header("UPDATE STUDENT DETAILS")
        roll_number = get_valid_integer("Enter Roll Number to update: ")
        existing = self.db.get_student_by_roll(roll_number)

        if not existing:
            print_error(f"No student found with Roll Number {roll_number}")
            return

        print("Leave a field blank and press Enter to keep its current value.\n")
        # existing looks like: (roll_number, name, age, course, email, phone, marks)
        _, old_name, old_age, old_course, old_email, old_phone, old_marks = existing

        try:
            name = input(f"Name [{old_name}]: ").strip() or old_name

            age_input = input(f"Age [{old_age}]: ").strip()
            age = int(age_input) if age_input else old_age

            course = input(f"Course [{old_course}]: ").strip() or old_course

            email_input = input(f"Email [{old_email}]: ").strip()
            email = email_input if email_input else old_email
            if not validate_email(email):
                print_error("Invalid email format! Update cancelled.")
                return

            phone_input = input(f"Phone [{old_phone}]: ").strip()
            phone = phone_input if phone_input else old_phone
            if not validate_phone(phone):
                print_error("Phone number must contain exactly 10 digits! Update cancelled.")
                return

            marks_input = input(f"Marks [{old_marks}]: ").strip()
            marks = float(marks_input) if marks_input else old_marks
            if not validate_marks(marks):
                print_error("Marks must be between 0 and 100! Update cancelled.")
                return

            if self.db.update_student(roll_number, name, age, course, email, phone, marks):
                print_success("Student details updated successfully!")
            else:
                print_error("Update failed.")

        except ValueError:
            print_error("Invalid input entered. Update cancelled.")

    # --------------------------------------------------------------
    # 5. DELETE STUDENT
    # --------------------------------------------------------------
    def delete_student(self):
        print_header("DELETE STUDENT")
        roll_number = get_valid_integer("Enter Roll Number to delete: ")

        existing = self.db.get_student_by_roll(roll_number)
        if not existing:
            print_error(f"No student found with Roll Number {roll_number}")
            return

        confirm = input(
            f"Are you sure you want to delete Roll Number {roll_number}? (y/n): "
        ).strip().lower()

        if confirm == "y":
            self.db.delete_student(roll_number)
            print_success("Student deleted successfully!")
        else:
            print("Deletion cancelled.")

    # --------------------------------------------------------------
    # 6. DISPLAY TOTAL STUDENTS
    # --------------------------------------------------------------
    def display_total_students(self):
        print_header("TOTAL STUDENTS")
        total = self.db.get_total_students()
        print(f"Total number of students in the system: {total}\n")

    # --------------------------------------------------------------
    # 7. DISPLAY STUDENTS WITH HIGHEST MARKS
    # --------------------------------------------------------------
    def display_highest_marks(self):
        print_header("STUDENT(S) WITH HIGHEST MARKS")
        students = self.db.get_highest_marks_students()
        print_students_table(students)

    # --------------------------------------------------------------
    # 8. DISPLAY AVERAGE MARKS
    # --------------------------------------------------------------
    def display_average_marks(self):
        print_header("AVERAGE MARKS")
        average = self.db.get_average_marks()
        if average is None:
            print_error("No student records found.")
        else:
            print(f"Average marks of all students: {average:.2f}\n")

    # --------------------------------------------------------------
    # 9. DISPLAY STUDENTS SORTED BY MARKS
    # --------------------------------------------------------------
    def display_sorted_by_marks(self):
        print_header("STUDENTS SORTED BY MARKS (HIGH TO LOW)")
        students = self.db.get_students_sorted_by_marks()
        print_students_table(students)

    # --------------------------------------------------------------
    # MENU DISPLAY + MAIN LOOP
    # --------------------------------------------------------------
    def show_menu(self):
        """Prints the list of available options."""
        print_header("STUDENT MANAGEMENT SYSTEM")
        print("1. Add Student")
        print("2. View All Students")
        print("3. Search Student by Roll Number")
        print("4. Update Student Details")
        print("5. Delete Student")
        print("6. Display Total Students")
        print("7. Display Students with Highest Marks")
        print("8. Display Average Marks")
        print("9. Display Students Sorted by Marks")
        print("10. Exit")
        print("-" * 70)

    def run(self):
        """
        The heart of the program: an infinite loop that keeps showing
        the menu and running whichever option the user picks, until
        they choose to exit (option 10).
        """
        while True:
            self.show_menu()
            choice = input("Enter your choice (1-10): ").strip()

            if choice == "1":
                self.add_student()
            elif choice == "2":
                self.view_all_students()
            elif choice == "3":
                self.search_student()
            elif choice == "4":
                self.update_student()
            elif choice == "5":
                self.delete_student()
            elif choice == "6":
                self.display_total_students()
            elif choice == "7":
                self.display_highest_marks()
            elif choice == "8":
                self.display_average_marks()
            elif choice == "9":
                self.display_sorted_by_marks()
            elif choice == "10":
                print("\nThank you for using Student Management System. Goodbye!\n")
                self.db.close()
                break
            else:
                print_error("Invalid choice! Please enter a number between 1 and 10.")


# This special check ensures the code below only runs when main.py is
# executed directly (e.g. `python main.py`), and NOT when this file is
# imported as a module into some other script.
if __name__ == "__main__":
    app = StudentManagementSystem()
    app.run()