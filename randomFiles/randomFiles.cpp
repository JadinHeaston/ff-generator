//#include <Windows.h>

#include <iostream>
#include <fstream> //File creation.
#include <random> //For generating random strings.
#include <filesystem> //Directory creation
#include "thread_pool.hpp" //Thread pool stuff.

#include "nfd.h" //Native File Dialog.
//This is needed for virtually everything in BrowseFolder.
#include <shlobj.h>


#include <chrono> //Time tracking.


std::string random_string(std::size_t length);
std::wstring stringToWString(const std::string& s); //Does as said. Found here: https://forums.codeguru.com/showthread.php?193852-How-to-convert-string-to-wstring
void createFiles(const std::wstring& rootFolder, const int& fileAmount, const bool& randomStringInput, const int& randomStringLength, const bool& randomAttributes);
std::wstring charToWString(char* givenCharArray);
std::wstring formatFilePath(std::wstring givenFile);
std::wstring browseFolder();

int main(int argc, char* argv[])
{
    //START TIMER.
    std::chrono::time_point start = std::chrono::steady_clock::now();

    //Declaring variables
    std::wstring rootFolderLocation;

    int totalFileAmount = 0; //Define how many files to create.
    int filesPerFolder = 1000; //Defines how many files will exist in each folder. The last folder could be smaller, if not evenly divisible.
    int foldersPerFolder = 500; //Defines how many subfolders are created before a new 
    size_t randomStringSizes = 50; //Controls how large strings are. Be wary of the 255 path limit.
    bool writeRandomStringToFile = false; //Disabling this will GREATLY increase performance.
    bool randomAttributes = false; //Defines whether random attributes will be applied to created files.
    std::wstring subRootFolder;


    //READ INPUT:
    //-help
    //-p rootFolderLocation
    //-f filesPerFolder
    //-o foldersPerFolder
    //-r randomAttributes
    //-s randomStringSizes
    //-t totalFileAmount
    //-w writeRandomStringToFile


    //Verifying that no \ escaped " exist in the path string.
    for (int i = 0; i < argc; i++)
    {
        std::size_t found = std::string(argv[i]).find("\"");
        if (found != std::string::npos)
        {
            std::cout << "ERROR: Rogue quote found. Likely due to a \"\\\" placed before a double quote (\"). Please double check your input and try again." << std::endl;
            return 0;
        }
    }
    //Reading args
    if (argc == 1) //No arguments provided. Notify. Close program.
    {
        std::cout << "No arguments provided.\nUse the \"-h\" or \"-help\" switch to show the available options.\n(-s and -d are required for operation)" << std::endl;
        system("PAUSE");
        return 0;
    }
    else if (argc == 2) //Checking for help message.
    {
        if (strncmp(argv[1], "-h", 3) == 0 || strcmp(argv[1], "-help") == 0) //Checking second argument for if it is "-h" or "-help".
        {
            //Display help
            std::cout << "HELP PROVIDED. GET FUCKED" << std::endl;
            system("PAUSE");
            return 0;
        }
        else //No arguments provided. Notify. Close program.
        {
            std::cout << "Use the \"-h\" or \"-help\" switch to show the available options.\n(-s and -d are required for operation)" << std::endl;
            system("PAUSE");
            return 0;
        }
    }
    for (int i = 0; i < argc; i++) // Cycle through all arguments.
    {
        if (strncmp(argv[i], "-p", 2) == 0) //Root folder path
        {
            rootFolderLocation = formatFilePath(charToWString(argv[i + 1])); //Assign to variable.

            if (rootFolderLocation.back() == L'\\')
                rootFolderLocation.pop_back(); //Remove the slash.
            if (!std::filesystem::is_directory(rootFolderLocation)) //Verify path is real and valid. Fail if invalid
            {
                std::wcout << "-s path provided was NOT found. (" << rootFolderLocation << ")" << std::endl;
                return 0;
            }

            rootFolderLocation = rootFolderLocation + L"\\";

        }
        else if (strncmp(argv[i], "-f", 2) == 0) //Files per folder.
            filesPerFolder = atoi(argv[i + 1]);
        else if (strncmp(argv[i], "-o", 2) == 0) //Folders per folder.
            foldersPerFolder = atoi(argv[i + 1]);
        else if (strncmp(argv[i], "-r", 2) == 0) //Random attributes bool.
            randomAttributes = true;
        else if (strncmp(argv[i], "-s", 2) == 0) //Random string size.
            randomStringSizes = atoi(argv[i + 1]);
        else if (strncmp(argv[i], "-t", 2) == 0) //Total file amount.
            totalFileAmount = atoi(argv[i + 1]);
        else if (strncmp(argv[i], "-w", 2) == 0) //Write random strings to files bool.
            writeRandomStringToFile = true;
        //std::cout << argv[i] << std::endl;
    }

    //DISPLAY USER OUTPUT AND ALLOW THEM TO CANCEL.
        //IMPLEMENT ARGUMENT TO SKIP THIS.

    //Make sure that a total file count is provided. Kinda the point of the program, huh?
    if (totalFileAmount == 0)
    {
        std::cout << "ERROR: Please provide -t {NUMBER} to specify the total amount of files to create." << std::endl;
        return 0;
    }

    //FOLDER VALIDATION
    if (rootFolderLocation.empty()) //If no path is provided, give a folder selection dialog.
        rootFolderLocation = browseFolder();

    if (rootFolderLocation == L"CANCEL" || rootFolderLocation.empty()) //Check if any errors occured, or no input was given.
    {
        std::cout << "FAILED. NO PATH PROVIDED." << std::endl;
        return 0; //End.
    }


    thread_pool threadPool;//Creating thread pool
    int threadAssignmentIteration = totalFileAmount / filesPerFolder; //How many folders will be created. Could be 1 additional one, if there is a remainder.
    
    


    for (int iterator = 0; iterator < threadAssignmentIteration; ++iterator) //Iterate!
    {
        if (iterator % foldersPerFolder == 0 && iterator >= foldersPerFolder) //Create new directory under the root when max is hit.
        {
            subRootFolder = rootFolderLocation + stringToWString(random_string(randomStringSizes)) + L"\\"; //Randomly create a new random folder within that given directory.

            std::filesystem::create_directories(subRootFolder); //Create the initial random directory.
        }
        threadPool.push_task(createFiles, subRootFolder, filesPerFolder, writeRandomStringToFile, randomStringSizes, randomAttributes); //Task pushing!
    }
            

    if (totalFileAmount % filesPerFolder != 0)
        threadPool.push_task(createFiles, rootFolderLocation, totalFileAmount % filesPerFolder, writeRandomStringToFile, randomStringSizes, randomAttributes); //Catching remainders.


    threadPool.wait_for_tasks(); //Waiting for all tasks to finish

    std::chrono::time_point end = std::chrono::steady_clock::now(); // Stop the clock!
    std::cout << "FINISHED! - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl; //Display clock results.


    system("PAUSE");

}

void createFiles(const std::wstring& rootFolder, const int& fileAmount, const bool& randomStringInput, const int& randomStringLength, const bool& randomAttributes)
{
    std::wstring subFolder = rootFolder + stringToWString(random_string(randomStringLength)) + L"\\"; //Randomly create a new random folder within that given directory.

    std::filesystem::create_directories(subFolder); //Create the initial random directory.

    std::ofstream randomFile; //Create randomFile handle that will be used to create files.

    std::wstring randomString;

    for (size_t iterator = 0; iterator < fileAmount; ++iterator) //Iterate!
    {
        randomString = stringToWString(random_string(randomStringLength));
        randomFile.open(subFolder + randomString + L".txt"); //Create random file with a handle to it.

        if (randomStringInput) //Check if we are writing a string to the file.
            randomFile << random_string(randomStringLength); //Input a random string into the file.



        randomFile.close(); //Close random file.

        //if (randomAttributes)
        //{
        //    std::filesystem::last_write_time(std::filesystem::path(subFolder + randomString + L".txt"), rand()); //Changing last modified time.
        //    //Changing creation date.
        //}

    }
}

std::wstring stringToWString(const std::string& s)
{
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}

std::string random_string(std::size_t length)
{
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

std::wstring charToWString(char* givenCharArray)
{
    std::string intermediaryString = givenCharArray;
    int wchars_num = MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, wstr, wchars_num);

    return wstr;
}

//Takes a string and removes "/" and places "\\".
std::wstring formatFilePath(std::wstring givenFile)
{
    //Formating givenFile to have the slashes ALL be \ instead of mixed with / and \.
    for (int i = 0; i < (int)givenFile.length(); ++i)
    {
        if (givenFile[i] == '/')
            givenFile[i] = '\\';
    }

    return givenFile;
}

//BROWSE FOLDER - Opens a Windows Explorer browse folder dialog.
std::wstring browseFolder()
{
    std::string outputString;
    nfdchar_t* outPath;
    //The empty string can 
    nfdresult_t result = NFD_PickFolder(NULL, &outPath);

    if (result == NFD::NFD_OKAY) //If the result is good...
    {
        //USE THE PATH
        outputString = outPath;
        return stringToWString(outputString);
    }
    else if (result == NFD::NFD_CANCEL) //USER CANCELLED...
        return L"CANCEL";
    else //ERROR...
        std::cout << "Error: " << NFD_GetError() << std::endl;

    //Return null values. Either an error occured, or the user cancelled.
    outputString = "";
    return stringToWString(outputString);
}
