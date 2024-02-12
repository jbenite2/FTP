Follow the steps below to configure and run the project autograder:

1. On your host operating system (not vagrant), place the ```project_autograder``` folder under your  ```Semester-Project-Skeleton-Code``` folder.

2. All steps below must be executed on vagrant (under /vagrant). Move to the ```project_autograder``` folder.

3. Copy or place your client.cpp, server.cpp, and makefile to the ```submission``` folder in the ```project_autograder``` folder (path to the folder: project_autograder/submission).

4. In the ```project_autograder``` folder, you can find the ```setup.sh``` script. Run the ```setup.sh``` script using the command ```sudo sh setup.sh```. This will setup the environment for you.

5. Once the setup is done, run the ```grade-run.sh``` script. To run the ```grade-run.sh``` script using the command ```sudo sh grade-run.sh```.

6. You will see the total score printed out on your terminal.
