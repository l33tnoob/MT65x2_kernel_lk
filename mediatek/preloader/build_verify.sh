#!/bin/bash

function show_usage() {
    echo "[Usage]       build_verify.sh [pass_string] [ignore_string]"
    echo "[Description] Build all projects with name containing \"pass_string\" and ignores projects with name containing \"ignore_string\"."
}

function show_projects() {
    pl_path="mediatek/preloader"

    cd ../../
    echo "projects = "
    projects=$(./makeMtk listp)
    for project in $projects
    do
    if [ "${project#*$1}" != "$project" ]; then
	if [ "$2" != "" ]; then
	    if [ "${project#*$2}" != "$project" ]; then
		echo "[ignore]" ${project}
		continue
	    fi
	fi
	echo ${project}
    fi
    done
    cd ${pl_path}
}

function build_projects() {
    pl_path="mediatek/preloader"
    cd ../../
    projects=$(./makeMtk listp)
    #verify projects
    for project in $projects
    do
    if [ "${project#*$1}" != "$project" ]; then
	if [ "$2" != "" ]; then
	    if [ "${project#*$2}" != "$project" ]; then
		continue
	    fi
	fi
    else
	continue
    fi

    echo "verifying project:" ${project}
    echo "building..."
    androidq ./makeMtk -t ${project} n pl > ${pl_path}/log.txt 2>&1

    #check build log
    if grep -i -n "Building Success" ${pl_path}/log.txt; then
	echo "${project} build ok!"
    else
	if grep -i -n "Not support preloader" ${pl_path}/log.txt; then
	    continue
	fi
	echo "${project} build failed! See log.txt for more information!"
	break
    fi
    done

    cd ${pl_path}
}

#main flow
if [ "$1" == "" ]; then
    show_usage;
fi
show_projects $1 $2;
build_projects $1 $2;

