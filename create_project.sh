#!/bin/zsh

project_name=$1
main_file_link=$2
root_dir=$PWD
project_dir=$root_dir/$project_name
build_dir=$root_dir/build/$project_name
main_file_name=$project_dir/$project_name\.cpp
project_file_template=$root_dir/CMakeLists.txt.template
project_file=$project_dir/CMakeLists.txt
generate_build_files_command='cmake $project_dir'
compiler_commands_file='compile_commands.json'

mkdir $project_dir
mkdir -p $build_dir

cp $project_file_template $project_file

eval "sed -i.bak 's/THIS_PROJECT_NAME/$project_name/g' $project_file"
rm ${project_file}.bak

wget -O $main_file_name "$main_file_link"
dos2unix $main_file_name

cd $build_dir
eval $generate_build_files_command
eval $generate_build_files_command

cd $project_dir
ln -s $build_dir/$compiler_commands_file $compiler_commands_file

echo 'project configured, have a nice coding session, bro'

