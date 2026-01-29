# Description
A super simple, lightweight and high-performance C++ library that handles file storage for you.

# Sample
```cpp
#include <iostream>
#include "filemanager.h"

int main() {
    fm::filemanager file("data.txt");
    file.append("Hello world!");        // Add line
    file.commit();                      // Hello world! is now saved
    
    std::cout << file.read(0);          // Read line
}
```

# What this class offers
1. Super simple to use and versatile; handles all ugly code parts about data persistence.
2. No complicated installation, setup or external dependencies (Just copy one file).
3. Active measures to prevent data loss.
4. Fills gaps for you; deleting a line in the middle shifts all later lines down.
5. Optimized for performance and low memory usage.

# Sections
* [Install guide and setup](docs/installation.md)
* [API documentation](docs/api.md)