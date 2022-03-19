//#include <Windows.h>

#include <iostream>
#include <fstream> //File creation.
#include <random> //For generating random strings.
#include <filesystem> //Directory creation
#include <chrono> //Time tracking.

#include "thread_pool.hpp" //Thread pool stuff.

#include "nfd.h" //Native File Dialog.
#include <shlobj.h> //This is needed for virtually everything in BrowseFolder.



//Holds an array of single letter arguments that need to be applied.
std::unordered_map<char, size_t> singleCharArguments;


std::string random_string(std::size_t length);
void createFiles(const std::string& rootFolder, const size_t& fileAmount, const bool& randomStringInput, const size_t& randomStringLength, const bool& randomAttributes);
std::string formatFilePath(std::string givenString, std::string givenDirectorySeparator = "");
std::string browseFolder();

std::string directorySeparator = "/";

int main(int argc, char* argv[])
{
    //START TIMER.
    std::chrono::time_point start = std::chrono::steady_clock::now();

    //Declaring variables
    std::string rootFolderLocation;

    size_t totalFileAmount = 0; //Define how many files to create.
    size_t filesPerFolder = 1000; //Defines how many files will exist in each folder. The last folder could be smaller, if not evenly divisible.
    size_t foldersPerFolder = 500; //Defines how many subfolders are created before a new 
    size_t randomStringSizes = 50; //Controls how large strings are. Be wary of the 255 path limit.
    bool writeRandomStringToFile = false; //Disabling this will GREATLY increase performance.
    bool randomAttributes = false; //Defines whether random attributes will be applied to created files.
    bool skipWarning = false; 
    std::string subRootFolder;
    size_t threads = std::thread::hardware_concurrency();
    bool windowsMaxPathBypass = false;

    //Verifying that no \ escaped " exist in the path string.
    for (size_t i = 0; i < argc; i++)
    {
        std::size_t found = std::string(argv[i]).find("\"");
        if (found != std::string::npos)
        {
            std::cout << "ERROR: Rogue quote found. Likely due to a \"\\\" placed before a double quote (\"). Please double check your input and try again." << std::endl;
            return 0;
        }
    }

    for (size_t i = 0; i < argc; i++) // Cycle through all arguments.
    {
        //Check if the argument contains a single or double slash
        if (strncmp(argv[i], "--", 2) == 0) //Check for double slash
        {
            if (strcmp(argv[1], "--help") == 0) //Checking second argument for if it is "-h" or "-help".
            {
                //Display help
                std::cout << "HELP PROVIDED. GET FUCKED" << std::endl;

                system("PAUSE");
                return 0;
            }
            else if ((strcmp(argv[i], "--files-per-folder") == 0)) //Files per folder.
                filesPerFolder = atoi(argv[i + 1]);
            else if ((strcmp(argv[i], "--folders-per-folder") == 0)) //Gets the provided configuration name.
                foldersPerFolder = atoi(argv[i + 1]);
            else if ((strcmp(argv[i], "--path") == 0))
            {
                rootFolderLocation = formatFilePath(argv[i + 1]); //Assign to variable.

                if (rootFolderLocation.back() == L'\\')
                    rootFolderLocation.pop_back(); //Remove the slash.
                if (!std::filesystem::is_directory(rootFolderLocation)) //Verify path is real and valid. Fail if invalid
                {
                    std::cout << "-s path provided was NOT found. (" << rootFolderLocation << ")" << std::endl;
                    return 0;
                }

                rootFolderLocation = rootFolderLocation + "\\";
            }
            else if ((strcmp(argv[i], "--string-size") == 0)) //Random string size.
                randomStringSizes = atoi(argv[i + 1]);
            else if ((strcmp(argv[i], "--total-file-count") == 0)) //Total file amount.
                totalFileAmount = atoi(argv[i + 1]);
            else if ((strcmp(argv[i], "--threads") == 0))
                threads = atoi(argv[i + 1]);
        }
        else if (strncmp(argv[i], "-", 1) == 0) //Check for single dash.
        {
            for (size_t iterator = 1; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
                singleCharArguments[tolower(argv[i][iterator])] = 1; //Ensuring keys are lowercase for easy use later.
        }
    }
            

    //Iterating through argument array and applying arguments.
    for (size_t iterator = 0; iterator < sizeof(singleCharArguments); ++iterator)
    {
        //std::cout << singleCharArguments['h'] << std::endl;
        if (singleCharArguments['h']) //Short help message.
        {
            //Display help message.
            std::cout << "The only required argument is \"--total-file-count <INTEGER>\"." << std::endl;
            std::cout << "Detailed help can be found by using '--help' or utilizing the readme.md file: https://github.com/JadinHeaston/ff-generator" << std::endl;
            system("PAUSE");
            exit(1);
        }
        else if (singleCharArguments['l']) //Windows Max Path Bypass
            windowsMaxPathBypass = true;
        else if (singleCharArguments['r']) //Random attributes bool.
            randomAttributes = true;
        else if (singleCharArguments['s']) //Skips the warning and confirmation screen.
            skipWarning = true;
        else if (singleCharArguments['w']) //Write random strings to files bool.
            writeRandomStringToFile = true;
    }
    


    //Argument Verification
    
    //Make sure that a total file count is provided. Kinda the point of the program, huh?
    totalFileAmount = 1;
    if (totalFileAmount == 0)
    {
        std::cout << "ERROR: No file amount was defined." << std::endl;
        std::cout << "Use --total-file-count <INTEGER> to define how many files should be created." << std::endl;
        //Prompt the user to enter it now, or cancel.
        system("PAUSE");
        return 0;
    }

    //FOLDER VALIDATION
    if (rootFolderLocation.empty()) //If no path is provided, give the folder selection dialog.
        rootFolderLocation = browseFolder();

    if (rootFolderLocation == "CANCEL" || rootFolderLocation.empty()) //Check if any errors occured, or no input was given.
    {
        std::cout << "FAILED. NO PATH PROVIDED." << std::endl;
        return 0; //End.
    }

    //MAX_PATH bypass.
    //Also ensuring that path is an absolute path.
    if (windowsMaxPathBypass)
        rootFolderLocation = formatFilePath("\\\\?\\" + std::filesystem::absolute(rootFolderLocation).string(), "\\");
    else
        rootFolderLocation = formatFilePath(std::filesystem::absolute(rootFolderLocation).string());



    //ARGS FINISHED.



    
    //Displaying warning/confirmation screen.
    if (!skipWarning)
    {
        //DISPLAY USER OUTPUT AND ALLOW THEM TO CANCEL.
        std::cout << "----- WARNING -----" << std::endl;
        std::cout << "Read the below settings very carefully:" << std::endl;
        std::cout << "The root path (--path) is: " << rootFolderLocation << std::endl;
        std::cout << "CALCULATE ME" << " Folders will be created." << std::endl; //***** Calculate how many folders will be made.
        std::cout << totalFileAmount << " Files will be created. (--total-file-count)" << std::endl;
        std::cout << "A maximum of " << filesPerFolder << " files can exist in each folder. (--files - per - folder)" << std::endl;
        if (writeRandomStringToFile)
            std::cout << "A \"" << randomStringSizes << "\" length string will be randomly created within each file. (--string-size)" << std::endl;
        if (randomAttributes)
            std::cout << "Each file, and folder, will have completely random attributes. (-r)" << std::endl;

        system("PAUSE");
    }





    thread_pool threadPool(threads);//Creating thread pool
    size_t threadAssignmentIteration = totalFileAmount / filesPerFolder; //How many folders will be created. Could be 1 additional one, if there is a remainder.
    
    for (size_t iterator = 0; iterator < threadAssignmentIteration; ++iterator) //Iterate!
    {
        if (iterator % foldersPerFolder == 0 && iterator >= foldersPerFolder) //Create new directory under the root when max is hit.
        {
            subRootFolder = rootFolderLocation + random_string(randomStringSizes) + "\\"; //Randomly create a new random folder within that given directory.

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
    return 1;
}

void createFiles(const std::string& rootFolder, const size_t& fileAmount, const bool& randomStringInput, const size_t& randomStringLength, const bool& randomAttributes)
{
    std::string subFolder = rootFolder + random_string(randomStringLength) + "\\"; //Randomly create a new random folder within that given directory.

    std::filesystem::create_directories(subFolder); //Create the initial random directory.

    std::ofstream randomFile; //Create randomFile handle that will be used to create files.

    std::string randomString;

    for (size_t iterator = 0; iterator < fileAmount; ++iterator) //Iterate!
    {
        randomString = random_string(randomStringLength);
        randomFile.open(std::filesystem::path(subFolder + randomString + ".txt")); //Create random file with a handle to it.

        if (randomStringInput) //Check if we are writing a string to the file.
            randomFile << random_string(randomStringLength); //Input a random string into the file.



        randomFile.close(); //Close random file.

        if (randomAttributes)
        {
            //std::filesystem::last_write_time(std::filesystem::path(subFolder + randomString + L".txt"), rand()); //Changing last modified time.
            //Changing creation date.
        }

    }
}

std::string random_string(size_t length)
{
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (size_t i = 0; i < length; ++i)
        random_string += CHARACTERS[distribution(generator)];

    return random_string;
}

//Uniformly sets directory separators.
std::string formatFilePath(std::string givenString, std::string givenDirectorySeparator)
{
    if (givenDirectorySeparator == "\\" || givenString.find("\\\\?\\") != std::string::npos) //If the windows max_path bypass is in the path, then all separators must be backslashes.
    {
        //Formating givenFile to have the slashes ALL be \.
        for (size_t i = 0; i < (size_t)givenString.length(); ++i)
        {
            if (givenString[i] == '/')
                givenString[i] = '\\';
        }
    }
    else
    {
        //Formating givenFile to have the slashes ALL be /.
        for (size_t i = 0; i < (size_t)givenString.length(); ++i)
        {
            if (givenString[i] == '\\')
                givenString[i] = '/';
        }
    }


    return givenString;
}


//BROWSE FOLDER - Opens a Windows Explorer browse folder dialog.
std::string browseFolder()
{
    std::string outputString;
    nfdchar_t* outPath;
    //The empty string can 
    nfdresult_t result = NFD_PickFolder(NULL, &outPath);

    if (result == NFD::NFD_OKAY) //If the result is good...
    {
        //USE THE PATH
        outputString = outPath;
        return outputString;
    }
    else if (result == NFD::NFD_CANCEL) //USER CANCELLED...
        return "CANCEL";
    else //ERROR...
        std::cout << "Error: " << NFD_GetError() << std::endl;

    //Return null values. Either an error occured, or the user cancelled.
    outputString = "";
    return outputString;
}
