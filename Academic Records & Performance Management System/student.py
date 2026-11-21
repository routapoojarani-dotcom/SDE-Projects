"""
student.py
-----------
This file defines the Student class.

Each Student object represents ONE student's details (roll number, name,
age, course, email, phone, marks).

Why a separate class?
----------------------
Instead of passing around 7 separate variables everywhere (roll_number,
name, age, ...), we bundle them into a single Student object. This is
the core idea of Object-Oriented Programming (OOP): group related data
and behaviour together.

Note: This class only STORES data. It does NOT talk to the database.
That job belongs to database.py. Keeping these responsibilities separate
is called "Separation of Concerns" and it's considered good design.
"""


class Student:
    """A simple class that represents a single student."""

    def __init__(self, roll_number, name, age, course, email, phone, marks):
        """
        The constructor runs automatically when we create a Student object,
        e.g. Student(101, "Riya", 20, "BCA", "riya@mail.com", "9876543210", 88.5)

        'self' refers to the object being created. Every attribute we
        attach to 'self' becomes part of that specific object.
        """
        self.roll_number = roll_number
        self.name = name
        self.age = age
        self.course = course
        self.email = email
        self.phone = phone
        self.marks = marks

    def to_tuple(self):
        """
        Converts the student's data into a tuple: (roll, name, age, ...)

        Why? Because Python's sqlite3 library expects data in tuple form
        when we insert rows into the database (see database.py).
        """
        return (
            self.roll_number,
            self.name,
            self.age,
            self.course,
            self.email,
            self.phone,
            self.marks,
        )

    def __str__(self):
        """
        A "dunder" (double underscore) method. It controls what gets
        shown when we do print(some_student_object).
        Handy for quick debugging.
        """
        return (
            f"Roll No: {self.roll_number}, Name: {self.name}, Age: {self.age}, "
            f"Course: {self.course}, Email: {self.email}, "
            f"Phone: {self.phone}, Marks: {self.marks}"
        )