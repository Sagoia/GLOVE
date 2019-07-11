#!/bin/bash
RETVAL=0
RED='\e[0;31m'
GREEN='\e[0;32m'
YELLOW='\e[0;33m'
NOCOLOR='\e[0m'
METRIC=AE
FUZZ=3%
GLOVE_ROOT=../../../
REF_PATH=$GLOVE_ROOT/Demos/ref

function printUsage() {
    echo "Usage:"
    echo "./run_all_samples.sh <option> where option is either of:"
    echo " --debug      # Save OpenGL ES API Debug output in a file per demo"
    echo " --profile    # Save OpenGL ES API Profile output in a file per demo"
    echo " --smoke      # Run GLOVE Smoke Tests"
}

function setupDemos() {

    if   [ $SMOKE_TESTS == "true" ]; then
        export GLOVE_DEMOS_MODE=GLOVE_CI
        # clean-up older results. if any
        rm *.rgba
        rm *.ppm
    fi

    CMDDIR="$(pwd)"
    #  choose the demo samples to be played
    SAMP2RUN="$CMDDIR/triangle2d_one_color \
              $CMDDIR/triangle2d_split_colors \
              $CMDDIR/circle2d_sdf \
              $CMDDIR/texture2d_color \
              $CMDDIR/cube3d_vertexcolors \
              $CMDDIR/cube3d_textures \
              $CMDDIR/render_to_texture_filter_gamma \
              $CMDDIR/render_to_texture_filter_invert \
              $CMDDIR/render_to_texture_filter_grayscale \
              $CMDDIR/render_to_texture_filter_sobel \
              $CMDDIR/render_to_texture_filter_boxblur"
}

function runDemos() {
    echo "*******************************************"
    echo "******* Running Demos...*******************"
    echo "*******************************************"

    # display short description of the sample and run it
    for file in $SAMP2RUN
    do
        # get name of the sample source file; need to remove .c from name
        BNAME=$(basename $file)
        echo "SAMPLE:  ./$BNAME"

        # run the built sample;
        RNAME=./${BNAME}

        if   [ $DEBUG == "true" ] && [ $PROFILE == "true" ]; then
            $RNAME >${BNAME}_gl_profile.log 2>${BNAME}_gl_error.log
        elif [ $DEBUG == "true" ]; then
            $RNAME 2>${BNAME}_gl_error.log
        elif [ $PROFILE == "true" ]; then
            $RNAME >${BNAME}_gl_profile.log
        else
            $RNAME
        fi
    done
}

function evaluateSmokeResults() {
    echo "*******************************************"
    echo "******* Evaluating Smoke Results...********"
    echo "*******************************************"

    for file in $SAMP2RUN
    do

        # get name of the sample source file; need to remove .c from name
        BNAME=$(basename $file)

        REFNAME="${REF_PATH}/${BNAME}.rgba"
        if test -f $REFNAME; then
            THISNAME="${BNAME}.rgba"
            CMDRES=`compare -metric $METRIC -fuzz $FUZZ -size 600x600 -depth 8 $THISNAME -size 600x600 -depth 8 $REFNAME ${BNAME}-diff.ppm 2>&1`
            if [ $CMDRES == "0" ]; then
                >&2 echo -e "${GREEN}${BNAME} PASS${NOCOLOR}"
            else
                >&2 echo -e "${RED}${BNAME} FAIL${NOCOLOR} : pixel error count is ${CMDRES}"
                RETVAL=1
            fi
        else
            >&2 echo -e "${YELLOW}${BNAME} FAIL${NOCOLOR} : Reference image is missing"
            RETVAL=1
        fi
    done
}

DEBUG=false
PROFILE=false
SMOKE_TESTS=false

for option in "$@"
do
    case $option in
        --help)
            printUsage
            exit 1
            ;;
        --debug)
            DEBUG=true
            echo "Enable Debug   ($option)"
            ;;
        --profile)
            PROFILE=true
            echo "Enable Profile ($option)"
            ;;
        --smoke)
            SMOKE_TESTS=true
            echo "Run Smoke Tests ($option)"
            ;;
        *)
            printUsage
            exit 1
            ;;
    esac
    shift
done

setupDemos
runDemos
if   [ $SMOKE_TESTS == "true" ]; then
    evaluateSmokeResults
fi

exit $RETVAL
