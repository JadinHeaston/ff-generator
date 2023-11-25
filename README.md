# File and Folder Generator
Allows the creation of randomly named folders and files, with the ability to add random content to generated files.  

# TODO
- [] Create a mechanism that allows the user to specify how deep folders should nest.
- [] Allow option to create only folders.
- [] Implement random attribute functionality.
- [] Allow thread count to be specified properly.
- [] 
- []
- []

# Table of Contents
- [File and Folder Generator](#file-and-folder-generator)
- [TODO](#todo)
- [Table of Contents](#table-of-contents)
- [Arguments:](#arguments)
- [Licensing](#licensing)

# Arguments:
```
	--files-per-folder <INTEGER>
		Amount of files per folder.
	--folders-per-folder <INTEGER>
		Amount of folders per folder.
	-h | -help
		Provides basic help.
	--path <ROOT_FOLDER_PATH> | <BLANK>
		Note: A folder selection dialog will be used if this argument is not provided. 
		Root Folder Location.
		Defines where the folders and files should be created.
	-r
		Assign random attributes to created files/folders.
	--string-size <INTEGER>
		Size of random content string.
	--threads <INTEGER>
		Defines how many processing threads will be utilized.
		The default is all available threads.
		Note: This is often more limited by the disks speed.
	* --total-file-count <INTEGER>
		Total file count.
	-w
		Write random content string to file.
```

# Licensing