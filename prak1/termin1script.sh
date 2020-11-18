#!/bin/bash
usage(){
  cat <<EOF
  splitfix.sh [OPTIONS] FILE [FILE ...] Split FILE into fixed-size pieces.
  The pieces are 10 lines long, if FILE is a text file.  
  The pieces are 10 kiB long, if FILE is *not* a text file.
  The last piece may be smaller, it contains the rest of the original file.
  The output files bear the name of the input file with a 4-digit numerical suffix.
  The original file can be reconstructed with the command ‘‘cat FILE.*’’

  EXAMPLE:
    splitfix.sh foo.pdf 
    splits foo.pdf into the files 
      foo.pdf.0000 foo.pdf.0001 etc.

    splitfix.sh [-h | --help] This help text
    splitfix.sh --version Print version number

  OPTIONS:
  -h
    --help this help text
  -s  SIZE  size of the pieces
        in lines (for text files)
        or in kiBytes (for other files)
  -v
    --verbose print debugging messages
EOF
}

if [ $# -le 0 ]; then
  echo "Specify arguments. Use --help for help."
  exit 1
else
  argVerbose=false
  argSize=10
  argFile=""
  while [ $# -gt 0 ]
  do
  key="$1"
  case $key in 
    -h|--help)
        usage
        exit 1 
        ;;
    --version)
    echo "1.0"
    exit 1
    ;;
    -v|--verbose)
      argVerbose=true
      shift
      ;;
    -s|--size)
      argSize="$2"
      shift
      shift
      ;;
    *)
      argFile="$key"
      shift
      ;;
  esac
  done
fi
if [ "$argSize" -le 0 ]; then
  echo "size must be larger than 0."
fi

filetype=`file "$argFile" --mime-type`
if [[ "$filetype" == *:*"text"* ]]; then
  if [[ $argVerbose ]]; then
    echo "File is a textfile"
    (split "$argFile" -dl "$argSize" -a 4 --verbose "$argFile")
  else
    (split "$argFile" -dl "$argSize" -a 4 "$argFile")
  fi
else
  if [ $argVerbose ]; then
    echo "File is not a text file"
    (split "$argFile" -db "$argSize"K -a 4 --verbose "$argFile")
  else
    (split "$argFile" -db "$argSize"K -a 4 "$argFile")
  fi
fi

