#!/bin/env bash

function get_version() {
  if [[ ! -e .version ]]; then
    echo "No .version file found. Creating."
    touch .version
    echo "0.0.0" > .version
  fi

  current_version=`cat .version`
  custom_version=""
  new_version=""
  replace_pat=""

  echo "Current version is $current_version"

  read -p "Select version type ((M)ajor, (m)inor, (p)atch or (c)ustom): " version_type

  if [[ $version_type == "major" || $version_type == "M" ]]; then
    replace_pat="(\$1+1), 0, 0"
    echo "Publishing major version."
    version_type="major"
  elif [[ $version_type == "minor" || $version_type == "m" ]]; then
    replace_pat="\$1, (\$2+1), 0"
    echo "Publishing minor version."
    version_type="minor"
  elif [[ $version_type == "patch" || $version_type == "p" || $version_type == "P" ]]; then
    replace_pat="\$1, \$2, (\$3+1)"
    echo "Publishing patch."
    version_type="patch"
  elif [[ $version_type == "custom" || $version_type == "c" || $version_type == "C" ]]; then
    read -p "Custom version: " custom_version
    version_type="custom"
  else
    echo "Unknown option " $version_type ". Exitting."
    exit 1
  fi

  if [[ $replace_pat != "" ]]; then
    new_version=`echo $current_version | awk -F. "{printf \"%d.%d.%d\", $replace_pat}"`
  else
    new_version=$curstom_version
  fi
}

function get_annotation() {
  tmpfile=$(mktemp)
  nano $tmpfile --temp
  annotation=`cat $tmpfile`
  rm $tmpfile
}

branch_name=`git rev-parse --abbrev-ref HEAD`

if [[ $branch_name != "master" ]]; then
  while [[ $branch_warned != "yes" && $branch_warned != "y" &&
        $branch_warned != "no" && $branch_warned != "n" ]]; do

    read -p "Currently on branch $branch_name. Publish from here (instead of master)? ((y)es or (n)o) " branch_warned
  done
  if [[ $branch_warned == "no" || $branch_warned == "n" ]]; then
    echo "Operation aborted."
    exit 1
  fi
fi

get_version

while [[ $refresh_docs != "yes" && $refresh_docs != "y" &&
    $refresh_docs != "no" && $refresh_docs != "n" ]]; do

read -p "Refresh docs? ((y)es or (n)o) " refresh_docs
done

while [[ $annotate != "yes" && $annotate != "y" &&
      $annotate != "no" && $annotate != "n" ]]; do

  read -p "Annotate commit? ((y)es or (n)o) " annotate
done

while [[ $confirm != "yes" && $confirm != "y" &&
      $confirm != "no" && $confirm != "n" ]]; do

  echo "Publishing new $version_type version: $new_version."
  echo "Refresh docs? $refresh_docs"
  echo "Annotate? ${annotate}"

  read -p "Confirm pusblishing? ((y)es or (n)o) " confirm
  if [[ $confirm == "no" || $confirm == "n" ]]; then
    echo "Publishing canceled by the user. Aborting."
    exit 1
  fi
done


if [[ $refresh_docs == "yes" || $refresh_docs == "y" ]]; then
  export DOXYGEN_PROJECT_NUMBER=$new_version
  make clean && make docs
  git add docs
fi

if [[ $annotate == "yes" || $annotate == "y" ]]; then
  get_annotation
fi

echo $new_version > .version

git add .version
git commit -m "Automatically bumped version to $new_version"

git tag -a $new_version -m "$annotation"
git push origin $new_version
