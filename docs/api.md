# Content
* Member functions
  * [constructor](#filemanagerfile_path)
* Element access
  * [read](#readindex)
  * [front](#front)
  * [back](#back)
  * [all](#all)
* Modifiers
  * [append](#appendargs)

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
## append(args)
Adds the specified arguments to a new line at the end of the file.

### Parameters
**Args...** `args` - Values to append (e.g. `"Hello", " World", "!"`)

### Exceptions
`std::invalid_argument` - Tried to append string which contains newline character ('\n' or '\r')