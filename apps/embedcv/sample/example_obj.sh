#!/bin/sh


#
# Example from webpage: http://golem5.org/embedcv/example_obj.html
#

echo "Remember to press the 'q' key and advance to the next image"


echo
echo "Detect Sobel edges"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2edge | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2edge > sobel_edge.ppm 

echo
echo "Blur away noisy artifacts before detecting edges"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2blur -r 10 | ./ppm2edge | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2blur -r 10 | ./ppm2edge > blur_noise.ppm 

echo
echo "Show box feature image"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox > box.ppm 

echo
echo "Show box feature blob boundaries (oops, blobs are too large)"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 > box_blob.ppm 

echo
echo "Show box feature blob boundaries (good, blobs shrunk with thresholding)"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 > box_blob_select.ppm 

echo
echo "Look for objects (oops, found too many)"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | ./ppm2tbox | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | ./ppm2tbox > look_for_objects.ppm 

echo
echo "Look for objects (works pretty well)"
echo "	cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | ./ppm2tbox -t 50 -r 50 | display -"
cat pooltable.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | ./ppm2tbox -t 50 -r 50 > look_for_objects2.ppm 

echo
echo "Look for objects"
echo "	cat rocks.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | ./ppm2tbox -t 100 -r 50 | display -"
cat rocks.jpg | ./jpg2ppm | ./ppm2fbox -s 13 | ./ppm2morph -r -d 1 | ./ppm2morph -g -d 1 | ./ppm2tbox -t 100 -r 50 > look_for_objects3.ppm 



                                                                                
#   Tiempo de ejecución en stamp                                                                               
#                                                                                
# real    4m 31.82s                                                               
# user    4m 26.36s                                                               
# sys     0m 5.36s  
