# Academic Records & Performance Management System

A clean, menu-driven **Student Management System** built with core Python and
SQLite — no frameworks, no external libraries. It demonstrates real
Object-Oriented Programming, proper file organization, input validation, and
full CRUD (Create, Read, Update, Delete) operations backed by a **permanent**
database.

This project is intentionally built with **beginner-readable code** but
**intermediate-level structure** — the kind of separation-of-concerns you'd
see in a real production codebase, which is exactly what makes it stand out
on a resume.

---

## 📌 Features

| # | Feature                              |
|---|---------------------------------------|
| 1 | Add Student                           |
| 2 | View All Students                     |
| 3 | Search Student by Roll Number         |
| 4 | Update Student Details                |
| 5 | Delete Student                        |
| 6 | Display Total Students                |
| 7 | Display Students with Highest Marks   |
| 8 | Display Average Marks                 |
| 9 | Display Students Sorted by Marks      |
| 10| Exit                                  |

**Validation built in:**
- ✅ Roll Number must be unique (enforced by the database itself)
- ✅ Email format validation (regex)
- ✅ Phone number must be exactly 10 digits
- ✅ Marks must be between 0–100
- ✅ Graceful handling of invalid/non-numeric input (no crashes)

---

## 🗂️ Folder Structure

```
student_management_system/
│
├── main.py          # Entry point – menu-driven interface, ties everything together
├── student.py        # Student class (OOP data model)
├── database.py        # DatabaseManager class – all SQLite CRUD operations
├── utils.py           # Validation functions + formatted printing helpers
├── students.db         # SQLite database file (auto-created on first run)
└── README.md            # This file
```

**Why split into 4 files instead of one big script?**
This mirrors how real-world software is structured — each file has ONE job:

- `student.py` → what a student **is** (data)
- `database.py` → how data is **stored/retrieved**
- `utils.py` → reusable **helpers** (validation, printing)
- `main.py` → the **user-facing menu** that wires it all together

This is called **Separation of Concerns**, and it's one of the first things
interviewers look for in a "project structure" discussion.

---

## 🛠️ Tech Stack

- **Language:** Python 3.x
- **Database:** SQLite3 (built into Python's standard library — no install needed)
- **IDE used:** VS Code
- **Libraries used:** `sqlite3`, `re` — both are part of Python's standard
  library. No pip installs, no Tkinter/Flask/Django.

---

## ▶️ How to Run

1. Make sure Python 3 is installed:
   ```bash
   python --version
   ```
2. Download/clone the project folder.
3. Open the folder in VS Code (or any terminal).
4. Run:
   ```bash
   python main.py
   ```
5. A `students.db` file will be automatically created in the same folder the
   first time you run the program. All your data is saved there permanently
   — closing the program does **not** delete your data.

---

## 🧱 SQL Table Structure

The table is created automatically by `database.py` when the program first
runs. Here is the exact SQL:

```sql
CREATE TABLE IF NOT EXISTS students (
    roll_number INTEGER PRIMARY KEY,
    name        TEXT    NOT NULL,
    age         INTEGER NOT NULL,
    course      TEXT    NOT NULL,
    email       TEXT    NOT NULL,
    phone       TEXT    NOT NULL,
    marks       REAL    NOT NULL
);
```

**Key design decisions:**
- `roll_number INTEGER PRIMARY KEY` → SQLite automatically rejects any
  attempt to insert a duplicate roll number, giving us uniqueness "for free"
  at the database level (backed up by an application-level check too).
- `NOT NULL` on every column → prevents incomplete/empty records.
- `marks REAL` → allows decimal marks like `76.25`, not just whole numbers.

---

## 📄 File-by-File Explanation

### 1. `student.py` — The Student Class (OOP Model)

This file defines a `Student` class. Each `Student` object holds one
student's data: `roll_number`, `name`, `age`, `course`, `email`, `phone`,
`marks`.

**Why a class instead of just variables or a dictionary?**
Because OOP lets us treat "a student" as one clean unit of data that we can
pass around the program (e.g. into `db.insert_student(student)`) instead of
juggling 7 loose variables everywhere.

**Line-by-line explanation:**

```python
class Student:
```
Defines a new class called `Student`. Think of a class as a *blueprint* — it
describes what every student object will look like, but it isn't an actual
student until we create one.

```python
def __init__(self, roll_number, name, age, course, email, phone, marks):
```
This is the **constructor**. It runs automatically every time we write
`Student(101, "Riya", 20, ...)`. `self` refers to the specific object being
built.

```python
self.roll_number = roll_number
```
...and so on for each field — this stores the value passed in as an
**attribute** on the object, so we can later access it as `student.name`,
`student.marks`, etc.

```python
def to_tuple(self):
    return (self.roll_number, self.name, ...)
```
Converts the object into a plain Python tuple. This exists because
`sqlite3`'s `execute()` function expects data in tuple form when running
`INSERT` queries — this method bridges our OOP object and the database
layer.

```python
def __str__(self):
    return f"Roll No: {self.roll_number}, ..."
```
A **dunder (double underscore) method**. It defines what `print(student)`
shows. Without it, `print()` would show something unreadable like
`<student.Student object at 0x7f...>`.

---

### 2. `database.py` — The Database Layer (SQLite CRUD)

This file defines `DatabaseManager`, a class that owns the database
connection and exposes clean methods like `insert_student()`,
`get_all_students()`, `delete_student()`, etc. **No other file talks to
SQLite directly** — everything goes through this class.

**Line-by-line explanation of the key parts:**

```python
self.connection = sqlite3.connect(db_name)
```
Opens (or creates, if it doesn't exist) the `students.db` file. This is what
makes the storage **permanent** — data survives after the program closes.

```python
self.cursor = self.connection.cursor()
```
A `cursor` is the object we use to actually run SQL commands against the
connection — think of it as a "remote control" for the database.

```python
self.cursor.execute(query)
self.connection.commit()
```
`execute()` runs the SQL. `commit()` **permanently saves** the change to
disk. Forgetting `commit()` is a classic beginner bug — without it, your
changes would be lost.

```python
def insert_student(self, student):
    try:
        ...
        self.cursor.execute(query, student.to_tuple())
        self.connection.commit()
        return True
    except sqlite3.IntegrityError:
        return False
```
We wrap the insert in a `try/except`. If the roll number already exists,
SQLite raises `IntegrityError` (because `roll_number` is a `PRIMARY KEY`).
We catch that specific error and return `False` instead of letting the
whole program crash — this is **exception handling** in action.

```python
query = "INSERT INTO students (...) VALUES (?, ?, ?, ?, ?, ?, ?)"
self.cursor.execute(query, student.to_tuple())
```
Notice the `?` placeholders instead of directly inserting values into the
string with an f-string. This is called a **parameterized query**, and it
protects against SQL Injection — a real security best practice worth
mentioning in interviews.

```python
def get_highest_marks_students(self):
    self.cursor.execute("SELECT MAX(marks) FROM students")
    highest = self.cursor.fetchone()[0]
    ...
    self.cursor.execute("SELECT * FROM students WHERE marks = ?", (highest,))
    return self.cursor.fetchall()
```
First we find the single highest mark using SQL's `MAX()`. Then we fetch
**every** student who has that mark — this correctly handles ties (e.g. two
students both scoring 92).

```python
def close(self):
    self.connection.close()
```
Closes the database connection cleanly when the program exits — good
practice to avoid locked files or resource leaks.

---

### 3. `utils.py` — Validation & Display Helpers

This file has two jobs: **validating user input** and **formatting
terminal output** so the program looks clean and professional.

**Line-by-line explanation of key parts:**

```python
import re
```
`re` is Python's built-in **regular expressions** module, used to check if
an email string matches a valid pattern.

```python
def validate_email(email):
    pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    return re.match(pattern, email) is not None
```
This checks the email has the shape `something@something.something` (like
`name@example.com`). `re.match()` returns a match object if it fits the
pattern, or `None` if it doesn't — so we convert that to a clean
`True`/`False`.

```python
def validate_phone(phone):
    return phone.isdigit() and len(phone) == 10
```
`isdigit()` ensures every character is a number (no letters, no `+91`, no
spaces). `len(phone) == 10` ensures it's exactly 10 digits long.

```python
def validate_marks(marks):
    return 0 <= marks <= 100
```
A **chained comparison** — Python lets us write `0 <= marks <= 100` instead
of `marks >= 0 and marks <= 100`. Both mean the same thing.

```python
def get_valid_integer(prompt):
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print_error("Invalid input! Please enter a whole number.")
```
This is a **safe input loop**. If the user types letters instead of a
number, `int()` raises `ValueError`. Instead of crashing, we catch it, show
an error, and **loop back** to ask again. This is exception handling used
to make the program more forgiving of human error.

```python
def print_students_table(students):
    header = f"{'Roll No':<10}{'Name':<20}..."
```
`{'Roll No':<10}` means: take the text `'Roll No'` and left-align it inside
a field that's **10 characters wide**, padding with spaces. Doing this for
every column is what creates the neat, aligned table you see in the
terminal — similar to how a spreadsheet lines up columns.

---

### 4. `main.py` — The Entry Point (Menu-Driven Interface)

This file defines `StudentManagementSystem`, a class that owns one
`DatabaseManager` instance and exposes one method per menu option (e.g.
`add_student()`, `delete_student()`), plus a `run()` method containing the
main loop.

**Line-by-line explanation of key parts:**

```python
from database import DatabaseManager
from student import Student
from utils import (...)
```
These imports pull in the classes/functions from our other files. This is
what makes the 4-file structure actually work together as one program.

```python
class StudentManagementSystem:
    def __init__(self):
        self.db = DatabaseManager()
```
When we create a `StudentManagementSystem` object, it immediately opens (or
creates) the database connection and keeps it alive for the whole program
session — we don't reconnect for every single operation.

```python
def add_student(self):
    ...
    roll_number = get_valid_integer("Enter Roll Number: ")
    if self.db.roll_number_exists(roll_number):
        print_error(f"Roll Number {roll_number} already exists!")
        return
```
We check for a duplicate roll number **before** asking for the rest of the
details — this saves the user from typing everything only to be rejected at
the very end.

```python
    email = input("Enter Email: ").strip()
    if not validate_email(email):
        print_error("Invalid email format!")
        return
```
This pattern — **collect input → validate → return early on failure** —
repeats throughout `main.py`. It's called a **guard clause**, and it keeps
the code flat and readable instead of deeply nested `if/else` blocks.

```python
except Exception as e:
    print_error(f"Something went wrong: {e}")
```
A final safety net around `add_student()`. Even if something unexpected
happens, the program shows a friendly error instead of crashing completely.

```python
def run(self):
    while True:
        self.show_menu()
        choice = input("Enter your choice (1-10): ").strip()
        if choice == "1":
            self.add_student()
        ...
        elif choice == "10":
            self.db.close()
            break
```
This is the **main loop** — the heart of any menu-driven program. It keeps
showing the menu and dispatching to the right method based on user input,
until option `10` (Exit) sets `break` and ends the loop. `self.db.close()`
is called right before exiting to cleanly close the database connection.

```python
if __name__ == "__main__":
    app = StudentManagementSystem()
    app.run()
```
This checks whether the file is being run **directly** (`python main.py`)
versus being **imported** into another file. Code inside this block only
runs in the first case — a standard Python convention worth knowing for
interviews.

---

## 🖥️ Sample Output (Text Format)

```
======================================================================
                      STUDENT MANAGEMENT SYSTEM
======================================================================
1. Add Student
2. View All Students
3. Search Student by Roll Number
4. Update Student Details
5. Delete Student
6. Display Total Students
7. Display Students with Highest Marks
8. Display Average Marks
9. Display Students Sorted by Marks
10. Exit
----------------------------------------------------------------------
Enter your choice (1-10): 1

======================================================================
                           ADD NEW STUDENT
======================================================================
Enter Roll Number: 101
Enter Name: Riya Sharma
Enter Age: 20
Enter Course: BCA
Enter Email: riya@mail.com
Enter Phone Number (10 digits): 9876543210
Enter Marks (0-100): 88.5

[SUCCESS] Student 'Riya Sharma' added successfully!
```

**Trying to add a duplicate roll number:**
```
Enter your choice (1-10): 1
Enter Roll Number: 101

[ERROR] Roll Number 101 already exists!
```

**Viewing all students:**
```
======================================================================
                             ALL STUDENTS
======================================================================

------------------------------------------------------------------------------------------------
Roll No   Name                Age   Course      Email                    Phone          Marks
------------------------------------------------------------------------------------------------
101       Riya Sharma         20    BCA         riya@mail.com            9876543210     88.50
102       Aman Verma          21    BSc CS      aman@mail.com            9123456780     92.00
103       Karan Singh         19    BCA         karan@mail.com           9988776655     92.00
104       Priya Nair          22    MCA         priya@mail.com           9090909090     76.25
------------------------------------------------------------------------------------------------
Total Records: 4
```

**Students with highest marks (handles ties correctly):**
```
======================================================================
                    STUDENT(S) WITH HIGHEST MARKS
======================================================================

------------------------------------------------------------------------------------------------
Roll No   Name                Age   Course      Email                    Phone          Marks
------------------------------------------------------------------------------------------------
102       Aman Verma          21    BSc CS      aman@mail.com            9123456780     92.00
103       Karan Singh         19    BCA         karan@mail.com           9988776655     92.00
------------------------------------------------------------------------------------------------
Total Records: 2
```

**Average marks:**
```
======================================================================
                            AVERAGE MARKS
======================================================================
Average marks of all students: 87.19
```

**Updating a student (blank = keep current value):**
```
======================================================================
                        UPDATE STUDENT DETAILS
======================================================================
Enter Roll Number to update: 104
Leave a field blank and press Enter to keep its current value.

Name [Priya Nair]:
Age [22]:
Course [MCA]:
Email [priya@mail.com]: priya.n@mail.com
Phone [9090909090]:
Marks [76.25]: 80

[SUCCESS] Student details updated successfully!
```

**Deleting a student:**
```
======================================================================
                            DELETE STUDENT
======================================================================
Enter Roll Number to delete: 104
Are you sure you want to delete Roll Number 104? (y/n): y

[SUCCESS] Student deleted successfully!
```

**Invalid input handled gracefully:**
```
Enter your choice (1-10): 99

[ERROR] Invalid choice! Please enter a number between 1 and 10.
```

---

## 🚀 Possible Future Enhancements

- Export student records to a CSV file
- Add a "Search by Name" feature (partial match)
- Add pagination for large student lists
- Add unit tests (`unittest` / `pytest`)
- Build a Tkinter GUI or Flask web interface on top of the same
  `database.py`/`student.py` logic (this project's structure was designed
  to make that easy — the UI layer is already separated from the logic)

---