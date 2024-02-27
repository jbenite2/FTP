#!/bin/bash

# Check if the directory exists
if [ ! -d "project_autograder" ]; then
    echo "Error: /project_autograder directory does not exist or is not accessible."
    exit 1
fi

# Define the submission directory
directory="project_autograder/submission"

# Check if the submission directory exists
if [ -d "$directory" ]; then
    echo "Removing existing submission directory..."
    rm -r "$directory"
fi

# Create the submission directory
mkdir "$directory"

# Check if the directory creation was successful
if [ $? -ne 0 ]; then
    echo "Error: Failed to create the submission directory."
    exit 1
fi

# Copy files to the submission directory
cp "client.cpp" "$directory"
cp "server.cpp" "$directory"

# Check if the file copying was successful
if [ $? -ne 0 ]; then
    echo "Error: Failed to copy files to the submission directory."
    exit 1
fi

echo "Files copied successfully to $directory"

