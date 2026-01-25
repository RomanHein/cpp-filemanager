#include "../include/filemanager.h"

int main() {
    fm::filemanager filemanager("test.txt");
    filemanager.append("hello world!");
    filemanager.commit();
}
