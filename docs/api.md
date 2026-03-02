# Content

Important

* [copy/move-semantics](#copymove-semantics)
* [thread-safety](#thread-safety)
* [destructor-behaviour](#destructor-behaviour)
* [journal](#journal)

Member functions

* [constructor](#constructor---filemanagerfile_path)

Element access

* [read](#read---readindex)
* [front](#front---front)
* [back](#back---back)
* [all](#all---all)

Capacity

* [size](#size---size)
* [empty](#empty--empty)

Modifiers

* [append](#append---appendargs)
* [overwrite](#overwrite---overwriteindex-args)
* [erase](#erase---eraseindex)
* [clear](#clear--clear)

Persistence

* [flush](#flush---flush)
* [commit](#commit---commit)

# -- Important --

## THREAD-SAFETY

The class is **not thread safe**.

## DESTRUCTOR-BEHAVIOUR

**Changes are not saved** when a filemanger object is destroyed. Explicit persistance is required via [flush](#flush---flush) or [commit](#commit---commit).

## COPY/MOVE-SEMANTICS

The class is **move-only**. Trying to copy a filemanager object won't work due to the nature of the design.

## JOURNAL

The journal file is kept in the same directory as the main file. Its name follows a simple rule: main file name + `_journal`. It always uses the `.log` extension.

### Example
Main file: `text.txt` in `C:/dir/text.txt`  
Journal file: `text_journal.log` in `C:/dir/text_journal.log`

# -- MEMBER FUNCTIONS --

## CONSTRUCTOR - filemanager(file_path)

### Description

Creates a new filemanager instance.

***Scans entire file*** *during instantiation as a necessary step for setting important internal systems and variables.* ***Can cause delayed program starts*** *depending on the size of the managed file.*

### Parameters

| Parameter   | Type                    | Description    | Notes                                                    |
| ----------- | ----------------------- | -------------- | -------------------------------------------------------- |
| `file_path` | `std::filesystem::path` | Path to a file | File and parent directories will be created if necessary |

### Exceptions

| Exception            | Cause                                           | Fix                                                                                        |
| -------------------- | ----------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `fm::open_error`     | Manager was unable to read file content         | Ensure that nothing is locking the file and that the program is run with enough permission |
| `fm::occupied_error` | Two managers tried to manage the same file path | Either pass by reference/pointer or use move semantics                                     |

### Code sample

```cpp
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
}
```

# -- ELEMENT ACCESS --

## READ - `read(index)`

### Description

Retrieves the content of a specified line.

*Jumps to the position of a specified line and retrieves the content on the storage drive.*

### Parameters

| Parameter | Datatype | Description        | Notes       |
| --------- | -------- | ------------------ | ----------- |
| `index`   | `size_t` | Which line to read | Starts at 0 |

### Returns

| Type          | Description    | Notes |
| ------------- | -------------- | ----- |
| `std::string` | Specified line |       |

### Exceptions

| Exception         | Cause                                       | Fix                                                                                        |
| ----------------- | ------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `fm::index_error` | Line does not exist                         | Check bounds with `size()` or `empty()` before access                                      |
| `fm::open_error`  | Manager was unable to open file for reading | Ensure that nothing is locking the file and that the program is run with enough permission |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    std::cout << file.read(1);
}
```

### Output

```powershell
World
```

## FRONT - `front()`

### Description

Retrieves the content at the first line.

*Retrieves the content on the storage drive at a cached index which represents the first line*

### Returns

| Type          | Description | Notes |
| ------------- | ----------- | ----- |
| `std::string` | First line  |       |

### Exceptions

| Exception         | Cause                                       | Fix                                                                                        |
| ----------------- | ------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `fm::empty_error` | File is empty                               | Check bounds with `size()` or `empty()` before access                                      |
| `fm::open_error`  | Manager was unable to open file for reading | Ensure that nothing is locking the file and that the program is run with enough permission |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    std::cout << file.front();
}
```

### Output

```powershell
Hello
```

## BACK - `back()`

### Description

Retrieves the content at the last line.

*Retrieves the content on the storage drive at a cached index which represents the last line*

### Returns

| Type          | Description | Notes |
| ------------- | ----------- | ----- |
| `std::string` | Last line   |       |

### Exceptions

| Exception         | Cause                                       | Fix                                                                                        |
| ----------------- | ------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `fm::empty_error` | File is empty                               | Check bounds with `size()` or `empty()` before access                                      |
| `fm::open_error`  | Manager was unable to open file for reading | Ensure that nothing is locking the file and that the program is run with enough permission |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    std::cout << file.back();
}
```

### Output

```powershell
!
```

## ALL - `all()`

### Description

Returns a chronological copy of every line.

*Iterates through every line and copies it into a vector. Finishes a tiny bit slower than expected because the CPU has to perform "jumps" over erased indices.*

### Returns

| Type                       | Description           | Notes        |
| -------------------------- | --------------------- | ------------ |
| `std::vector<std::string>` | All lines in the file | Can be empty |

### Exceptions

| Exception        | Cause                                       | Fix                                                                                        |
| ---------------- | ------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `fm::open_error` | Manager was unable to open file for reading | Ensure that nothing is locking the file and that the program is run with enough permission |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    for (std::string line : file.all()) {
      std::cout << line << "\n";
    }
}
```

### Output

```powershell
Hello
World
!
```

# -- CAPACITY --

## SIZE - `size()`

### Description

Returns the number of present lines.

*Not commited appends also account for the number of present lines*

### Returns

| Type     | Description             | Notes |
| -------- | ----------------------- | ----- |
| `size_t` | Number of present lines |       |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    std::cout << file.size();
}
```

### Output

```
3
```

## EMPTY - `empty()`

### Description

Returns true if the file is empty.

*Not commited, erased lines also account for whether the result is true*

### Returns

| Type   | Description               | Notes |
| ------ | ------------------------- | ----- |
| `bool` | Whether the file is empty |       |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.clear();
    std::cout << file.empty();
}
```

### Output

```
1 
```

Bools are displayed as 1 (true) or 0 (false) on the console

# -- MODIFIERS --

## APPEND - `append(args)`

### Description

Adds data as a new line at the end of the file.

*Appended content is stored in memory.* ***Does not update the file*** *(Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).*

### Parameters

| Parameter | Datatype  | Description                  | Notes                                                             |
| --------- | --------- | ---------------------------- | ----------------------------------------------------------------- |
| `args`    | `Args...` | One or more values to append | Must not contain `\n` or `\r` and must be convertable to a string |

### Returns

| Type   | Description | Notes |
| ------ | ----------- | ----- |
| `void` |             |       |

### Exceptions

| Exception          | Cause                                                       | Fix                                                                                             |
| ------------------ | ----------------------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `fm::input_error`  | Input contains newline character (`\n` or `\r`)             | Remove `\n` and `\r` occurrences when using `append()` or `overwrite()`                         |
| `fm::locked_error` | All attempts to save failed, manager entered read-only mode | Ensure nothing is locking the journal and main file and run the program with enough permissions |

### Code sample

```cpp
#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.append("hello", "world", '!', 11);
    std::cout << file.back();
}
```

### Output

```powershell
helloworld!11
```

## OVERWRITE - `overwrite(index, args)`

### Description

Overwrites a line with new text.

*Overwrite is only stored in memory.* *Does not update the file (Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).*

### Parameters

| Parameter | Datatype  | Description                       | Notes                                                             |
| --------- | --------- | --------------------------------- | ----------------------------------------------------------------- |
| `index`   | `size_t`  | Line to overwrite                 | Starts at 0                                                       |
| `args`    | `Args...` | Values to overwrite the line with | Must not contain `\n` or `\r` and must be convertable to a string |

### Returns

| Type   | Description | Notes |
| ------ | ----------- | ----- |
| `void` |             |       |

### Exceptions

| Exception          | Cause                                                       | Fix                                                                                             |
| ------------------ | ----------------------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `fm::input_error`  | Input contains newline character (`\n` or `\r`)             | Remove `\n` and `\r` occurrences when using `append()` or `overwrite()`                         |
| `fm::index_error`  | Specified line does not exist                               | Check bounds with `size()` or `empty()` before access                                           |
| `fm::locked_error` | All attempts to save failed, manager entered read-only mode | Ensure nothing is locking the journal and main file and run the program with enough permissions |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.overwrite(0, "Good", "bye");
    std::cout << file.read(0);
}
```

### Output

```powershell
Goodbye
```

## ERASE - `erase(index)`

### Description

Deletes a line, shifting later lines down to fill the gap.

*Delete is only stored in memory.* ***Does not update the file*** *(Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).*

### Parameters

| Parameter | Datatype | Description   | Notes       |
| --------- | -------- | ------------- | ----------- |
| `index`   | `size_t` | Line to erase | Starts at 0 |

### Returns

| Type   | Description | Notes |
| ------ | ----------- | ----- |
| `void` |             |       |

### Exceptions

| Exception          | Cause                                                       | Fix                                                                                             |
| ------------------ | ----------------------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `fm::index_error`  | Specified line does not exist                               | Check bounds with `size()` or `empty()` before access                                           |
| `fm::locked_error` | All attempts to save failed, manager entered read-only mode | Ensure nothing is locking the journal and main file and run the program with enough permissions |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.erase(0);
    std::cout << file.read(0) << "\n";
    std::cout << file.size();
}
```

### Output

```powershell
World
2
```

## CLEAR- `clear()`

### Description

Delete all lines, effectively making the file empty.

*Clear is only stored in memory.* ***Does not update the file*** *(Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).*

### Returns

| Type   | Description | Notes |
| ------ | ----------- | ----- |
| `void` |             |       |

### Exceptions

| Exception          | Cause                                                       | Fix                                                                                             |
| ------------------ | ----------------------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `fm::locked_error` | All attempts to save failed, manager entered read-only mode | Ensure nothing is locking the journal and main file and run the program with enough permissions |

### Code sample

```cpp
/*
    Imagine a file 'data.txt' with following lines:
    (1) Hello
    (2) World
    (3) !  
*/

#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.clear();
    std::cout << file.size();
}
```

### Output

```powershell
0
```

# -- PERSISTENCE --

## FLUSH - `flush()`

### Description

Records all changes in a journal, making recovery possible after a crash.

***Changes won't be written to the main file*** *(see [commit()](#commit---commit)).* ***Can cause delays*** *during program start if changes are not committed occasionaly.*

### Returns

| Type   | Description                     | Notes                                                         |
| ------ | ------------------------------- | ------------------------------------------------------------- |
| `bool` | Whether flushing was successful | Handle possible failures if the working directory is unstable |

### Code sample

```cpp
#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.append("this data will be recovered!");
    file.flush();

    // Imagine abort() is called once here, regardless whether the program
    // is restarted, to simulate a random crash and test the recovery.

    for (std::string line : file.all()) {
        std::cout << line << "\n";
    }
}
```

### Output (2nd start)

```powershell
this data will be recovered!
this data will be recovered!
```

First line was recovered from the journal and loaded into memory during construction. The object therefore started with the memory of the previous filemanager instance before the program crashed.

Second line was added just now because after running the program for the second time, we also call `file.append("this data will be recovered!");` for the second time.

### File content

```

```

## COMMIT - `commit()`

### Description

Deletes the journal and applies all changes to the main file.

*Triggers reconstruction internals to make up for the new file.* *It's* ***discouraged to repeatedly call*** `commit()`. *It can have a* ***noticeable impact on performance*** *if used carelessly on big files.*

### Returns

| Type   | Description                       | Notes                                                         |
| ------ | --------------------------------- | ------------------------------------------------------------- |
| `bool` | Whether committing was successful | Handle possible failures if the working directory is unstable |

## Exceptions

| Exception          | Cause                                                 | Fix                                                                                                                               |
| ------------------ | ----------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| `fm::delete_error` | Failed to delete journal file after successful commit | **Delete journal file manually!** Ensure that program is run with enough permissions and that nothing is locking the journal file |

## Code sample

```cpp
#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.append("this data will be written!");
    file.commit();
    std::cout << file.front();
}
```

### Output

```
this data will be written!
```

### File content

```
this data will be written!
```
