# Content
* Member functions
  * [constructor](#filemanagerfile_path)
* Element access
  * [read](#readindex)
  * [front](#front)
  * [back](#back)
  * [all](#all)
* Modifiers
  * [append](#append---appendargs)
  * [overwrite](#overwrite---overwriteindex-args)
* Persistance
  * flush
  * commit

# -- MEMBER FUNCTIONS --
## filemanager(file_path)
Constructs a new filemanager instance and maps the file structure.  
\- Creates the file if it doesn't exist.

### Parameters
**std::filesystem::path** `file_path` - Path to a file which the filemanager should handle (e.g. `"C:\file.txt"`)

### Exceptions
**std::runtime_error** - Problems with permission

### Code sample
```
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt"); // Creates file if it doesn't exist
}
```


# -- ELEMENT ACCESS --
## read(index)
Retrieves the content of a specified line.

### Parameters
**size_t** `index` - Index of the line to read e.g. `0`

### Returns
`std::string` - Text at the specified line (without trailing '\n')

### Exceptions
**std::out_of_range** - Specified index is out of bounds (e.g. when trying to read line 5 when the file only has 4 lines)  
**std::runtime_error** - File was deleted or problems with permission

### Code sample
```
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
```
World
```


## front()
Retrieves the content at the first line.

### Returns
`std::string` - Text at the first line (without trailing '\n')

### Exceptions
`std::out_of_range` - File is empty  
`std::runtime_error` - File was deleted or problems with permission



## back()
Retrieves the content at the last line.

### Returns
`std::string` - Text at the last line (without trailing '\n')

### Exceptions
`std::out_of_range` - File is empty  
`std::runtime_error` - File was deleted or problems with permission



## all()
Returns a chronological copy of every line.

### Returns
`std::vector<std::string>` - Text at every line (without trailing '\n')

### Remarks
* Returning vector can be empty

### Exceptions
`std::runtime_error` - File was deleted or problems with permission



# -- MODIFIERS --
## APPEND - `append(args)`
### Description
Adds data as a new line at the end of the file. Doesn't update the file (Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).

### Parameters
| Parameter | Datatype  | Description                  | Notes                         |  
|-----------|-----------|------------------------------|-------------------------------|
| `args`    | `Args...` | One or more values to append | Must not contain `\n` or `\r` |

### Exceptions
| Exception               | Cause                                                                           | Fix                                                                                       |  
|-------------------------|---------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------|
| `std::invalid_argument` | Input contains newline character (`\n` or `\r`)                                 | Remove `\n` and `\r` occurences when using `append()` or `overwrite()`                    |

### Code sample
```
#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.append("hello", "world", '!', 11);
    std::cout << file.back();
}
```

### Output
```
helloworld!11
```



## OVERWRITE - `overwrite(index, args)`
### Description
Overwrites a line with new data. Doesn't update the file (Check [flush()](#flush---flush) and [commit()](#commit---commit) for more info).

### Parameters
| Parameter | Datatype  | Description                   | Notes                          |  
|-----------|-----------|-------------------------------|--------------------------------|
| `index`   | `size_t`  | Line to overwrite.            | Starts at 0.                   |
| `args`    | `Args...` | One or more values to append. | Must not contain `\n` or `\r`. |

### Exceptions
| Exception               | Cause                                           | Fix                                                                    |  
|-------------------------|-------------------------------------------------|------------------------------------------------------------------------|
| `std::invalid_argument` | Input contains newline character (`\n` or `\r`) | Remove `\n` and `\r` occurences when using `append()` or `overwrite()` |
| `std::out_of_range`     | Specified line doesn't exist                    | Check bounds with `size()` or `empty() `                               |

### Code sample
```
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
```
Goodbye
```

# -- PERSISTENCE --
## FLUSH - `flush()`

## COMMIT - `commit()`