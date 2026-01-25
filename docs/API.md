# filemanager(file_path)
Constructs a new filemanager instance and maps the file structure.

### Parameters
**std::filesystem::path** `file_path` - Path to a file which the filemanager should handle (e.g. `"C:\file.txt"`)

### Exceptions
`std::runtime_error` - Problems with permission



# read(index)
Retrieves the content of a specified line.

### Parameters
**size_t** `index` - Index of the line to read e.g. `0`

### Returns
`std::string` - Text at the specified line (without trailing '\n')

### Exceptions
`std::out_of_range` - Specified index is out of bounds (e.g. when trying to read line 5 when the file only has 4 lines)  
`std::runtime_error` - File was deleted or problems with permission



# front()
Retrieves the content at the first line.

### Returns
`std::string` - Text at the first line (without trailing '\n')

### Exceptions
`std::out_of_range` - File is empty  
`std::runtime_error` - File was deleted or problems with permission



# back()
Retrieves the content at the last line.

### Returns
`std::string` - Text at the last line (without trailing '\n')

### Exceptions
`std::out_of_range` - File is empty  
`std::runtime_error` - File was deleted or problems with permission



# all()
Returns a chronological copy of every line.

### Returns
`std::vector<std::string>` - Text at every line (without trailing '\n')

### Remarks
* Returning vector can be empty

### Exceptions
`std::runtime_error` - File was deleted or problems with permission



# append(args)
Adds the specified arguments to a new line at the end of the file.

### Parameters
**Args...** `args` - Values to append (e.g. `"Hello", " World", "!"`)

### Exceptions
`std::invalid_argument` - Tried to append string which contains newline character ('\n' or '\r')