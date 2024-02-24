#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <stack>
#include <sstream>
#include <unistd.h> 
#include <algorithm>

using namespace std;


// Function prototypes


// struct to store filename
struct Filename { 
    char filename[8];
    char nullTerminator = '\0';
    char period = '.';
    char extension;
};

// Function to create a filename
// takes a string and returns a Filename struct
Filename createFilename(char * filename) {

    Filename newFilename;

    string input = filename;
    size_t pos = input.find('.');

    // Allocate memory for filename
    char* fn = new char[8];

    // If there's a period and it's in the correct place, treat it as a file
    if (pos != string::npos && pos <= 8) {
        // Copy the filename or pad with null terminators
        for (int i = 0; i < 8; ++i) {
            int pos1 = pos;
            if (i < pos1) {
                fn[i] = input[i];
            } else {
                fn[i] = '\0';  // Null terminator for padding
            }
        }
       
        newFilename.extension = input[pos + 1];
        strcpy(newFilename.filename, fn);
         
    } 
    // If there's no period or it's in the wrong place, treat it as a directory
    else {
        // Copy the filename or pad with null terminators
        for (int i = 0; i < 8; ++i) {
            if (i < input.length()) {
                fn[i] = input[i];
            } else {
                fn[i] = '\0';  // Null terminator for padding
            }
        }

        strcpy(newFilename.filename, fn);
        newFilename.period = '.';
        newFilename.extension = 'd';
        newFilename.nullTerminator = '\0';
    }

    delete[] fn; // Deallocate memory for filename
    return newFilename;
}

// struct to store file
struct File {
    Filename filename;
    int size;
    char* contents;  // Dynamic array to store contents
};

// Function to create a file
// takes a string and returns a File struct
File createFile(char *filename) {
    
    File newFile;
    newFile.filename = createFilename(filename); 
    
    
    // Allocate memory for contents
    newFile.contents = new char[1024];  // Assuming maximum size of 1024
    if (isatty(STDIN_FILENO)) {
        cout << "Enter file contents: ";
    }
    
    
    cin.getline(newFile.contents, 1024);

    // Determine and set the size of the contents
    newFile.size = strlen(newFile.contents);
    

    return newFile;
}

// struct to store program
struct Program {
    Filename filename;
    int cpuRequired;
    int memoryRequired;

};

// Function to create a program
// takes a string and returns a Program struct
Program createProgram(char *filename) {
    Program newProgram;
    newProgram.filename = createFilename(filename); 

    if (isatty(STDIN_FILENO)) {
        cout << "Enter cpu required for the program: ";
    }
    cin >> newProgram.cpuRequired;
    if (isatty(STDIN_FILENO)) {
        cout << "Enter memory required for the program: "; 
    }
    
    cin >> newProgram.memoryRequired;
    cin.ignore();  // Ignore newline character

    return newProgram;
}



// struct to store directory
struct Directory {
    Filename filename;
    int numOfFiles = 0;
    vector<Directory> subdirectories;
    vector<File> files;
    vector<Program> programs;
    char end[15] = "End";
};

// Function to create a directory
// takes a string and returns a Directory struct
Directory createDirectory(char *filename) {
    Directory newDir;
    newDir.filename = createFilename(filename);
    strcat(newDir.end, newDir.filename.filename);
    return newDir;
}

// Function to print the directory
// takes a Directory struct, positions vector, position index, and indentation level as arguments
void printDirectory(const Directory& dir, vector<int>& positions, int& posIndex, int indent = 0) {
    // Print directory name with indentation
    cout << positions[posIndex++] << ": Directory: " << dir.filename.filename << dir.filename.period << dir.filename.extension <<  endl;

    // Print number of files/directories in the directory
    int total = dir.files.size() + dir.programs.size() + dir.subdirectories.size();
    cout << positions[posIndex++] << ":  Directory " << dir.filename.filename << " contains " << dir.numOfFiles << " files/directories" << endl;

    // Print files in the directory
    for (const File& file : dir.files) {
        cout << positions[posIndex++] << ":   Filename: " << file.filename.filename << file.filename.period << file.filename.extension << endl;
        cout << "      Type: Text file" << endl;
        cout << positions[posIndex++] << ":   Size of text file: " << file.size << " byte" << endl;
        cout << positions[posIndex++] << ":   Contents of text file: " << file.contents << endl;
    }

    // Print programs in the directory
    for (const Program& program : dir.programs) {
        cout << positions[posIndex++] << ":   Filename: " << program.filename.filename << dir.filename.period << dir.filename.extension <<  endl;
        cout <<  "      Type: Program" << endl;
        cout << "      Contents: CPU Requirement: " << program.cpuRequired << ", Memory Requirement: " << program.memoryRequired << endl;
    }

    // Recursively print subdirectories
    for (const Directory& subdir : dir.subdirectories) {
        printDirectory(subdir, positions, posIndex, indent + 2);
    }

    // Print end of directory
    cout << positions[posIndex++] << ": End of directory " << dir.filename.filename << dir.filename.period << dir.filename.extension << endl;
}



// Function to serialize the directory to a file
// takes a Directory struct and an ofstream object as arguments
void serialize(Directory& dir, ofstream& file) {
    // Write directory name
    file.write(dir.filename.filename, 8);
    file.write(&dir.filename.period, 1);
    file.write(&dir.filename.extension, 1);
    file.write(&dir.filename.nullTerminator, 1);

    // Write number of files
    file.write(reinterpret_cast<char*>(&dir.numOfFiles), sizeof(int));

    

    // Write files
    for (File& f : dir.files) {
        file.write(f.filename.filename, 8);
        file.write(&f.filename.period, 1);
        file.write(&f.filename.extension, 1);
        file.write(&f.filename.nullTerminator, 1);
        file.write(reinterpret_cast<char*>(&f.size), sizeof(int));
        file.write(f.contents, f.size);
    }

    
    // Write programs
    for (Program& p : dir.programs) {
        file.write(p.filename.filename, 8);
        file.write(&p.filename.period, 1);
        file.write(&p.filename.extension, 1);
        file.write(&p.filename.nullTerminator, 1);
        file.write(reinterpret_cast<char*>(&p.cpuRequired), sizeof(int));
        file.write(reinterpret_cast<char*>(&p.memoryRequired), sizeof(int));
    }

        // Write subdirectories
    for (Directory& subDir : dir.subdirectories) {
        serialize(subDir, file);
    }

    // Write end of directory
    file.write(dir.end, 14);

}

// Function to deserialize the directory from a file
// takes a Directory struct, an ifstream object, and a vector of positions as arguments
void deseralize(Directory& dir, ifstream& file, std::vector<int>& positions) {
    positions.push_back({file.tellg()});
    // Read directory name
    file.read(dir.filename.filename, 8);
    file.read(&dir.filename.period, 1);
    file.read(&dir.filename.extension, 1);
    file.read(&dir.filename.nullTerminator, 1);
    

    positions.push_back({file.tellg()});
    // Read number of files
    file.read(reinterpret_cast<char*>(&dir.numOfFiles), sizeof(int));
    int num = dir.numOfFiles;
  

    // Read files
    while (num > 0) {
        positions.push_back({file.tellg()});
        Filename f;
        file.read(f.filename, 8);
        file.read(&f.period, 1);
        file.read(&f.extension, 1);
        file.read(&f.nullTerminator, 1);
        
        
        if (f.extension == 't') {
            File nf;
            nf.filename = f;
            positions.push_back({file.tellg()});
            file.read(reinterpret_cast<char*>(&nf.size), sizeof(int));
            nf.contents = new char[nf.size];
            positions.push_back({file.tellg()});
            file.read(nf.contents, nf.size);
            dir.files.push_back(nf);
            num -= 1;
        } else if (f.extension == 'p') {
            Program np;
            np.filename = f;
            file.read(reinterpret_cast<char*>(&np.cpuRequired), sizeof(int));
            file.read(reinterpret_cast<char*>(&np.memoryRequired), sizeof(int));
            dir.programs.push_back(np);
            num -= 1;
        } else if (f.extension == 'd') {
            positions.pop_back();
            Directory nSubDir;
            // go back to the start of the directory
            file.seekg(-11, ios::cur);
            num -= 1;
            deseralize(nSubDir, file, positions);
            // Read end of directory
            
            dir.subdirectories.push_back(nSubDir);
        } else {
            break;
        }
    }
    positions.push_back({file.tellg()});
    file.read(dir.end, 14);

    
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

void fileCommand(Directory* currentDir, string& filename);
void mkdir(Directory* currentDir, const string& dirname);
void printInfo(Directory& root, string& file);
void cat(Directory* currentDir, const string& filename);
void ls(const Directory* currentDir);
void cd(Directory* currentDir, stack<Directory*>& dirStack, string& dirname);
void pwd(stack<Directory*>& dirStack);



// Main function
// takes command line arguments
int main(int argc, char** argv) {
    // Create a directory for testing

    string ogFileName = argv[1];
    string filename;
    ifstream file(ogFileName, ios::binary);
    stack<Directory*> dirStack;
    Directory root;
    // If the file does not exist, create it
    if (!file) {
        ofstream newFile(ogFileName, ios::binary);
        newFile.close();
        file.open(ogFileName, ios::binary);
        
        strcpy(root.filename.filename, "root\0\0\0\0");
        root.filename.extension = 'd';
          // Stack to keep track of path
        strcat(root.end, root.filename.filename);
        dirStack.push(&root);
    }else {
        // Deserialize the root directory from the binary file
        
        vector<int> positions;
        int posIndex1 =  0;
        deseralize(root, file, positions);
        dirStack.push(&root);
        Directory* currentDir = dirStack.top();

    }


    while (true) {
    if (isatty(STDIN_FILENO)) {
        cout << "EnterCommand>";
    }

    string line;
    getline(cin, line);  // Read entire line of input

    if (line.empty()) {
        continue;  // Ignore empty lines
    }

    std::stringstream ss(line); // Use the fully qualified name for stringstream
    string command;
    getline(ss, command, ' ');  // Parse command up to the first space

    Directory* currentDir = dirStack.top();

    string argument;
    getline(ss, argument);  // Get the rest of the line as the argument

    if (command == "createFile") {
        fileCommand(currentDir, argument);
    } else if (command == "mkdir") {
        argument = trim(argument);
        mkdir(currentDir, argument);
    } else if (command == "printInfo") {
        if (argument.empty()) {
            printInfo(root, ogFileName);
        } else{
            cout << "Invalid command. Please try again." << endl;
        }
    } else if (command == "cat") {
        argument = trim(argument);
        cat(currentDir, argument);
    } else if (command == "ls") {
        if (argument.empty()) {
            ls(currentDir);
        } else {
            cout << "Invalid command. Please try again." << endl;
        }
    } else if (command == "cd") {
        argument = trim(argument);
        cd(currentDir, dirStack, argument);
    } else if (command == "pwd") {
        if (argument.empty()) {
            pwd(dirStack);
        } else {
            cout << "Invalid command. Please try again." << endl;
        }
    } else if (command == "quit") {
        if (argument.empty()) {
            break;
        } else {
            cout << "Invalid command. Please try again." << endl;
        }
    } else {
        cout << "Invalid command. Please try again." << endl;
    }
}

    file.close();

    // ... (previous code)
    return 0;
}

// Function definitions
// takes a Directory struct and a string as arguments
void fileCommand(Directory* currentDir, string& filename) {
    

    string line = filename;
    size_t pos = line.find('.');
    if (pos != std::string::npos) {
        line = line.substr(0, 10);  // Truncate to 11 characters
        line.erase(line.find_last_not_of(" \n\r\t")+1);  // Remove trailing white spaces
        char* name = new char[line.length() + 1];  // Allocate memory for filename
        strcpy(name, line.c_str());    
        if (name[strlen(name) - 1] == 'p') {
            Program newProgram = createProgram(name);
            currentDir->programs.push_back(newProgram);  // Add program to current directory
            currentDir->numOfFiles++;
            return;
        } else if (name[strlen(name) - 1] == 't') {
            File newFile = createFile(name); 
            currentDir->files.push_back(newFile); // Add file to current directory
            currentDir->numOfFiles++;
            return;
        } 
    } else {
        cout << "Incorrect filename format. Please try again." << endl;
    }
}

// Function to create a directory
// takes a Directory struct and a string as arguments
void mkdir(Directory* currentDir, const string& dirname) {
    if (dirname.size() == 0) {   
         return;
    }
    string line = dirname;
    line = line.substr(0, 8);  // Truncate to 7 characters
    line.erase(line.find_last_not_of(" \n\r\t")+1);  // Remove trailing white spaces
    char* filename = new char[line.length() + 1];  // Allocate memory for filename
    strcpy(filename, line.c_str());
    Directory newDir = createDirectory(filename);
    currentDir->subdirectories.push_back(newDir); // Add directory to current directory
    currentDir->numOfFiles++;
    return;
}

// Function to print the directory
// takes a Directory struct and a string as arguments
void printInfo(Directory& root, string& file) {
     cout << "Binary file structure is:" << endl;
        // Serialize the directory to a file
        ofstream file1(file, ios::binary);
        serialize(root, file1);
        file1.close();

        // Deserialize the directory from the file
        Directory newRoot;
        ifstream inFile(file, ios::binary);
        vector<int> positions; // Store positions and descriptions of data
        deseralize(newRoot, inFile, positions);
        inFile.close();
        //Print the directory
        int posIndex = 0;
        printDirectory(newRoot, positions, posIndex);
}


// Function to print the contents of a text file
// takes a Directory struct and a string as arguments
void cat(Directory* currentDir, const string& filename) {
    string line = filename;
    string extention;
    size_t pos = line.find('.');
    if (pos != std::string::npos) {
        line = line.substr(0, pos);
        extention = filename.substr(pos, filename.length() - 1);
        if (extention != ".t") {
            cout << "File does not exist. Please try again." << endl;
            return;
        }
        for (File& file : currentDir->files) {
            if (line == file.filename.filename) {
               cout << "Text file contents: " << file.contents << endl;
               return;
            }
        }
        cout << "File does not exist. Please try again." << endl;
    }else {
        cout << "Incorrect filename format. Please try again." << endl;
    }
}

// Function to list the contents of the current directory
// takes a Directory struct as an argument
void ls(const Directory* currentDir) {

    cout << "Directory Name: " << currentDir->filename.filename << endl;
    for (const File& file : currentDir->files) {
        cout << "Filename: " << file.filename.filename << file.filename.period << file.filename.extension  << " Type: Text file " << endl;
    }
    
    for (const Program& program : currentDir->programs) {
        cout << "Filename: " << program.filename.filename << program.filename.period << program.filename.extension << " Type: Program " << endl;
    }
    
    for (const Directory& dir : currentDir->subdirectories) {
        cout << "Directory: " << dir.filename.filename << dir.filename.period << dir.filename.extension << " Type: Directory " << endl;
    }

}

// Function to change the current directory
// takes a Directory struct, a stack of Directory pointers, and a string as arguments
void cd(Directory* currentDir, stack<Directory*>& dirStack, string& dirname) {
    string dirName = dirname;
    if (dirName == "..") {
        if (dirStack.size() > 1) {  // Don't pop the root directory
            dirStack.pop();  // Pop current directory off stack
        }
    } else {
        for (Directory& dir : currentDir->subdirectories) {
            if (dirName == dir.filename.filename) {
                dirStack.push(&dir);
                return;
            }
        }
        cout << "Directory does not exist. Please try again." << endl;
    }
}



// Function to print the current working directory
// takes a stack of Directory pointers as an argument
void pwd(stack<Directory*>& dirStack) {
    stack<Directory*> tempStack = dirStack;  // Copy the directory stack
    vector<string> path;
    while (!tempStack.empty()) {
        path.push_back(tempStack.top()->filename.filename);
        tempStack.pop();
    }
    reverse(path.begin(), path.end());  // Reverse the path
    cout << "Current directory is ";
    for (const string& dir : path) {
        if (dir == "root") {
            cout <<  dir;
        } else {
            cout << "/" << dir;
        }
    }
    cout << endl;
}


