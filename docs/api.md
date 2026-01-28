# Content

* Member functions
  * [constructor](#constructor---filemanagerfile_path)
* Element access
  * [read](#read---readindex)
  * [front](#front---front)
  * [back](#back---back)
  * [all](#all---all)
* Modifiers
  * [append](#append---appendargs)
  * [overwrite](#overwrite---overwriteindex-args)
* Persistence
  * [flush](#flush---flush)
  * [commit](#commit---commit)

# -- MEMBER FUNCTIONS --

## CONSTRUCTOR - filemanager(file_path)

### Description

Constructs a new filemanager instance and maps the file structure.

### Parameters

| Parameter   | Datatype                | Description    | Notes                                     |
|-------------|-------------------------|----------------|-------------------------------------------|
| `file_path` | `std::filesystem::path` | Path to a file | File will be created if it does not exist |

### Exceptions

| Exception            | Cause                                        | Fix                                                                                     |
|----------------------|----------------------------------------------|-----------------------------------------------------------------------------------------|
| `std::runtime_error` | File was deleted or problems with permission | Ensure to run the program with enough permissions and check if read-only mode is active |

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

### Parameters

| Parameter | Datatype | Description        | Notes       |
| --------- | -------- | ------------------ | ----------- |
| `index`   | `size_t` | Which line to read | Starts at 0 |

### Returns

| Type          | Description    | Notes |
| ------------- | -------------- | ----- |
| `std::string` | Specified line |       |

### Exceptions

| Exception            | Cause                                        | Fix                                                                                     |
| -------------------- | -------------------------------------------- | --------------------------------------------------------------------------------------- |
| `std::out_of_range`  | Line doesn't exist                           | Check bounds with `size()` or `empty()` before access                                   |
| `std::runtime_error` | File was deleted or problems with permission | Ensure to run the program with enough permissions and check if read-only mode is active |

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

### Returns

| Type          | Description | Notes |
|---------------|-------------|-------|
| `std::string` | First line  |       |

### Exceptions

| Exception            | Cause                                        | Fix                                                                                     |
|----------------------|----------------------------------------------|-----------------------------------------------------------------------------------------|
| `std::out_of_range`  | File is empty                                | Check bounds with `size()` or `empty()` before access                                   |
| `std::runtime_error` | File was deleted or problems with permission | Ensure to run the program with enough permissions and check if read-only mode is active |

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

### Returns

| Type          | Description | Notes |
|---------------|-------------|-------|
| `std::string` | Last line   |       |

### Exceptions

| Exception            | Cause                                        | Fix                                                                                     |
|----------------------|----------------------------------------------|-----------------------------------------------------------------------------------------|
| `std::out_of_range`  | File is empty                                | Check bounds with `size()` or `empty()` before access                                   |
| `std::runtime_error` | File was deleted or problems with permission | Ensure to run the program with enough permissions and check if read-only mode is active |

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

### Returns

| Type                       | Description           | Notes        |
|----------------------------|-----------------------|--------------|
| `std::vector<std::string>` | All lines in the file | Can be empty |

### Exceptions

| Exception            | Cause                                        | Fix                                                                                     |
|----------------------|----------------------------------------------|-----------------------------------------------------------------------------------------|
| `std::runtime_error` | File was deleted or problems with permission | Ensure to run the program with enough permissions and check if read-only mode is active |

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

# -- MODIFIERS --

## APPEND - `append(args)`

### Description

Adds data as a new line at the end of the file. Doesn't update the file (Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).

### Parameters

| Parameter | Datatype  | Description                  | Notes                         |
|-----------|-----------|------------------------------|-------------------------------|
| `args`    | `Args...` | One or more values to append | Must not contain `\n` or `\r` |

### Exceptions

| Exception               | Cause                                           | Fix                                                                    |
|-------------------------|-------------------------------------------------|------------------------------------------------------------------------|
| `std::invalid_argument` | Input contains newline character (`\n` or `\r`) | Remove `\n` and `\r` ocurrences when using `append()` or `overwrite()` |

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

Overwrites a line with new data.

*Does not update the file (Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).*

### Parameters

| Parameter | Datatype  | Description                  | Notes                         |
|-----------|-----------|------------------------------|-------------------------------|
| `index`   | `size_t`  | Line to overwrite            | Starts at 0                   |
| `args`    | `Args...` | One or more values to append | Must not contain `\n` or `\r` |

### Exceptions

| Exception               | Cause                                           | Fix                                                                    |
|-------------------------|-------------------------------------------------|------------------------------------------------------------------------|
| `std::invalid_argument` | Input contains newline character (`\n` or `\r`) | Remove `\n` and `\r` ocurrences when using `append()` or `overwrite()` |
| `std::out_of_range`     | Specified line doesn't exist                    | Check bounds with `size()` or `empty()` before access                  |

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

# -- PERSISTENCE --

## FLUSH - `flush()`

### Description

Records all changes in a journal, making recovery possible after a crash.

**Does not apply changes** (see [commit()](#commit---commit)).

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

### File content

```

```

## COMMIT - `commit()`

### Description

Deletes the journal and applies all changes to the main file.

Triggers reconstruction internals to make up for the new file. It's **discouraged to repeatedly call** `commit()`. It can have a **noticeable impact on performance** if used carelessly on big files.

### Returns

| Type   | Description                       | Notes                                                         |
| ------ | --------------------------------- | ------------------------------------------------------------- |
| `bool` | Whether committing was successful | Handle possible failures if the working directory is unstable |

Code sample

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