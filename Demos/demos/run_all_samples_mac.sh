#!/bin/bash

if [ $# != 1 ]; then
    echo "Usage: ./run_all_samples_mac.sh <build_type>"
    echo "build_type: Debug/Release"
    exit 0
fi;

BUILD_TYPE="$1"

open -W ${BUILD_TYPE}/triangle2d_one_color.app
open -W ${BUILD_TYPE}/triangle2d_split_colors.app
open -W ${BUILD_TYPE}/circle2d_sdf.app
open -W ${BUILD_TYPE}/texture2d_color.app
open -W ${BUILD_TYPE}/cube3d_vertexcolors.app
open -W ${BUILD_TYPE}/cube3d_textures.app
open -W ${BUILD_TYPE}/render_to_texture_filter_gamma.app
open -W ${BUILD_TYPE}/render_to_texture_filter_invert.app
open -W ${BUILD_TYPE}/render_to_texture_filter_grayscale.app
open -W ${BUILD_TYPE}/render_to_texture_filter_sobel.app
open -W ${BUILD_TYPE}/render_to_texture_filter_boxblur.app
