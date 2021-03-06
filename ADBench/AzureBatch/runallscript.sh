#!/bin/bash

# This script is created for Ubuntu 18.04 for Azure Batch.
# It should be run by the root user.

# Script constants.
REPOSITORY_URL="https://github.com/awf/ADBench"
RESULT_ROOT="$AZ_BATCH_TASK_WORKING_DIR/results"
INPUT_ROOT="$AZ_BATCH_TASK_WORKING_DIR"
COMMIT_HASH_FILE_NAME="last_commit.txt" # holds the hash of the last commit,
                                        # this script has been run for

# Exit codes.
SUCCESS_EXIT_CODE=0
UNNECESSARY_RUN_EXIT_CODE=1
GIT_PROBLEM_EXIT_CODE=2
DOCKER_INSTALLATION_PROBLEM_EXIT_CODE=3
DOCKER_ACTIVATING_PROBLEM_EXIT_CODE=4
DOCKER_BUILD_PROBLEM_EXIT_CODE=5
TEST_FAIL_EXIT_CODE=6
TOOL_RUNNING_FAILURE_EXIT_CODE=7
PLOT_CREATING_FAILURE_EXIT_CODE=8



# Dealing with the problem that seems to be described here:
#
#   https://github.com/Microsoft/azure-pipelines-image-generation/issues/185
#
# but solution that is provided didn't help in this script. So, the script just
# deletes locking files.
rm -f /var/lib/dpkg/lock
rm -f /var/lib/dpkg/lock-frontend

# Note that all "apt-get" actions are done using this comand prefix:
#
#   DEBIAN_FRONTEND="noninteractive"
#
# This is necessary because on the node this script is running without tty,
# so, "apt-get" and tools that it calls can't create a dialog and crash.
# Giving this environment variable we explicitly say that interactive mode
# should be turned off.

# Clone the repo.
cd $AZ_BATCH_TASK_WORKING_DIR
DEBIAN_FRONTEND="noninteractive" apt-get update \
    && apt-get install -y git \
    && git clone $REPOSITORY_URL

if [[ $? -ne 0 ]]
then
    echo "Repository cloning is crashed! Stopping the script"
    exit $GIT_PROBLEM_EXIT_CODE
fi

cd ADBench

# Check the last commit.
COMMIT=$(git rev-parse HEAD)
if [[ $COMMIT == $(cat "$INPUT_ROOT/$COMMIT_HASH_FILE_NAME") ]]
then
    echo "Latest commit has been run. Stopping the script"

    # Return non-zero code to prevent result files uploading to the share.
    exit $UNNECESSARY_RUN_EXIT_CODE
fi

# Install docker.
DEBIAN_FRONTEND="noninteractive" apt-get install -y docker.io

# Check that docker is installed.
if [[ $? -ne 0 ]]
then
    echo "Docker installation fail! Stopping the script"
    exit $DOCKER_INSTALLATION_PROBLEM_EXIT_CODE
fi

# Start docker.
systemctl start docker && sudo systemctl enable docker

# Check that docker is active.
if [[ ! $(systemctl status docker | grep 'Active: active (running)') ]]
then
    echo "Docker is not active! Stopping the script"
    exit $DOCKER_ACTIVATING_PROBLEM_EXIT_CODE
fi

# Create docker container.
docker build -t adb-docker .

# Check that the container has been created successfully.
if [[ $? -ne 0 ]]
then
    echo "Docker container build fail! Stopping the script"
    exit $DOCKER_BUILD_PROBLEM_EXIT_CODE
fi

# Some tools/or interpreters (e.g. dotnet, julia) perform some long
# operations the first time they run, which theoretically can cause some
# unjustified timeouts. Running test before actual benchmarks would eliminate
# that problem. Moreover, thus we can be sure that the code in the repository
# passes the tests.

# Creating directory for test output.
mkdir testout

# Running CTests.
docker run -v $(pwd)/testout:/adb/tmp/ adb-docker -t -Q -T test

# Check that tests are passed.
if [[ $? -ne 0 ]]
then
    echo "Tests are failed! Stopping the script"
    exit $TEST_FAIL_EXIT_CODE
fi

# Prepare docker output directory.
mkdir tmp

# Run all tools.
docker run -v $(pwd)/tmp:/adb/tmp/ adb-docker -r "-timeout 900 -ba_max_n 10 -hand_max_n 10"

# Check tool running.
# Note: run-all.ps1 script returns "8" in case of non-fatal errors.
if [[ $? -ne 0 && $? -ne 8 ]]
then
    echo "Running all tools failure! Stopping the script"
    exit $TOOL_RUNNING_FAILURE_EXIT_CODE
fi

# Create plots.
docker run -v $(pwd)/tmp:/adb/tmp/ adb-docker -p --save --plotly

# Check plot creating.
if [[ $? -ne 0 ]]
then
    echo "Plot creating failure! Stopping the script"
    exit $PLOT_CREATING_FAILURE_EXIT_CODE
fi

# Create output directory name in the format:
#   <year>-<month>-<day>_<hour>-<minute>-<second>_<commit hash>
PLOT_DIR_NAME=$(date -u +"%Y-%m-%d_%H-%M-%S")_$COMMIT

# Create directories from which results will be stored to the blob container.
mkdir -p "$RESULT_ROOT/$PLOT_DIR_NAME/plotly"
mkdir "$RESULT_ROOT/$PLOT_DIR_NAME/static"

# Move results to output directory.
mv tmp/graphs/plotly/Release/* "$RESULT_ROOT/$PLOT_DIR_NAME/plotly"
mv tmp/graphs/static/Release/* "$RESULT_ROOT/$PLOT_DIR_NAME/static"

# Save hash of the commit this script has been run for.
echo -en $COMMIT > "$RESULT_ROOT/$COMMIT_HASH_FILE_NAME"

exit $SUCCESS_EXIT_CODE