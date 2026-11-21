"""
database.py
-----------
This file handles EVERYTHING related to the SQLite database.

It contains the DatabaseManager class, responsible for:
    - Connecting to the database and creating the 'students' table
    - CRUD operations: Create, Read, Update, Delete student records
    - Special report queries: total count, average marks, highest marks,
      sorted list

Why a separate file?
---------------------
main.py should only worry about the MENU and user interaction.
student.py should only worry about WHAT a student looks like.
database.py should only worry about HOW data is stored/retrieved.
This separation makes the project easier to read, test, and extend.
"""

import sqlite3


class DatabaseManager:
    """Handles all interactions with the SQLite database."""

    def __init__(self, db_name="students.db"):
        """
        Runs automatically when a DatabaseManager object is created.

        sqlite3.connect(db_name) does two things:
            1. If 'students.db' doesn't exist yet, it CREATES the file.
            2. If it already exists, it just opens a connection to it.

        This means your data is stored permanently on disk, even after
        the program closes and the computer restarts.
        """
        self.connection = sqlite3.connect(db_name)

        # A "cursor" is like a pointer/remote-control we use to run SQL commands.
        self.cursor = self.connection.cursor()

        # Make sure the table exists before we try to use it.
        self.create_table()

    def create_table(self):
        """
        Creates the 'students' table if it doesn't already exist.

        roll_number INTEGER PRIMARY KEY
            -> automatically enforces uniqueness. SQLite will refuse to
               insert a second row with a roll number that already exists.
        """
        query = """
        CREATE TABLE IF NOT EXISTS students (
            roll_number INTEGER PRIMARY KEY,
            name        TEXT    NOT NULL,
            age         INTEGER NOT NULL,
            course      TEXT    NOT NULL,
            email       TEXT    NOT NULL,
            phone       TEXT    NOT NULL,
            marks       REAL    NOT NULL
        )
        """
        self.cursor.execute(query)
        self.connection.commit()  # commit() permanently saves changes to disk

    # ------------------------------------------------------------------
    # CREATE
    # ------------------------------------------------------------------
    def insert_student(self, student):
        """
        Inserts a new student record.
        'student' is a Student object from student.py.

        Returns:
            True  -> insert succeeded
            False -> insert failed (roll number already exists)
        """
        try:
            query = """
            INSERT INTO students (roll_number, name, age, course, email, phone, marks)
            VALUES (?, ?, ?, ?, ?, ?, ?)
            """
            # The '?' marks are placeholders. Using them (instead of f-strings)
            # protects us from SQL injection attacks - a security best practice.
            self.cursor.execute(query, student.to_tuple())
            self.connection.commit()
            return True
        except sqlite3.IntegrityError:
            # This specific error is raised when the PRIMARY KEY (roll_number)
            # already exists in the table.
            return False

    # ------------------------------------------------------------------
    # READ
    # ------------------------------------------------------------------
    def get_all_students(self):
        """Returns a list of tuples - one tuple per student, all columns."""
        self.cursor.execute("SELECT * FROM students")
        return self.cursor.fetchall()

    def get_student_by_roll(self, roll_number):
        """
        Returns a single student's data (as a tuple) for the given roll
        number, or None if no such student exists.
        """
        self.cursor.execute(
            "SELECT * FROM students WHERE roll_number = ?", (roll_number,)
        )
        return self.cursor.fetchone()

    def roll_number_exists(self, roll_number):
        """Quick True/False check used before inserting a new student."""
        return self.get_student_by_roll(roll_number) is not None

    # ------------------------------------------------------------------
    # UPDATE
    # ------------------------------------------------------------------
    def update_student(self, roll_number, name, age, course, email, phone, marks):
        """
        Updates every field (except roll_number) for a given student.

        Returns:
            True  -> a row was updated
            False -> no matching roll_number was found
        """
        query = """
        UPDATE students
        SET name = ?, age = ?, course = ?, email = ?, phone = ?, marks = ?
        WHERE roll_number = ?
        """
        self.cursor.execute(query, (name, age, course, email, phone, marks, roll_number))
        self.connection.commit()

        # cursor.rowcount tells us how many rows were affected by the last query.
        return self.cursor.rowcount > 0

    # ------------------------------------------------------------------
    # DELETE
    # ------------------------------------------------------------------
    def delete_student(self, roll_number):
        """
        Deletes the student with the given roll number.

        Returns:
            True  -> a row was deleted
            False -> no matching roll_number was found
        """
        self.cursor.execute("DELETE FROM students WHERE roll_number = ?", (roll_number,))
        self.connection.commit()
        return self.cursor.rowcount > 0

    # ------------------------------------------------------------------
    # STATISTICS / REPORTS
    # ------------------------------------------------------------------
    def get_total_students(self):
        """Returns the total number of student records (an integer)."""
        self.cursor.execute("SELECT COUNT(*) FROM students")
        return self.cursor.fetchone()[0]

    def get_average_marks(self):
        """
        Returns the average marks across all students, or None if the
        table is empty (avoids a crash / division-by-zero situation).
        """
        self.cursor.execute("SELECT AVG(marks) FROM students")
        return self.cursor.fetchone()[0]

    def get_highest_marks_students(self):
        """
        Finds the single highest marks value in the table, then returns
        ALL students who scored that value (handles ties gracefully -
        e.g. two students both scoring the top marks).
        """
        self.cursor.execute("SELECT MAX(marks) FROM students")
        highest = self.cursor.fetchone()[0]

        if highest is None:  # table is empty
            return []

        self.cursor.execute("SELECT * FROM students WHERE marks = ?", (highest,))
        return self.cursor.fetchall()

    def get_students_sorted_by_marks(self):
        """Returns all students ordered from highest marks to lowest."""
        self.cursor.execute("SELECT * FROM students ORDER BY marks DESC")
        return self.cursor.fetchall()

    # ------------------------------------------------------------------
    def close(self):
        """Closes the database connection. Call this before exiting the program."""
        self.connection.close()